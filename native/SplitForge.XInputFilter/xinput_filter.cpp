// SplitForge · Filtro XInput por proceso (EXPERIMENTAL)
//
// Se copia en la carpeta de un juego con el nombre de la DLL de XInput que el juego carga
// (xinput1_3.dll, xinput1_4.dll o xinput9_1_0.dll). Reenvía todas las llamadas a la DLL real
// de System32, pero hace que ESTE proceso vea un único mando: la ranura física que SplitForge
// le asigna en %LOCALAPPDATA%\SplitForge\xinput\asignacion.txt (líneas "PID RANURA").
// Sin asignación para el PID, es transparente.
//
// Estado: cada copia cargada escribe estado-PID-<dll>.txt (7DTD carga xinput1_3 y xinput1_4 a la vez),
// con la ranura vigente, cuántas lecturas (XInputGetState/Ex) ha atendido y si el foco está forzado.
// Se reescribe al cambiar la asignación y, como mucho, cada 2 s mientras el juego lee mandos.
//
// Con mando asignado, además simula el foco de su ventana (ver «Foco simulado»): en pantalla dividida
// cada cliente atiende su mando aunque no sea la ventana activa.
//
// No inyecta código en otros procesos. Úsese con el juego abierto sin EAC (opción oficial del juego).

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <atomic>
#include <cstdio>
#include <cwchar>

namespace {

constexpr DWORD kNotConnected = 1167; // ERROR_DEVICE_NOT_CONNECTED
constexpr int kPassThrough = -1;      // Sin asignación: comportamiento original.
constexpr int kMaxSlots = 4;
constexpr ULONGLONG kReloadMs = 1000;
constexpr ULONGLONG kStatusMs = 2000;

struct XInputGamepad {
    WORD buttons;
    BYTE leftTrigger;
    BYTE rightTrigger;
    SHORT thumbLX;
    SHORT thumbLY;
    SHORT thumbRX;
    SHORT thumbRY;
};

struct XInputState {
    DWORD packetNumber;
    XInputGamepad gamepad;
};

struct XInputVibration {
    WORD leftMotorSpeed;
    WORD rightMotorSpeed;
};

struct XInputCapabilities {
    BYTE type;
    BYTE subType;
    WORD flags;
    XInputGamepad gamepad;
    XInputVibration vibration;
};

struct XInputBatteryInformation {
    BYTE batteryType;
    BYTE batteryLevel;
};

struct XInputKeystroke {
    WORD virtualKey;
    WCHAR unicode;
    WORD flags;
    BYTE userIndex;
    BYTE hidCode;
};

using GetStateFn = DWORD(WINAPI*)(DWORD, XInputState*);
using SetStateFn = DWORD(WINAPI*)(DWORD, XInputVibration*);
using GetCapabilitiesFn = DWORD(WINAPI*)(DWORD, DWORD, XInputCapabilities*);
using EnableFn = void(WINAPI*)(BOOL);
using GetDSoundGuidsFn = DWORD(WINAPI*)(DWORD, GUID*, GUID*);
using GetBatteryFn = DWORD(WINAPI*)(DWORD, BYTE, XInputBatteryInformation*);
using GetKeystrokeFn = DWORD(WINAPI*)(DWORD, DWORD, XInputKeystroke*);
using GetAudioIdsFn = DWORD(WINAPI*)(DWORD, LPWSTR, UINT*, LPWSTR, UINT*);
using WaitGuideFn = DWORD(WINAPI*)(DWORD, DWORD, void*);
using CancelGuideFn = DWORD(WINAPI*)(DWORD);
using PowerOffFn = DWORD(WINAPI*)(DWORD);

HMODULE g_self = nullptr;
std::atomic<HMODULE> g_real{nullptr};
std::atomic<int> g_slot{kPassThrough};
std::atomic<ULONGLONG> g_nextReload{0};
std::atomic<unsigned long long> g_reads{0}; // Llamadas a XInputGetState/Ex atendidas por esta copia.
std::atomic_flag g_statusBusy; // C++20: empieza libre.
int g_writtenSlot = -2;                       // Último estado escrito (solo con g_statusBusy tomado).
unsigned long long g_writtenReads = ~0ull;
ULONGLONG g_writtenAt = 0;
std::atomic<bool> g_statusDirty{false};        // Algo cambió (p. ej. el foco): reescribir el estado ya.
std::atomic<bool> g_focusActive{false};
wchar_t g_realName[MAX_PATH] = L"xinput1_3.dll";

// Nombre de esta DLL (xinput1_3.dll, xinput1_4.dll...) para cargar la real con el mismo nombre.
void ResolveOwnName() {
    wchar_t path[MAX_PATH] = {};
    if (GetModuleFileNameW(g_self, path, MAX_PATH) == 0)
        return;
    const wchar_t* name = wcsrchr(path, L'\\');
    name = name ? name + 1 : path;
    wcsncpy_s(g_realName, name, _TRUNCATE);
}

// Carga una DLL de System32 por ruta absoluta y comprueba que de verdad viene de allí (nunca esta misma
// copia ni otra copia del filtro con otro nombre, que ya está cargada en el proceso con ese nombre base).
HMODULE LoadFromSystem(const wchar_t* system, const wchar_t* name) {
    wchar_t full[MAX_PATH] = {};
    if (swprintf_s(full, L"%ls\\%ls", system, name) <= 0)
        return nullptr;
    HMODULE module = LoadLibraryExW(full, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!module)
        return nullptr;
    wchar_t loaded[MAX_PATH] = {};
    const DWORD length = GetModuleFileNameW(module, loaded, MAX_PATH);
    const size_t prefix = wcslen(system);
    const bool fromSystem = length > prefix && length < MAX_PATH && _wcsnicmp(loaded, system, prefix) == 0 && loaded[prefix] == L'\\';
    if (module == g_self || !fromSystem) {
        FreeLibrary(module);
        return nullptr;
    }
    return module;
}

HMODULE Real() {
    HMODULE module = g_real.load(std::memory_order_acquire);
    if (module)
        return module;
    wchar_t system[MAX_PATH] = {};
    const UINT length = GetSystemDirectoryW(system, MAX_PATH);
    if (length == 0 || length >= MAX_PATH - 20)
        return nullptr;
    module = LoadFromSystem(system, g_realName);
    if (!module) // p. ej. xinput1_3.dll no instalada: xinput1_4.dll viene con Windows 8 o posterior.
        module = LoadFromSystem(system, L"xinput1_4.dll");
    HMODULE expected = nullptr;
    if (!g_real.compare_exchange_strong(expected, module) && module)
        FreeLibrary(module); // Otro hilo ganó la carrera.
    return g_real.load(std::memory_order_acquire);
}

template <typename Fn>
Fn Resolve(const char* name) {
    HMODULE module = Real();
    return module ? reinterpret_cast<Fn>(reinterpret_cast<void*>(GetProcAddress(module, name))) : nullptr;
}

bool AssignmentPath(wchar_t* out, size_t size, const wchar_t* file) {
    wchar_t local[MAX_PATH] = {};
    const DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", local, MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
        return false;
    return swprintf_s(out, size, L"%ls\\SplitForge\\xinput\\%ls", local, file) > 0;
}

// Nombre base de esta copia sin extensión (xinput1_4.dll → xinput1_4) para su archivo de estado.
void BaseName(wchar_t* out, size_t size) {
    wcsncpy_s(out, size, g_realName, _TRUNCATE);
    wchar_t* dot = wcsrchr(out, L'.');
    if (dot)
        *dot = 0;
}

// Escribe el estado si cambió la ranura o, cada kStatusMs, si cambiaron las lecturas. Reemplazo atómico
// (temporal + MoveFileEx) para que SplitForge nunca lea un archivo a medias.
void MaybeWriteStatus(int slot, ULONGLONG now) {
    if (g_statusBusy.test_and_set(std::memory_order_acquire))
        return;
    const unsigned long long reads = g_reads.load(std::memory_order_relaxed);
    const bool dirty = g_statusDirty.exchange(false, std::memory_order_relaxed);
    const bool due = dirty || slot != g_writtenSlot || (reads != g_writtenReads && now - g_writtenAt >= kStatusMs);
    if (due) {
        wchar_t base[MAX_PATH] = {};
        BaseName(base, MAX_PATH);
        wchar_t file[MAX_PATH] = {};
        wchar_t path[MAX_PATH * 2] = {};
        wchar_t temp[MAX_PATH * 2] = {};
        FILE* f = nullptr;
        if (swprintf_s(file, L"estado-%lu-%ls.txt", GetCurrentProcessId(), base) > 0
            && AssignmentPath(path, MAX_PATH * 2, file)
            && swprintf_s(temp, L"%ls.tmp", path) > 0
            && _wfopen_s(&f, temp, L"w") == 0 && f) {
            fwprintf(f, L"dll=%ls\nreal=%ls\nranura=%d\nlecturas=%llu\nfoco=%ls\n", g_realName, Real() ? L"cargada" : L"NO",
                     slot, reads, g_focusActive.load(std::memory_order_relaxed) ? L"forzado" : L"no");
            const bool written = fclose(f) == 0;
            if (written && MoveFileExW(temp, path, MOVEFILE_REPLACE_EXISTING)) {
                g_writtenSlot = slot;
                g_writtenReads = reads;
                g_writtenAt = now;
            } else {
                DeleteFileW(temp);
            }
        }
    }
    g_statusBusy.clear(std::memory_order_release);
}

// --- Foco simulado (técnica ForceFocus / FocusHooks de Nucleus Co-op) ---
//
// Unity solo atiende la entrada de la ventana con foco (en 7DTD: GameManager.OnApplicationFocus →
// GameIsFocused). En pantalla dividida, cada cliente con mando asignado debe creer que siempre lo tiene:
//  - su ventana ignora las pérdidas de foco (WM_KILLFOCUS) y las desactivaciones se tratan como
//    activaciones (WM_ACTIVATE, WM_ACTIVATEAPP, WM_NCACTIVATE);
//  - GetForegroundWindow / GetActiveWindow / GetFocus de UnityPlayer.dll y del ejecutable devuelven su
//    ventana (se reescribe su tabla de importaciones: una escritura atómica de puntero, sin parchear código).
// Solo dentro de este proceso y solo si SplitForge le asignó un mando. Una sola vez por proceso aunque se
// carguen varias copias del filtro (xinput1_3 y xinput1_4).

std::atomic<HWND> g_window{nullptr};
WNDPROC g_originalProc = nullptr;
std::atomic<bool> g_focusStarted{false};

HWND WINAPI FocusedWindow() { return g_window.load(std::memory_order_relaxed); }

LRESULT CALLBACK FocusProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_KILLFOCUS:
        return 0;
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
            wParam = MAKEWPARAM(WA_ACTIVE, HIWORD(wParam));
        break;
    case WM_ACTIVATEAPP:
    case WM_NCACTIVATE:
        wParam = TRUE;
        break;
    default:
        break;
    }
    return CallWindowProcW(g_originalProc, hwnd, message, wParam, lParam);
}

struct FindWindowData {
    DWORD pid;
    bool anyWindow; // false: solo la ventana de Unity (UnityWndClass)
    HWND found;
};

BOOL CALLBACK FindOwnWindow(HWND hwnd, LPARAM lParam) {
    auto* data = reinterpret_cast<FindWindowData*>(lParam);
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid != data->pid || !IsWindowVisible(hwnd) || GetWindow(hwnd, GW_OWNER))
        return TRUE;
    wchar_t cls[64] = {};
    GetClassNameW(hwnd, cls, 64);
    if (wcscmp(cls, L"UnityWndClass") == 0) {
        data->found = hwnd;
        return FALSE;
    }
    if (data->anyWindow && !data->found)
        data->found = hwnd; // Juego que no es Unity: la primera ventana visible propia.
    return TRUE;
}

// Sustituye en la tabla de importaciones de `module` las funciones de foco de user32 por FocusedWindow.
void RedirectFocusImports(HMODULE module) {
    if (!module)
        return;
    auto* base = reinterpret_cast<BYTE*>(module);
    auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE)
        return;
    auto* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
    const auto& dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (!dir.VirtualAddress)
        return;
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (!user32)
        return;
    void* targets[] = {
        reinterpret_cast<void*>(GetProcAddress(user32, "GetForegroundWindow")),
        reinterpret_cast<void*>(GetProcAddress(user32, "GetActiveWindow")),
        reinterpret_cast<void*>(GetProcAddress(user32, "GetFocus")),
    };
    for (auto* imp = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base + dir.VirtualAddress); imp->Name; ++imp) {
        if (_stricmp(reinterpret_cast<const char*>(base + imp->Name), "user32.dll") != 0)
            continue;
        for (auto* thunk = reinterpret_cast<IMAGE_THUNK_DATA*>(base + imp->FirstThunk); thunk->u1.Function; ++thunk) {
            void* current = reinterpret_cast<void*>(thunk->u1.Function);
            for (void* target : targets) {
                if (!target || current != target)
                    continue;
                DWORD old = 0;
                if (VirtualProtect(&thunk->u1.Function, sizeof(void*), PAGE_READWRITE, &old)) {
                    InterlockedExchangePointer(reinterpret_cast<void* volatile*>(&thunk->u1.Function),
                                               reinterpret_cast<void*>(&FocusedWindow));
                    VirtualProtect(&thunk->u1.Function, sizeof(void*), old, &old);
                }
            }
        }
    }
}

// Hilo propio (nunca en DllMain): espera la ventana del juego y activa el foco simulado.
DWORD WINAPI FocusThread(void*) {
    FindWindowData data{GetCurrentProcessId(), false, nullptr};
    for (int attempt = 0; attempt < 480 && !data.found; ++attempt) { // hasta 2 minutos
        data.anyWindow = attempt >= 80; // tras 20 s sin ventana de Unity, se acepta cualquier ventana propia
        EnumWindows(&FindOwnWindow, reinterpret_cast<LPARAM>(&data));
        if (!data.found)
            Sleep(250);
    }
    if (!data.found)
        return 0;
    const HWND hwnd = data.found;
    g_window.store(hwnd, std::memory_order_relaxed);
    RedirectFocusImports(GetModuleHandleW(nullptr));
    RedirectFocusImports(GetModuleHandleW(L"UnityPlayer.dll"));
    g_originalProc = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hwnd, GWLP_WNDPROC));
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&FocusProc));
    g_focusActive.store(true, std::memory_order_relaxed);
    // Como el bucle de foco de Nucleus: unas cuantas activaciones para que el juego se dé por enfocado.
    for (int i = 0; i < 8; ++i) {
        SendMessageW(hwnd, WM_ACTIVATEAPP, TRUE, 0);
        SendMessageW(hwnd, WM_NCACTIVATE, TRUE, 0);
        SendMessageW(hwnd, WM_ACTIVATE, WA_ACTIVE, 0);
        SendMessageW(hwnd, WM_SETFOCUS, 0, 0);
        Sleep(25);
    }
    g_statusDirty.store(true, std::memory_order_relaxed); // reescribir el estado con «foco=forzado»
    return 0;
}

// Arranca el foco simulado una sola vez por proceso (aunque haya varias copias del filtro cargadas).
void StartFocusOnce() {
    bool expected = false;
    if (!g_focusStarted.compare_exchange_strong(expected, true))
        return;
    wchar_t name[64] = {};
    swprintf_s(name, L"Local\\SplitForgeFoco-%lu", GetCurrentProcessId());
    HANDLE guard = CreateMutexW(nullptr, FALSE, name);
    if (!guard || GetLastError() == ERROR_ALREADY_EXISTS)
        return; // Otra copia del filtro ya lo hace (el mutex se queda abierto mientras viva el proceso).
    HANDLE thread = CreateThread(nullptr, 0, &FocusThread, nullptr, 0, nullptr);
    if (thread)
        CloseHandle(thread);
}

// Relee la asignación como mucho una vez por segundo. Formato: "PID RANURA" por línea (ranura 0–3).
int CurrentSlot() {
    const ULONGLONG now = GetTickCount64();
    ULONGLONG next = g_nextReload.load(std::memory_order_relaxed);
    if (now < next || !g_nextReload.compare_exchange_strong(next, now + kReloadMs))
        return g_slot.load(std::memory_order_relaxed);
    wchar_t path[MAX_PATH * 2] = {};
    int slot = kPassThrough;
    FILE* f = nullptr;
    if (AssignmentPath(path, MAX_PATH * 2, L"asignacion.txt") && _wfopen_s(&f, path, L"r") == 0 && f) {
        const unsigned long self = GetCurrentProcessId();
        unsigned long pid = 0;
        int value = 0;
        while (fscanf_s(f, "%lu %d", &pid, &value) == 2) {
            if (pid == self && value >= 0 && value < kMaxSlots) {
                slot = value;
                break;
            }
        }
        fclose(f);
    }
    g_slot.store(slot, std::memory_order_relaxed);
    if (slot != kPassThrough)
        StartFocusOnce(); // pantalla dividida: este cliente debe creer que siempre tiene el foco
    MaybeWriteStatus(slot, now);
    return slot;
}

// Índice que pide el juego → ranura física. Con asignación, solo el índice 0 existe.
bool Map(DWORD userIndex, DWORD* physical) {
    if (userIndex >= kMaxSlots)
        return false;
    const int slot = CurrentSlot();
    if (slot == kPassThrough) {
        *physical = userIndex;
        return true;
    }
    if (userIndex != 0)
        return false;
    *physical = static_cast<DWORD>(slot);
    return true;
}

} // namespace

extern "C" {

DWORD WINAPI SF_XInputGetState(DWORD userIndex, XInputState* state) {
    static auto fn = Resolve<GetStateFn>("XInputGetState");
    g_reads.fetch_add(1, std::memory_order_relaxed);
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(physical, state);
}

// Ordinal 100: igual que XInputGetState pero incluye el botón Guía. Muchos juegos lo usan.
DWORD WINAPI SF_XInputGetStateEx(DWORD userIndex, XInputState* state) {
    static auto fn = Resolve<GetStateFn>(MAKEINTRESOURCEA(100));
    static auto fallback = Resolve<GetStateFn>("XInputGetState");
    g_reads.fetch_add(1, std::memory_order_relaxed);
    DWORD physical = 0;
    if (!Map(userIndex, &physical))
        return kNotConnected;
    if (fn)
        return fn(physical, state);
    return fallback ? fallback(physical, state) : kNotConnected;
}

DWORD WINAPI SF_XInputSetState(DWORD userIndex, XInputVibration* vibration) {
    static auto fn = Resolve<SetStateFn>("XInputSetState");
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(physical, vibration);
}

DWORD WINAPI SF_XInputGetCapabilities(DWORD userIndex, DWORD flags, XInputCapabilities* capabilities) {
    static auto fn = Resolve<GetCapabilitiesFn>("XInputGetCapabilities");
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(physical, flags, capabilities);
}

void WINAPI SF_XInputEnable(BOOL enable) {
    static auto fn = Resolve<EnableFn>("XInputEnable");
    if (fn)
        fn(enable);
}

DWORD WINAPI SF_XInputGetDSoundAudioDeviceGuids(DWORD userIndex, GUID* render, GUID* capture) {
    static auto fn = Resolve<GetDSoundGuidsFn>("XInputGetDSoundAudioDeviceGuids");
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(physical, render, capture);
}

DWORD WINAPI SF_XInputGetBatteryInformation(DWORD userIndex, BYTE devType, XInputBatteryInformation* info) {
    static auto fn = Resolve<GetBatteryFn>("XInputGetBatteryInformation");
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(physical, devType, info);
}

DWORD WINAPI SF_XInputGetKeystroke(DWORD userIndex, DWORD reserved, XInputKeystroke* keystroke) {
    static auto fn = Resolve<GetKeystrokeFn>("XInputGetKeystroke");
    if (!fn)
        return kNotConnected;
    const int slot = CurrentSlot();
    if (slot == kPassThrough)
        return fn(userIndex, reserved, keystroke);
    // XUSER_INDEX_ANY (0xFF) o 0: solo la ranura asignada, y se informa como índice 0.
    if (userIndex != 0 && userIndex != 0xFF)
        return kNotConnected;
    const DWORD result = fn(static_cast<DWORD>(slot), reserved, keystroke);
    if (result == ERROR_SUCCESS && keystroke)
        keystroke->userIndex = 0;
    return result;
}

DWORD WINAPI SF_XInputGetAudioDeviceIds(DWORD userIndex, LPWSTR render, UINT* renderCount, LPWSTR capture, UINT* captureCount) {
    static auto fn = Resolve<GetAudioIdsFn>("XInputGetAudioDeviceIds");
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(physical, render, renderCount, capture, captureCount);
}

DWORD WINAPI SF_XInputWaitForGuideButton(DWORD userIndex, DWORD flags, void* listen) {
    static auto fn = Resolve<WaitGuideFn>(MAKEINTRESOURCEA(101));
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(physical, flags, listen);
}

DWORD WINAPI SF_XInputCancelGuideButtonWait(DWORD userIndex) {
    static auto fn = Resolve<CancelGuideFn>(MAKEINTRESOURCEA(102));
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(physical);
}

DWORD WINAPI SF_XInputPowerOffController(DWORD userIndex) {
    static auto fn = Resolve<PowerOffFn>(MAKEINTRESOURCEA(103));
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(physical);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        // Nada de E/S ni de cargar librerías aquí (bloqueo del cargador): solo datos propios.
        g_self = instance;
        DisableThreadLibraryCalls(instance);
        ResolveOwnName();
    }
    return TRUE;
}

} // extern "C"
