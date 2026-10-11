// SplitForge · Filtro XInput por proceso (EXPERIMENTAL)
//
// Se copia en la carpeta de un juego con el nombre de la DLL de XInput que el juego carga
// (xinput1_3.dll, xinput1_4.dll o xinput9_1_0.dll). Reenvía todas las llamadas a la XInput real
// de System32, pero hace que ESTE proceso vea un único mando: la ranura física que SplitForge
// le asigna en %LOCALAPPDATA%\SplitForge\xinput\asignacion.txt (líneas "PID RANURA").
// Sin asignación para el PID, es transparente.
//
// XInput real: xinput1_4.dll de System32, la única implementación desde Windows 8 (en Windows 11
// recientes, XInput1_3.dll y XInput9_1_0.dll de System32 solo la cargan y le reenvían). Al cargarla, el
// filtro redirige además sus exportaciones hacia sí mismo (ver «Intercepción»): quien la pida después
// sin pasar por la copia del filtro (en 7DTD, InControlNative) también ve un único mando.
//
// Estado: cada copia cargada escribe estado-PID-<dll>.txt con la ranura vigente, cuántas lecturas
// (XInputGetState/Ex) ha atendido, cuántas de ellas llegaron por la DLL real interceptada, si la
// intercepción está activa y si el foco está forzado. Se reescribe al cambiar la asignación y, como mucho,
// cada 2 s mientras el juego lee mandos.
//
// Con mando asignado, además simula el foco de su ventana (ver «Foco simulado»): en pantalla dividida
// cada cliente atiende su mando aunque no sea la ventana activa. Y, mientras no es la ventana activa de
// verdad, usa un puntero propio (ver «Puntero propio»): no mueve ni encierra el puntero de Windows; al dejar
// de serlo, se le sueltan las teclas que tenía pulsadas (ver «Teclas pegadas»).
//
// Diagnóstico: un «mando de prueba» opcional (prueba.txt) pulsa botones para ensayar sin manos.
//
// No inyecta código en otros procesos. Úsese con el juego abierto sin EAC (opción oficial del juego).

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstring>
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
using GetBusInfoFn = DWORD(WINAPI*)(DWORD, void*);
using GetCapabilitiesExFn = DWORD(WINAPI*)(DWORD, DWORD, DWORD, void*);
using IndexedFn = DWORD(WINAPI*)(DWORD, void*);

// Funciones de la XInput real que el filtro reenvía.
enum RouteId {
    kGetState, kGetStateEx, kSetState, kGetCapabilities, kEnable, kGetDSoundGuids, kGetBattery, kGetKeystroke,
    kGetAudioIds, kWaitGuide, kCancelGuide, kPowerOff, kGetBusInfo, kGetCapabilitiesEx, kOrdinal109, kRouteCount
};

enum Interception : int { kNotIntercepted, kIntercepted, kDelegated };

HMODULE g_self = nullptr;
std::atomic<HMODULE> g_real{nullptr};
void* g_original[kRouteCount] = {};                // Direcciones reales; se rellenan una sola vez en LoadBackend().
std::atomic<int> g_interception{kNotIntercepted};
std::atomic<bool> g_realPreloaded{false};          // La DLL real ya estaba en el proceso antes de interceptarla.
std::atomic<unsigned long long> g_hookedReads{0};  // Lecturas que llegaron por las exportaciones de la DLL real.
std::atomic<int> g_slot{kPassThrough};
std::atomic<ULONGLONG> g_nextReload{0};
std::atomic<unsigned long long> g_reads{0}; // Llamadas a XInputGetState/Ex atendidas por esta copia.
std::atomic_flag g_statusBusy; // C++20: empieza libre.
int g_writtenSlot = -2;                       // Último estado escrito (solo con g_statusBusy tomado).
unsigned long long g_writtenReads = ~0ull;
ULONGLONG g_writtenAt = 0;
std::atomic<bool> g_statusDirty{false};        // Algo cambió (p. ej. el foco): reescribir el estado ya.
std::atomic<bool> g_focusActive{false};
std::atomic<bool> g_cursorOwn{false};                 // Puntero propio instalado en este proceso.
std::atomic<unsigned long long> g_cursorVirtual{0};   // Llamadas de puntero atendidas sin tocar el de Windows.
std::atomic<unsigned long long> g_keysReleased{0};    // Teclas soltadas por el filtro al perder el foco de verdad.
std::atomic<bool> g_testFile{false};                  // Existe prueba.txt (se mira una vez por segundo).
std::atomic<WORD> g_testButtons{0};                   // Botones que el mando de prueba mantiene pulsados.
std::atomic<DWORD> g_testSerial{0};                   // Cambia con cada cambio del mando de prueba.
std::atomic<bool> g_testKeepKeys{false};              // Prueba: no soltar las teclas (reproduce el fallo).
std::atomic<bool> g_testForeground{false};            // Prueba: tratar la ventana como la activa de verdad.
std::atomic<ULONGLONG> g_nextTestRead{0};
std::atomic_flag g_testBusy;
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
// `alreadyLoaded`: si esa DLL ya estaba cargada en el proceso antes de esta llamada.
HMODULE LoadFromSystem(const wchar_t* system, const wchar_t* name, bool* alreadyLoaded) {
    wchar_t full[MAX_PATH] = {};
    if (swprintf_s(full, L"%ls\\%ls", system, name) <= 0)
        return nullptr;
    HMODULE before = nullptr;
    *alreadyLoaded = GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, full, &before) && before;
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

void LoadBackend(); // Al final del archivo: necesita las funciones exportadas.

// Carga la XInput real e instala la intercepción una sola vez (inicialización estática, segura entre hilos).
// Nunca en DllMain: la dispara la primera llamada del juego a cualquier función del filtro.
void EnsureBackend() {
    static const bool loaded = (LoadBackend(), true);
    static_cast<void>(loaded);
}

// Dirección real de una función de XInput (nullptr si la DLL real no la tiene).
template <typename Fn>
Fn Original(RouteId id) {
    EnsureBackend();
    return reinterpret_cast<Fn>(g_original[id]);
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
            static const wchar_t* const interception[] = {L"no", L"activa", L"delegada"};
            fwprintf(f, L"dll=%ls\nreal=%ls\nranura=%d\nlecturas=%llu\nfoco=%ls\nintercepcion=%ls\ninterceptadas=%llu\nprevia=%ls\n"
                        L"puntero=%ls\npuntero_propio=%llu\nteclas_soltadas=%llu\n",
                     g_realName, g_real.load(std::memory_order_acquire) ? L"cargada" : L"NO", slot, reads,
                     g_focusActive.load(std::memory_order_relaxed) ? L"forzado" : L"no",
                     interception[g_interception.load(std::memory_order_relaxed)],
                     g_hookedReads.load(std::memory_order_relaxed),
                     g_realPreloaded.load(std::memory_order_relaxed) ? L"si" : L"no",
                     g_cursorOwn.load(std::memory_order_relaxed) ? L"propio" : L"no",
                     g_cursorVirtual.load(std::memory_order_relaxed),
                     g_keysReleased.load(std::memory_order_relaxed));
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

// --- Puntero propio (como el puntero falso y los ganchos de cursor de ProtoInput en Nucleus Co-op) ---
//
// Windows tiene UN puntero. Con el foco simulado, cada cliente cree estar en primer plano, y Unity
// (ScreenManagerWin::SetLockCursorInternal) lo lleva al centro de su ventana y lo encierra allí con
// ClipCursor; además lee su posición en cada fotograma (GetCursorPos), y 7DTD coloca el puntero de sus
// menús donde esté el de Windows en cuanto cambia (SoftCursor.HandleMovement). Con dos clientes, cada uno
// le quita el puntero al otro. Por eso el cliente que NO es la ventana activa de verdad usa uno propio:
//  - GetCursorPos devuelve su última posición conocida (o el centro de su ventana), que no cambia aunque
//    se mueva el puntero de Windows;
//  - SetCursorPos solo la recuerda y ClipCursor no hace nada.
// El cliente que sí es la ventana activa usa el puntero real, como siempre (ratón y teclado intactos).
// Igual que el foco: se reescribe la tabla de importaciones de UnityPlayer.dll y del ejecutable.

std::atomic<bool> g_cursorKnown{false};
std::atomic<LONG> g_cursorX{0};
std::atomic<LONG> g_cursorY{0};
SRWLOCK g_clipLock = SRWLOCK_INIT;
bool g_clipHeld = false; // Este proceso encerró el puntero real (con g_clipLock tomado).
RECT g_clipRect{};       // Rectángulo que Windows aplicó entonces.

void RememberCursor(LONG x, LONG y) {
    g_cursorX.store(x, std::memory_order_relaxed);
    g_cursorY.store(y, std::memory_order_relaxed);
    g_cursorKnown.store(true, std::memory_order_relaxed);
}

// ¿La ventana activa de verdad es de este proceso? (Las importaciones del propio filtro no se redirigen.)
bool OwnsRealForeground() {
    if (g_testForeground.load(std::memory_order_relaxed))
        return true;
    const HWND foreground = GetForegroundWindow();
    if (!foreground)
        return false;
    if (foreground == g_window.load(std::memory_order_relaxed))
        return true;
    DWORD pid = 0;
    GetWindowThreadProcessId(foreground, &pid);
    return pid == GetCurrentProcessId();
}

BOOL WINAPI Own_GetCursorPos(POINT* point) {
    if (!point || OwnsRealForeground()) {
        const BOOL ok = GetCursorPos(point);
        if (ok && point)
            RememberCursor(point->x, point->y);
        return ok;
    }
    g_cursorVirtual.fetch_add(1, std::memory_order_relaxed);
    if (!g_cursorKnown.load(std::memory_order_relaxed)) {
        // Nunca fue la ventana activa ni fijó el puntero: el centro de su ventana.
        POINT center{};
        RECT client{};
        const HWND hwnd = g_window.load(std::memory_order_relaxed);
        if (hwnd && GetClientRect(hwnd, &client)) {
            center = {(client.left + client.right) / 2, (client.top + client.bottom) / 2};
            ClientToScreen(hwnd, &center);
        }
        RememberCursor(center.x, center.y);
    }
    point->x = g_cursorX.load(std::memory_order_relaxed);
    point->y = g_cursorY.load(std::memory_order_relaxed);
    return TRUE;
}

BOOL WINAPI Own_SetCursorPos(int x, int y) {
    if (OwnsRealForeground()) {
        const BOOL ok = SetCursorPos(x, y);
        if (ok)
            RememberCursor(x, y);
        return ok;
    }
    g_cursorVirtual.fetch_add(1, std::memory_order_relaxed);
    RememberCursor(x, y);
    return TRUE;
}

BOOL WINAPI Own_ClipCursor(const RECT* rect) {
    if (!OwnsRealForeground()) {
        g_cursorVirtual.fetch_add(1, std::memory_order_relaxed);
        return TRUE;
    }
    const BOOL ok = ClipCursor(rect);
    AcquireSRWLockExclusive(&g_clipLock);
    g_clipHeld = ok && rect && GetClipCursor(&g_clipRect);
    ReleaseSRWLockExclusive(&g_clipLock);
    return ok;
}

// Al dejar de ser la ventana activa: si el encierro vigente es el que puso este proceso, se suelta. Nunca se
// quita el de otro (el siguiente cliente activo pone el suyo).
void ReleaseClipIfBackground() {
    if (OwnsRealForeground())
        return;
    AcquireSRWLockExclusive(&g_clipLock);
    const bool held = g_clipHeld;
    const RECT ours = g_clipRect;
    g_clipHeld = false;
    ReleaseSRWLockExclusive(&g_clipLock);
    RECT current{};
    if (held && GetClipCursor(&current) && EqualRect(&current, &ours))
        ClipCursor(nullptr);
}

// --- Teclas pegadas ---
//
// Unity suelta todas las teclas cuando su ventana se desactiva (win::Input::Activate(false)). El foco simulado
// le oculta esa desactivación, así que la tecla que estaba pulsada al cambiar de ventana se queda pulsada
// para el juego: su «soltar» llega a la otra ventana. Con Alt+Tab se queda Alt, y 7DTD con Alt pulsada hace
// que LB/RB pasen de página en el cinturón en vez de cambiar de objeto (XUiC_Radial.HandleToolbeltCommand →
// InputUtils.AltKeyPressed). Por eso el filtro apunta las teclas que el juego vio pulsar y, cuando la ventana
// deja de ser la activa de verdad, le entrega el «soltar» de cada una. Además, GetAsyncKeyState y GetKeyState
// del juego responden «sin pulsar» mientras no es la ventana activa (Unity los consulta para Mayús).
// Una pulsación que llega cuando la ventana ya no es la activa (una repetición de Alt que quedó en cola: Unity
// vuelve a darse el foco de teclado con SetFocus al «activarse») no se entrega: nadie la soltaría.
// Solo se tocan desde el hilo de la ventana.

BYTE g_keysDown[256] = {};    // Bit 0: tecla normal; bit 1: tecla extendida (Alt o Ctrl derechos, flechas...).
BYTE g_keyScan[256][2] = {};  // Código de exploración con el que se pulsó.

void TrackKey(UINT message, WPARAM key, LPARAM lParam) {
    if (key >= 256)
        return;
    const int extended = (lParam >> 24) & 1;
    const BYTE bit = static_cast<BYTE>(1 << extended);
    if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN) {
        g_keysDown[key] = static_cast<BYTE>(g_keysDown[key] | bit);
        g_keyScan[key][extended] = static_cast<BYTE>((lParam >> 16) & 0xFF);
    } else {
        g_keysDown[key] = static_cast<BYTE>(g_keysDown[key] & ~bit);
    }
}

SHORT WINAPI Own_GetAsyncKeyState(int key) { return OwnsRealForeground() ? GetAsyncKeyState(key) : 0; }
SHORT WINAPI Own_GetKeyState(int key) { return OwnsRealForeground() ? GetKeyState(key) : 0; }

// La ventana dejó de ser la activa de verdad: el juego recibe el «soltar» de cada tecla que tenía pulsada.
void ReleaseHeldKeysIfBackground(HWND hwnd) {
    if (OwnsRealForeground() || g_testKeepKeys.load(std::memory_order_relaxed))
        return;
    for (UINT key = 0; key < 256; ++key) {
        for (int extended = 0; extended < 2; ++extended) {
            const BYTE bit = static_cast<BYTE>(1 << extended);
            if (!(g_keysDown[key] & bit))
                continue;
            g_keysDown[key] = static_cast<BYTE>(g_keysDown[key] & ~bit);
            // Como un WM_KEYUP de verdad: repetición 1, estado previo «pulsada», transición «soltar».
            const DWORD flags = 0xC0000001u | (static_cast<DWORD>(g_keyScan[key][extended]) << 16)
                                | (static_cast<DWORD>(extended) << 24);
            CallWindowProcW(g_originalProc, hwnd, WM_KEYUP, key, static_cast<LPARAM>(flags));
            g_keysReleased.fetch_add(1, std::memory_order_relaxed);
        }
    }
}

LRESULT CALLBACK FocusProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (!OwnsRealForeground() && !g_testKeepKeys.load(std::memory_order_relaxed))
            return 0; // Pulsación atrasada: la ventana ya no es la activa y no recibirá el «soltar».
        TrackKey(message, wParam, lParam);
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        TrackKey(message, wParam, lParam);
        break;
    case WM_KILLFOCUS:
        ReleaseClipIfBackground();
        ReleaseHeldKeysIfBackground(hwnd);
        return 0;
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE) {
            ReleaseClipIfBackground();
            ReleaseHeldKeysIfBackground(hwnd);
            wParam = MAKEWPARAM(WA_ACTIVE, HIWORD(wParam));
        }
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
    if (data->anyWindow && !data->found) {
        // Juego que no es de Unity: su primera ventana visible de tamaño de juego (no un aviso ni una auxiliar).
        RECT rect{};
        if (GetWindowRect(hwnd, &rect) && rect.right - rect.left >= 320 && rect.bottom - rect.top >= 200)
            data->found = hwnd;
    }
    return TRUE;
}

struct ImportHook {
    const char* name;  // Función de user32.
    void* replacement; // Función del filtro que la sustituye.
};

constexpr size_t kMaxImportHooks = 8;

// Sustituye en la tabla de importaciones de `module` las funciones de user32 indicadas por las del filtro.
void RedirectImports(HMODULE module, const ImportHook* hooks, size_t count) {
    if (!module || count > kMaxImportHooks)
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
    void* targets[kMaxImportHooks] = {};
    for (size_t i = 0; i < count; ++i)
        targets[i] = reinterpret_cast<void*>(GetProcAddress(user32, hooks[i].name));
    for (auto* imp = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base + dir.VirtualAddress); imp->Name; ++imp) {
        if (_stricmp(reinterpret_cast<const char*>(base + imp->Name), "user32.dll") != 0)
            continue;
        for (auto* thunk = reinterpret_cast<IMAGE_THUNK_DATA*>(base + imp->FirstThunk); thunk->u1.Function; ++thunk) {
            void* current = reinterpret_cast<void*>(thunk->u1.Function);
            for (size_t i = 0; i < count; ++i) {
                if (!targets[i] || current != targets[i])
                    continue;
                DWORD old = 0;
                if (VirtualProtect(&thunk->u1.Function, sizeof(void*), PAGE_READWRITE, &old)) {
                    InterlockedExchangePointer(reinterpret_cast<void* volatile*>(&thunk->u1.Function), hooks[i].replacement);
                    VirtualProtect(&thunk->u1.Function, sizeof(void*), old, &old);
                }
                break;
            }
        }
    }
}

// Hilo propio (nunca en DllMain): espera la ventana del juego y activa el foco simulado.
DWORD WINAPI FocusThread(void*) {
    FindWindowData data{GetCurrentProcessId(), false, nullptr};
    // Con Unity se espera a su ventana (20 s de margen); en otros motores (No Man's Sky usa GLFW) se acepta
    // la ventana principal del proceso a los 3 s.
    const int anyWindowAfter = GetModuleHandleW(L"UnityPlayer.dll") ? 80 : 12;
    for (int attempt = 0; attempt < 480 && !data.found; ++attempt) { // hasta 2 minutos
        data.anyWindow = attempt >= anyWindowAfter;
        EnumWindows(&FindOwnWindow, reinterpret_cast<LPARAM>(&data));
        if (!data.found)
            Sleep(250);
    }
    if (!data.found)
        return 0;
    const HWND hwnd = data.found;
    g_window.store(hwnd, std::memory_order_relaxed);
    const ImportHook hooks[] = {
        {"GetForegroundWindow", reinterpret_cast<void*>(&FocusedWindow)},
        {"GetActiveWindow", reinterpret_cast<void*>(&FocusedWindow)},
        {"GetFocus", reinterpret_cast<void*>(&FocusedWindow)},
        {"GetCursorPos", reinterpret_cast<void*>(&Own_GetCursorPos)},
        {"SetCursorPos", reinterpret_cast<void*>(&Own_SetCursorPos)},
        {"ClipCursor", reinterpret_cast<void*>(&Own_ClipCursor)},
        {"GetAsyncKeyState", reinterpret_cast<void*>(&Own_GetAsyncKeyState)},
        {"GetKeyState", reinterpret_cast<void*>(&Own_GetKeyState)},
    };
    RedirectImports(GetModuleHandleW(nullptr), hooks, sizeof(hooks) / sizeof(hooks[0]));
    RedirectImports(GetModuleHandleW(L"UnityPlayer.dll"), hooks, sizeof(hooks) / sizeof(hooks[0]));
    g_cursorOwn.store(true, std::memory_order_relaxed);
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

// --- Mando de prueba (diagnóstico) ---
//
// Para ensayar sin manos. Si existe %LOCALAPPDATA%\SplitForge\xinput\prueba.txt, cada línea
// "PID BOTONES_HEX [opciones]" hace que el mando del proceso PID tenga pulsados esos botones (máscara
// wButtons de XInput) mientras la línea siga en el archivo; si su ranura no tiene mando, se simula uno en
// reposo. Opciones, solo para pruebas: «sin_soltar» desactiva la corrección de las teclas pegadas (reproduce
// ese fallo) y «activa» trata la ventana como la activa de verdad (para probar sin robar el foco a nadie).
// Sin el archivo no hace nada: solo se mira si existe una vez por segundo.

constexpr ULONGLONG kTestReloadMs = 30;
std::atomic<bool> g_testListed{false}; // Este proceso tiene línea en prueba.txt.

void SetTestPad(bool listed, WORD buttons, bool keepKeys, bool foreground) {
    const bool changed = g_testButtons.exchange(buttons, std::memory_order_relaxed) != buttons;
    if (g_testListed.exchange(listed, std::memory_order_relaxed) != listed || changed)
        g_testSerial.fetch_add(1, std::memory_order_relaxed); // Los juegos solo miran el estado si cambia el paquete.
    g_testKeepKeys.store(keepKeys, std::memory_order_relaxed);
    g_testForeground.store(foreground, std::memory_order_relaxed);
}

void ReloadTestPad(ULONGLONG now) {
    if (now < g_nextTestRead.load(std::memory_order_relaxed) || g_testBusy.test_and_set(std::memory_order_acquire))
        return;
    g_nextTestRead.store(now + kTestReloadMs, std::memory_order_relaxed);
    bool listed = false;
    bool keepKeys = false;
    bool foreground = false;
    WORD buttons = 0;
    wchar_t path[MAX_PATH * 2] = {};
    FILE* f = nullptr;
    const errno_t opened = AssignmentPath(path, MAX_PATH * 2, L"prueba.txt") ? _wfopen_s(&f, path, L"r") : ENOENT;
    if (opened != 0 && opened != ENOENT) {
        // Lo están reescribiendo en este instante: se conserva lo anterior (si no, el botón «parpadearía»).
        g_testBusy.clear(std::memory_order_release);
        return;
    }
    if (opened == 0 && f) {
        const unsigned long self = GetCurrentProcessId();
        char line[128] = {};
        while (!listed && fgets(line, sizeof(line), f)) {
            unsigned long pid = 0;
            unsigned int mask = 0;
            if (sscanf_s(line, "%lu %x", &pid, &mask) == 2 && pid == self) {
                listed = true;
                buttons = static_cast<WORD>(mask);
                keepKeys = std::strstr(line, "sin_soltar") != nullptr;
                foreground = std::strstr(line, "activa") != nullptr;
            }
        }
        fclose(f);
    }
    SetTestPad(listed, buttons, keepKeys, foreground);
    g_testBusy.clear(std::memory_order_release);
}

// ¿Aplica el mando de prueba a esta copia? Solo a la que cambia el índice (la delegada reenvía a ella).
bool TestPadActive() {
    if (!g_testFile.load(std::memory_order_relaxed))
        return false;
    if (g_slot.load(std::memory_order_relaxed) == kPassThrough
        || g_interception.load(std::memory_order_relaxed) == kDelegated)
        return false;
    ReloadTestPad(GetTickCount64());
    return g_testListed.load(std::memory_order_relaxed);
}

// Añade al estado leído los botones del mando de prueba; sin mando real en la ranura, uno en reposo.
DWORD WithTestPad(DWORD result, XInputState* state) {
    if (!state || !TestPadActive())
        return result;
    if (result != ERROR_SUCCESS)
        *state = XInputState{};
    state->gamepad.buttons = static_cast<WORD>(state->gamepad.buttons | g_testButtons.load(std::memory_order_relaxed));
    state->packetNumber += g_testSerial.load(std::memory_order_relaxed);
    return ERROR_SUCCESS;
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
    const bool testFile = AssignmentPath(path, MAX_PATH * 2, L"prueba.txt") && GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
    if (g_testFile.exchange(testFile, std::memory_order_relaxed) && !testFile)
        SetTestPad(false, 0, false, false); // Se retiró el archivo: el mando de prueba deja de existir.
    if (slot != kPassThrough)
        StartFocusOnce(); // pantalla dividida: este cliente debe creer que siempre tiene el foco
    MaybeWriteStatus(slot, now);
    return slot;
}

// Ranura que aplica ESTA copia. Si otra copia del filtro ya intercepta la DLL real, las llamadas de esta
// llegan a aquella y es ella quien cambia el índice (hacerlo dos veces ocultaría el mando).
int MappingSlot() {
    const int slot = CurrentSlot();
    return g_interception.load(std::memory_order_relaxed) == kDelegated ? kPassThrough : slot;
}

// Índice que pide el juego → ranura física. Con asignación, solo el índice 0 existe.
bool Map(DWORD userIndex, DWORD* physical) {
    if (userIndex >= kMaxSlots)
        return false;
    const int slot = MappingSlot();
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
    const auto fn = Original<GetStateFn>(kGetState);
    g_reads.fetch_add(1, std::memory_order_relaxed);
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return WithTestPad(fn(physical, state), state);
}

// Ordinal 100: igual que XInputGetState pero incluye el botón Guía. Muchos juegos lo usan.
DWORD WINAPI SF_XInputGetStateEx(DWORD userIndex, XInputState* state) {
    const auto fn = Original<GetStateFn>(kGetStateEx);
    const auto fallback = Original<GetStateFn>(kGetState);
    g_reads.fetch_add(1, std::memory_order_relaxed);
    DWORD physical = 0;
    if (!Map(userIndex, &physical))
        return kNotConnected;
    if (fn)
        return WithTestPad(fn(physical, state), state);
    return fallback ? WithTestPad(fallback(physical, state), state) : kNotConnected;
}

DWORD WINAPI SF_XInputSetState(DWORD userIndex, XInputVibration* vibration) {
    const auto fn = Original<SetStateFn>(kSetState);
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(physical, vibration);
}

DWORD WINAPI SF_XInputGetCapabilities(DWORD userIndex, DWORD flags, XInputCapabilities* capabilities) {
    const auto fn = Original<GetCapabilitiesFn>(kGetCapabilities);
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    const DWORD result = fn(physical, flags, capabilities);
    if (result == ERROR_SUCCESS || !capabilities || !TestPadActive())
        return result;
    // Mando de prueba sin mando real en la ranura: un mando estándar completo.
    *capabilities = XInputCapabilities{};
    capabilities->type = 1;    // XINPUT_DEVTYPE_GAMEPAD
    capabilities->subType = 1; // XINPUT_DEVSUBTYPE_GAMEPAD
    capabilities->gamepad = {0xF3FF, 255, 255, 32767, 32767, 32767, 32767};
    capabilities->vibration = {0xFFFF, 0xFFFF};
    return ERROR_SUCCESS;
}

void WINAPI SF_XInputEnable(BOOL enable) {
    const auto fn = Original<EnableFn>(kEnable);
    if (fn)
        fn(enable);
}

DWORD WINAPI SF_XInputGetDSoundAudioDeviceGuids(DWORD userIndex, GUID* render, GUID* capture) {
    const auto fn = Original<GetDSoundGuidsFn>(kGetDSoundGuids);
    DWORD physical = 0;
    if (!Map(userIndex, &physical))
        return kNotConnected;
    if (fn)
        return fn(physical, render, capture);
    // xinput1_4 ya no tiene esta función (es de xinput1_3): mando conectado, sin dispositivo de audio.
    const auto getState = Original<GetStateFn>(kGetState);
    XInputState state{};
    if (!getState || getState(physical, &state) != ERROR_SUCCESS)
        return kNotConnected;
    if (render)
        *render = GUID{};
    if (capture)
        *capture = GUID{};
    return ERROR_SUCCESS;
}

DWORD WINAPI SF_XInputGetBatteryInformation(DWORD userIndex, BYTE devType, XInputBatteryInformation* info) {
    const auto fn = Original<GetBatteryFn>(kGetBattery);
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(physical, devType, info);
}

DWORD WINAPI SF_XInputGetKeystroke(DWORD userIndex, DWORD reserved, XInputKeystroke* keystroke) {
    const auto fn = Original<GetKeystrokeFn>(kGetKeystroke);
    if (!fn)
        return kNotConnected;
    const int slot = MappingSlot();
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
    const auto fn = Original<GetAudioIdsFn>(kGetAudioIds);
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(physical, render, renderCount, capture, captureCount);
}

DWORD WINAPI SF_XInputWaitForGuideButton(DWORD userIndex, DWORD flags, void* listen) {
    const auto fn = Original<WaitGuideFn>(kWaitGuide);
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(physical, flags, listen);
}

DWORD WINAPI SF_XInputCancelGuideButtonWait(DWORD userIndex) {
    const auto fn = Original<CancelGuideFn>(kCancelGuide);
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(physical);
}

DWORD WINAPI SF_XInputPowerOffController(DWORD userIndex) {
    const auto fn = Original<PowerOffFn>(kPowerOff);
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(physical);
}

// Ordinal 104 (solo xinput1_4): su primer argumento es un bus (admite hasta 16), no un mando; se reenvía tal cual.
DWORD WINAPI SF_XInputGetBaseBusInformation(DWORD busIndex, void* info) {
    const auto fn = Original<GetBusInfoFn>(kGetBusInfo);
    return fn ? fn(busIndex, info) : kNotConnected;
}

// Ordinal 108 (solo xinput1_4): XInputGetCapabilities con identificadores del mando (lo usa SDL).
DWORD WINAPI SF_XInputGetCapabilitiesEx(DWORD reserved, DWORD userIndex, DWORD flags, void* capabilities) {
    const auto fn = Original<GetCapabilitiesExFn>(kGetCapabilitiesEx);
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(reserved, physical, flags, capabilities);
}

// Ordinal 109 (xinput1_4 de Windows 11 recientes, sin documentar). Supuesto a partir de su código: recibe
// (índice de mando < 4, puntero), como XInputGetState; solo se cambia el índice.
DWORD WINAPI SF_XInputOrdinal109(DWORD userIndex, void* data) {
    const auto fn = Original<IndexedFn>(kOrdinal109);
    DWORD physical = 0;
    if (!fn || !Map(userIndex, &physical))
        return kNotConnected;
    return fn(physical, data);
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

namespace {

// --- Intercepción de XInput dentro del proceso ---
//
// Un módulo del juego puede llegar a la XInput real sin pasar por la copia del filtro. En 7DTD, InControlNative
// hace LoadLibrary("XInput1_4.dll") cuando la xinput1_4.dll de System32 ya está en el proceso (cargada como
// DLL real de la copia xinput1_3 que usa Unity): por nombre, Windows devuelve la que ya está en memoria, y el
// juego veía los dos mandos. Para que cualquier vía pase por el filtro, al cargar la DLL real se redirigen sus
// exportaciones: cada entrada de su tabla apunta a un salto hacia la función equivalente del filtro.
// Solo cambia datos de este proceso (la tabla de exportación, con escrituras atómicas de 32 bits): no se
// parchea código de Windows ni se toca otro proceso.
// Límite: quien resolviera las funciones ANTES de que el filtro cargara la DLL real conserva las direcciones
// originales; el estado lo avisa con «previa=si».

constexpr char kStubMagic[16] = {'S', 'p', 'l', 'i', 't', 'F', 'o', 'r', 'g', 'e', 'X', 'I', 'n', 'p', 't', '1'};
constexpr size_t kStubSize = 16; // jmp [rip+0] (6 bytes) + dirección (8) + relleno (2)
constexpr size_t kStubPageSize = 4096;

struct Route {
    RouteId id;
    const char* name; // Nombre u ordinal (MAKEINTRESOURCEA) en la DLL real.
    void* wrapper;    // Función del filtro que la sustituye; nullptr: no lleva índice de mando, no se intercepta.
};

// Las lecturas que entran por la DLL real se cuentan aparte: prueban que esa vía pasa por el filtro.
DWORD WINAPI Hooked_XInputGetState(DWORD userIndex, XInputState* state) {
    g_hookedReads.fetch_add(1, std::memory_order_relaxed);
    return SF_XInputGetState(userIndex, state);
}

DWORD WINAPI Hooked_XInputGetStateEx(DWORD userIndex, XInputState* state) {
    g_hookedReads.fetch_add(1, std::memory_order_relaxed);
    return SF_XInputGetStateEx(userIndex, state);
}

// ¿La dirección es un salto instalado por otra copia del filtro (su página empieza con la firma)?
bool IsFilterStub(const void* address) {
    MEMORY_BASIC_INFORMATION mbi{};
    if (!address || VirtualQuery(address, &mbi, sizeof(mbi)) == 0)
        return false;
    if (mbi.State != MEM_COMMIT || mbi.Type != MEM_PRIVATE || mbi.BaseAddress != mbi.AllocationBase
        || mbi.Protect != PAGE_EXECUTE_READ || mbi.RegionSize < kStubPageSize)
        return false;
    return std::memcmp(mbi.AllocationBase, kStubMagic, sizeof(kStubMagic)) == 0;
}

#if defined(_M_X64) || defined(__x86_64__)

// Reserva una página por encima de la DLL real y a menos de 2 GB de su base: la tabla de exportación guarda
// desplazamientos de 32 bits respecto a la base del módulo.
BYTE* AllocateStubPage(const BYTE* base, DWORD imageSize) {
    SYSTEM_INFO info{};
    GetSystemInfo(&info);
    const ULONG_PTR step = info.dwAllocationGranularity;
    const ULONG_PTR limit = reinterpret_cast<ULONG_PTR>(base) + 0x7FF00000;
    ULONG_PTR address = reinterpret_cast<ULONG_PTR>(base) + imageSize;
    for (;;) {
        address = (address + step - 1) / step * step;
        MEMORY_BASIC_INFORMATION mbi{};
        if (address >= limit || VirtualQuery(reinterpret_cast<void*>(address), &mbi, sizeof(mbi)) == 0)
            return nullptr;
        const ULONG_PTR end = reinterpret_cast<ULONG_PTR>(mbi.BaseAddress) + mbi.RegionSize;
        if (mbi.State != MEM_FREE || end - address < kStubPageSize) {
            address = end;
            continue;
        }
        void* page = VirtualAlloc(reinterpret_cast<void*>(address), kStubPageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (page)
            return static_cast<BYTE*>(page);
        address += step; // Otro hilo ocupó el hueco: se prueba el siguiente.
    }
}

// Posición de una exportación (por nombre u ordinal) en la tabla de funciones, o -1.
int ExportIndex(const BYTE* base, const IMAGE_EXPORT_DIRECTORY* exports, const char* name) {
    if (IS_INTRESOURCE(name)) {
        const DWORD ordinal = LOWORD(reinterpret_cast<ULONG_PTR>(name));
        return ordinal >= exports->Base && ordinal - exports->Base < exports->NumberOfFunctions
                   ? static_cast<int>(ordinal - exports->Base) : -1;
    }
    const auto* names = reinterpret_cast<const DWORD*>(base + exports->AddressOfNames);
    const auto* ordinals = reinterpret_cast<const WORD*>(base + exports->AddressOfNameOrdinals);
    for (DWORD i = 0; i < exports->NumberOfNames; ++i) {
        if (std::strcmp(reinterpret_cast<const char*>(base + names[i]), name) == 0)
            return ordinals[i];
    }
    return -1;
}

// Apunta las exportaciones de la DLL real a las funciones del filtro. Primero se escriben todos los saltos
// y se deja su página como solo ejecución; después se cambia cada entrada de la tabla.
bool InterceptExports(HMODULE real, const Route* routes, size_t count) {
    auto* base = reinterpret_cast<BYTE*>(real);
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE)
        return false;
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
    const auto& dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (nt->Signature != IMAGE_NT_SIGNATURE || !dir.VirtualAddress)
        return false;
    const auto* exports = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(base + dir.VirtualAddress);
    auto* functions = reinterpret_cast<DWORD*>(base + exports->AddressOfFunctions);
    BYTE* page = AllocateStubPage(base, nt->OptionalHeader.SizeOfImage);
    if (!page)
        return false;
    std::memcpy(page, kStubMagic, sizeof(kStubMagic));

    struct Patch {
        DWORD* entry;
        DWORD rva;
    } patches[kRouteCount] = {};
    size_t pending = 0;
    BYTE* stub = page + sizeof(kStubMagic);
    for (size_t i = 0; i < count && pending < kRouteCount; ++i) {
        const Route& route = routes[i];
        if (!route.wrapper || !g_original[route.id])
            continue;
        const int index = ExportIndex(base, exports, route.name);
        // Solo si la tabla apunta a lo mismo que resolvió GetProcAddress (ni reenvíos ni tablas ya alteradas).
        if (index < 0 || base + functions[index] != g_original[route.id])
            continue;
        const BYTE jump[] = {0xFF, 0x25, 0, 0, 0, 0}; // jmp qword ptr [rip+0]
        std::memcpy(stub, jump, sizeof(jump));
        std::memcpy(stub + sizeof(jump), &route.wrapper, sizeof(route.wrapper));
        stub[14] = stub[15] = 0xCC;
        patches[pending++] = {&functions[index], static_cast<DWORD>(stub - base)};
        stub += kStubSize;
    }
    DWORD old = 0;
    if (pending == 0 || !VirtualProtect(page, kStubPageSize, PAGE_EXECUTE_READ, &old)) {
        VirtualFree(page, 0, MEM_RELEASE);
        return false;
    }
    FlushInstructionCache(GetCurrentProcess(), page, kStubPageSize);
    size_t done = 0;
    for (size_t i = 0; i < pending; ++i) {
        if (!VirtualProtect(patches[i].entry, sizeof(DWORD), PAGE_READWRITE, &old))
            continue;
        InterlockedExchange(reinterpret_cast<volatile LONG*>(patches[i].entry), static_cast<LONG>(patches[i].rva));
        VirtualProtect(patches[i].entry, sizeof(DWORD), old, &old);
        ++done;
    }
    return done == pending;
}

#else

bool InterceptExports(HMODULE, const Route*, size_t) { return false; } // Los saltos son de x64.

#endif

void LoadBackend() {
    const Route routes[] = {
        {kGetState, "XInputGetState", reinterpret_cast<void*>(&Hooked_XInputGetState)},
        {kGetStateEx, MAKEINTRESOURCEA(100), reinterpret_cast<void*>(&Hooked_XInputGetStateEx)},
        {kSetState, "XInputSetState", reinterpret_cast<void*>(&SF_XInputSetState)},
        {kGetCapabilities, "XInputGetCapabilities", reinterpret_cast<void*>(&SF_XInputGetCapabilities)},
        {kEnable, "XInputEnable", nullptr},
        {kGetDSoundGuids, "XInputGetDSoundAudioDeviceGuids", reinterpret_cast<void*>(&SF_XInputGetDSoundAudioDeviceGuids)},
        {kGetBattery, "XInputGetBatteryInformation", reinterpret_cast<void*>(&SF_XInputGetBatteryInformation)},
        {kGetKeystroke, "XInputGetKeystroke", reinterpret_cast<void*>(&SF_XInputGetKeystroke)},
        {kGetAudioIds, "XInputGetAudioDeviceIds", reinterpret_cast<void*>(&SF_XInputGetAudioDeviceIds)},
        {kWaitGuide, MAKEINTRESOURCEA(101), reinterpret_cast<void*>(&SF_XInputWaitForGuideButton)},
        {kCancelGuide, MAKEINTRESOURCEA(102), reinterpret_cast<void*>(&SF_XInputCancelGuideButtonWait)},
        {kPowerOff, MAKEINTRESOURCEA(103), reinterpret_cast<void*>(&SF_XInputPowerOffController)},
        {kGetBusInfo, MAKEINTRESOURCEA(104), nullptr},
        {kGetCapabilitiesEx, MAKEINTRESOURCEA(108), reinterpret_cast<void*>(&SF_XInputGetCapabilitiesEx)},
        {kOrdinal109, MAKEINTRESOURCEA(109), reinterpret_cast<void*>(&SF_XInputOrdinal109)},
    };
    wchar_t system[MAX_PATH] = {};
    const UINT length = GetSystemDirectoryW(system, MAX_PATH);
    if (length == 0 || length >= MAX_PATH - 20)
        return;
    bool preloaded = false;
    HMODULE real = LoadFromSystem(system, L"xinput1_4.dll", &preloaded);
    if (!real) // Windows 7: solo hay xinput1_3.dll (redistribuible de DirectX) o xinput9_1_0.dll.
        real = LoadFromSystem(system, g_realName, &preloaded);
    if (!real)
        return;
    bool delegated = false;
    for (const Route& route : routes) {
        void* address = reinterpret_cast<void*>(GetProcAddress(real, route.name));
        g_original[route.id] = address;
        delegated = delegated || (route.wrapper && IsFilterStub(address));
    }
    g_real.store(real, std::memory_order_release);
    if (delegated) { // Otra copia del filtro ya intercepta esta DLL: esta solo reenvía.
        g_interception.store(kDelegated, std::memory_order_relaxed);
        return;
    }
    g_realPreloaded.store(preloaded, std::memory_order_relaxed);
    // Los saltos apuntan a esta copia: no debe descargarse mientras viva el proceso.
    HMODULE pinned = nullptr;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                           reinterpret_cast<LPCWSTR>(g_self), &pinned)
        && InterceptExports(real, routes, sizeof(routes) / sizeof(routes[0])))
        g_interception.store(kIntercepted, std::memory_order_relaxed);
}

} // namespace
