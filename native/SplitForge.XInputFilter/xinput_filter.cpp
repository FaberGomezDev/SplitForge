// SplitForge · Filtro XInput por proceso (EXPERIMENTAL)
//
// Se copia en la carpeta de un juego con el nombre de la DLL de XInput que el juego carga
// (xinput1_3.dll, xinput1_4.dll o xinput9_1_0.dll). Reenvía todas las llamadas a la DLL real
// de System32, pero hace que ESTE proceso vea un único mando: la ranura física que SplitForge
// le asigna en %LOCALAPPDATA%\SplitForge\xinput\asignacion.txt (líneas "PID RANURA").
// Sin asignación para el PID, es transparente.
//
// No inyecta código en otros procesos ni toca DRM/antitrampas. Úsese solo con EAC desactivado
// mediante la opción oficial del juego.

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
std::atomic<bool> g_statusWritten{false};
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

HMODULE Real() {
    HMODULE module = g_real.load(std::memory_order_acquire);
    if (module)
        return module;
    wchar_t system[MAX_PATH] = {};
    const UINT length = GetSystemDirectoryW(system, MAX_PATH);
    if (length == 0 || length >= MAX_PATH - 20)
        return nullptr;
    wchar_t full[MAX_PATH] = {};
    swprintf_s(full, L"%ls\\%ls", system, g_realName);
    // Ruta absoluta de System32: nunca se carga a sí misma.
    module = LoadLibraryExW(full, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!module)
        module = LoadLibraryExW(L"xinput1_4.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
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

void WriteStatus(int slot) {
    bool expected = false;
    if (!g_statusWritten.compare_exchange_strong(expected, true))
        return;
    wchar_t file[64] = {};
    swprintf_s(file, L"estado-%lu.txt", GetCurrentProcessId());
    wchar_t path[MAX_PATH * 2] = {};
    if (!AssignmentPath(path, MAX_PATH * 2, file))
        return;
    FILE* f = nullptr;
    if (_wfopen_s(&f, path, L"w") != 0 || !f)
        return;
    fwprintf(f, L"dll=%ls\nreal=%ls\nranura=%d\n", g_realName, Real() ? L"cargada" : L"NO", slot);
    fclose(f);
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
    WriteStatus(slot);
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
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(physical, state);
}

// Ordinal 100: igual que XInputGetState pero incluye el botón Guía. Muchos juegos lo usan.
DWORD WINAPI SF_XInputGetStateEx(DWORD userIndex, XInputState* state) {
    static auto fn = Resolve<GetStateFn>(MAKEINTRESOURCEA(100));
    static auto fallback = Resolve<GetStateFn>("XInputGetState");
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
