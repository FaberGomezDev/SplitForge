// Prueba de humo del filtro XInput (se ejecuta en la CI de Windows, sin mandos conectados).
// Carga el filtro con nombre xinput1_4.dll (y, si se pasa, también como xinput1_3.dll, igual que 7DTD,
// que carga ambas), le asigna la ranura 2 a este proceso y comprueba:
//  - que encuentra y carga la DLL real de System32 (nunca otra copia del filtro),
//  - que solo existe el índice 0 para el juego (los demás: no conectado sin consultar la real),
//  - que cada copia escribe su propio estado-PID-<dll>.txt con la ranura y las lecturas,
//  - que quien pide la XInput real de System32 sin pasar por la copia del filtro (como InControlNative en
//    7DTD) también queda filtrado: sus exportaciones llevan al filtro y esas lecturas se cuentan aparte,
//  - que una segunda copia del filtro delega en la primera (el índice no se cambia dos veces),
//  - que, con una ventana como la de Unity que no es la activa, el proceso cree tener el foco y usa un
//    puntero propio: mover o encerrar «su» puntero no toca el de Windows,
//  - que una tecla que el juego vio pulsar se le suelta al perder el foco de verdad (Alt tras Alt+Tab) y que
//    no ve la desactivación,
//  - que el mando de prueba (prueba.txt) pulsa y suelta botones aunque no haya mando en la ranura,
//  - que el estado se actualiza cuando SplitForge cambia la asignación con el juego abierto.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <string>

namespace {

constexpr DWORD kNotConnected = 1167;

struct State {
    DWORD packet;
    WORD buttons;
    BYTE lt, rt;
    SHORT lx, ly, rx, ry;
};

using GetStateFn = DWORD(WINAPI*)(DWORD, State*);

int Fail(const char* what) {
    std::printf("FALLO: %s\n", what);
    return 1;
}

std::wstring Folder() {
    wchar_t local[MAX_PATH] = {};
    GetEnvironmentVariableW(L"LOCALAPPDATA", local, MAX_PATH);
    return std::wstring(local) + L"\\SplitForge\\xinput";
}

bool WriteAssignment(const std::wstring& folder, const char* text) {
    // Igual que SplitForge: temporal + reemplazo atómico.
    const std::wstring path = folder + L"\\asignacion.txt";
    const std::wstring temp = path + L".tmp";
    FILE* f = nullptr;
    if (_wfopen_s(&f, temp.c_str(), L"w") != 0 || !f)
        return false;
    std::fputs(text, f);
    std::fclose(f);
    return MoveFileExW(temp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING) != 0;
}

std::string ReadStatus(const std::wstring& folder, const wchar_t* dll) {
    wchar_t name[96] = {};
    swprintf_s(name, L"\\estado-%lu-%ls.txt", GetCurrentProcessId(), dll);
    FILE* f = nullptr;
    if (_wfopen_s(&f, (folder + name).c_str(), L"r") != 0 || !f)
        return {};
    char text[512] = {};
    const size_t n = std::fread(text, 1, sizeof(text) - 1, f);
    std::fclose(f);
    text[n] = 0;
    return text;
}

// Llama a XInputGetState durante ~ms (como un juego en cada fotograma) hasta que el estado contenga 'expected'.
bool PollUntil(GetStateFn getState, const std::wstring& folder, const wchar_t* dll, const char* expected, DWORD ms) {
    const ULONGLONG until = GetTickCount64() + ms;
    State state{};
    while (GetTickCount64() < until) {
        getState(0, &state);
        if (std::strstr(ReadStatus(folder, dll).c_str(), expected))
            return true;
        Sleep(16);
    }
    return false;
}

// Valor numérico de una línea "clave=número" del estado (-1 si no está).
long long StatusNumber(const std::string& text, const char* key) {
    const char* found = std::strstr(text.c_str(), key);
    return found ? std::strtoll(found + std::strlen(key), nullptr, 10) : -1;
}

// Hace lo que un módulo del juego que no pasa por la copia del filtro: pide la XInput real de System32 y
// resuelve sus funciones. Con el filtro ya cargado, deben llevar al filtro (índices 1–3 ocultos, lecturas
// contadas como interceptadas).
int CheckRealIsIntercepted(const std::wstring& folder, const wchar_t* dll) {
    // Por ruta completa: por nombre, Windows devolvería la copia del filtro ya cargada como xinput1_4.dll.
    wchar_t system[MAX_PATH] = {};
    GetSystemDirectoryW(system, MAX_PATH);
    HMODULE real = LoadLibraryW((std::wstring(system) + L"\\xinput1_4.dll").c_str());
    if (!real)
        return Fail("no se pudo cargar la xinput1_4.dll de System32");
    auto getState = reinterpret_cast<GetStateFn>(reinterpret_cast<void*>(GetProcAddress(real, "XInputGetState")));
    auto getStateEx = reinterpret_cast<GetStateFn>(reinterpret_cast<void*>(GetProcAddress(real, MAKEINTRESOURCEA(100))));
    if (!getState || !getStateEx)
        return Fail("la DLL real no exporta XInputGetState / ordinal 100");
    HMODULE owner = nullptr;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           reinterpret_cast<LPCWSTR>(reinterpret_cast<void*>(getState)), &owner) && owner == real)
        return Fail("XInputGetState de la DLL real no está interceptada");
    State state{};
    for (DWORD i = 1; i < 4; i++) {
        if (getState(i, &state) != kNotConnected || getStateEx(i, &state) != kNotConnected)
            return Fail("por la DLL real, los índices 1–3 deben estar desconectados");
    }
    const DWORD first = getStateEx(0, &state);
    if (first != ERROR_SUCCESS && first != kNotConnected)
        return Fail("respuesta inesperada por la DLL real para el índice 0");
    const ULONGLONG until = GetTickCount64() + 6000; // El estado se reescribe como mucho cada 2 s.
    long long intercepted = 0;
    while (intercepted <= 0 && GetTickCount64() < until) {
        getState(0, &state);
        intercepted = StatusNumber(ReadStatus(folder, dll), "interceptadas=");
        Sleep(16);
    }
    if (intercepted <= 0)
        return Fail("las lecturas por la DLL real no pasan por el filtro");
    std::printf("Por la DLL real de System32: %lld lecturas interceptadas.\n", intercepted);
    return 0;
}

// Lo que le llega al «juego» (la ventana de prueba) después de pasar por el filtro.
int g_altPresses = 0;   // WM_SYSKEYDOWN / WM_KEYDOWN de Alt
int g_altReleases = 0;  // WM_KEYUP de Alt
int g_inactiveSeen = 0; // WM_ACTIVATE con WA_INACTIVE (el filtro debe convertirlos en activaciones)

LRESULT CALLBACK GameProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if ((message == WM_SYSKEYDOWN || message == WM_KEYDOWN) && wParam == VK_MENU)
        ++g_altPresses;
    if (message == WM_KEYUP && wParam == VK_MENU && (lParam & 0x80000000))
        ++g_altReleases;
    if (message == WM_ACTIVATE && LOWORD(wParam) == WA_INACTIVE)
        ++g_inactiveSeen;
    if (message == WM_SYSKEYDOWN || message == WM_SYSKEYUP || message == WM_KEYDOWN || message == WM_KEYUP)
        return 0; // Como Unity: las teclas no llegan a DefWindowProc (evita el modo menú con Alt).
    return DefWindowProcW(hwnd, message, wParam, lParam);
}

// Ventana con la clase de Unity, visible pero fuera de la pantalla y sin tomar el foco: el filtro la adopta
// como «ventana del juego» de este proceso.
HWND CreateGameWindow() {
    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = GameProc;
    windowClass.hInstance = GetModuleHandleW(nullptr);
    windowClass.lpszClassName = L"UnityWndClass";
    if (!RegisterClassW(&windowClass))
        return nullptr;
    HWND hwnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, windowClass.lpszClassName, L"SplitForge humo",
                                WS_POPUP, -20000, -20000, 320, 200, nullptr, nullptr, windowClass.hInstance, nullptr);
    if (hwnd)
        ShowWindow(hwnd, SW_SHOWNOACTIVATE);
    return hwnd;
}

void PumpMessages() {
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        DispatchMessageW(&message);
}

// El «juego» de esta prueba no es la ventana activa (la activa es la consola o lo que tenga el usuario): debe
// creer que tiene el foco y, al mover o encerrar el puntero, no tocar el de Windows.
int CheckOwnCursor(HWND hwnd, GetStateFn getState, const std::wstring& folder, const wchar_t* dll) {
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    using GetPosFn = BOOL(WINAPI*)(POINT*);
    using GetClipFn = BOOL(WINAPI*)(RECT*);
    using GetWindowFn = HWND(WINAPI*)();
    // Por GetProcAddress: las funciones de verdad, sin pasar por la tabla de importaciones de este ejecutable.
    const auto realGetPos = reinterpret_cast<GetPosFn>(reinterpret_cast<void*>(GetProcAddress(user32, "GetCursorPos")));
    const auto realGetClip = reinterpret_cast<GetClipFn>(reinterpret_cast<void*>(GetProcAddress(user32, "GetClipCursor")));
    const auto realForeground = reinterpret_cast<GetWindowFn>(reinterpret_cast<void*>(GetProcAddress(user32, "GetForegroundWindow")));
    if (!realGetPos || !realGetClip || !realForeground)
        return Fail("user32 no exporta las funciones de puntero");

    // El filtro activa el foco desde su hilo con SendMessage: esta ventana tiene que atender sus mensajes.
    const ULONGLONG until = GetTickCount64() + 15000;
    State state{};
    std::string text;
    while (GetTickCount64() < until) {
        PumpMessages();
        getState(0, &state);
        text = ReadStatus(folder, dll);
        if (std::strstr(text.c_str(), "foco=forzado") && std::strstr(text.c_str(), "puntero=propio"))
            break;
        Sleep(16);
    }
    if (!std::strstr(text.c_str(), "foco=forzado") || !std::strstr(text.c_str(), "puntero=propio"))
        return Fail("el filtro no activó el foco simulado y el puntero propio");
    if (realForeground() == hwnd) {
        std::printf("AVISO: la ventana de prueba es la activa; no se comprueba el puntero propio.\n");
        return 0;
    }
    if (GetForegroundWindow() != hwnd || GetActiveWindow() != hwnd || GetFocus() != hwnd)
        return Fail("el proceso no cree tener el foco");

    POINT before{};
    RECT clipBefore{};
    if (!realGetPos(&before) || !realGetClip(&clipBefore)) {
        std::printf("AVISO: sin puntero en esta sesión; no se comprueba el puntero propio.\n");
        return 0;
    }
    const POINT target{before.x + 137, before.y + 91};
    POINT seen{};
    if (!SetCursorPos(target.x, target.y) || !GetCursorPos(&seen))
        return Fail("las funciones de puntero del filtro fallaron");
    if (seen.x != target.x || seen.y != target.y)
        return Fail("el proceso no ve su propio puntero donde lo dejó");
    POINT real{};
    realGetPos(&real);
    if (real.x == target.x && real.y == target.y)
        return Fail("mover el puntero propio movió el puntero de Windows");
    const RECT cage{target.x, target.y, target.x + 1, target.y + 1};
    if (!ClipCursor(&cage))
        return Fail("ClipCursor del filtro falló");
    RECT clipAfter{};
    realGetClip(&clipAfter);
    if (!EqualRect(&clipBefore, &clipAfter))
        return Fail("encerrar el puntero propio encerró el puntero de Windows");
    ClipCursor(nullptr);

    long long served = 0;
    const ULONGLONG wait = GetTickCount64() + 6000; // El estado se reescribe como mucho cada 2 s.
    while (served <= 0 && GetTickCount64() < wait) {
        PumpMessages();
        getState(0, &state);
        served = StatusNumber(ReadStatus(folder, dll), "puntero_propio=");
        Sleep(16);
    }
    if (served <= 0)
        return Fail("el estado no cuenta las llamadas atendidas con el puntero propio");
    std::printf("Puntero propio: %lld llamadas sin tocar el de Windows (real en %ld,%ld; propio en %ld,%ld).\n",
                served, real.x, real.y, seen.x, seen.y);
    return 0;
}

bool WriteTestPad(const std::wstring& folder, const char* text) {
    const std::wstring path = folder + L"\\prueba.txt";
    if (!text)
        return DeleteFileW(path.c_str()) != 0 || GetLastError() == ERROR_FILE_NOT_FOUND;
    const std::wstring temp = path + L".tmp";
    FILE* f = nullptr;
    if (_wfopen_s(&f, temp.c_str(), L"w") != 0 || !f)
        return false;
    std::fputs(text, f);
    std::fclose(f);
    return MoveFileExW(temp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING) != 0;
}

// Escribe la línea del mando de prueba de este proceso y espera a que el filtro la aplique: LB (0x0100)
// sirve de acuse (pulsado o no, según `lb`). Las opciones van en la misma línea, así que llegan a la vez.
bool SetTestPad(const std::wstring& folder, GetStateFn getState, bool lb, const char* options) {
    char line[96] = {};
    std::snprintf(line, sizeof(line), "%lu %s %s\n", GetCurrentProcessId(), lb ? "0100" : "0000", options);
    if (!WriteTestPad(folder, line))
        return false;
    const ULONGLONG until = GetTickCount64() + 5000;
    State state{};
    static DWORD lastPacket = 0xFFFFFFFF;
    while (GetTickCount64() < until) {
        PumpMessages();
        const DWORD result = getState(0, &state);
        // El mando de prueba existe aunque la ranura no tenga mando; el paquete cambia con cada cambio.
        if (result == ERROR_SUCCESS && ((state.buttons & 0x0100) != 0) == lb && state.packet != lastPacket) {
            lastPacket = state.packet;
            return true;
        }
        Sleep(16);
    }
    return false;
}

// La ventana deja de ser la activa: Windows le avisa con estos dos mensajes.
void Deactivate(HWND hwnd) {
    SendMessageW(hwnd, WM_ACTIVATE, WA_INACTIVE, 0);
    SendMessageW(hwnd, WM_KILLFOCUS, 0, 0);
}

void PressAlt(HWND hwnd) { SendMessageW(hwnd, WM_SYSKEYDOWN, VK_MENU, 0x20380001); }

// Teclas pegadas (el fallo de LB/RB en 7DTD tras Alt+Tab) y mando de prueba. La ventana de prueba nunca es la
// activa de verdad: la opción «activa» del mando de prueba hace que el filtro la trate como tal.
int CheckStuckKeysAndTestPad(HWND hwnd, GetStateFn getState, const std::wstring& folder, const wchar_t* dll) {
    // 1) Alt+Tab: el juego ve pulsar Alt siendo la ventana activa y pierde el foco antes de verla soltar.
    if (!SetTestPad(folder, getState, true, "activa"))
        return Fail("el mando de prueba no pulsó LB");
    PressAlt(hwnd);
    if (g_altPresses != 1)
        return Fail("con la ventana activa, el juego debe ver la pulsación de Alt");
    if (!SetTestPad(folder, getState, false, ""))
        return Fail("el mando de prueba no soltó LB (o el paquete no cambió)");
    Deactivate(hwnd);
    if (g_altReleases != 1)
        return Fail("al perder el foco con Alt pulsada, el juego debe recibir que Alt se soltó (una vez)");
    if (g_inactiveSeen != 0)
        return Fail("el juego no debe ver la desactivación de su ventana");
    Deactivate(hwnd);
    if (g_altReleases != 1)
        return Fail("una tecla ya soltada no se suelta dos veces");

    // 2) Una pulsación atrasada (repetición de Alt en cola) llega cuando la ventana ya no es la activa.
    PressAlt(hwnd);
    if (g_altPresses != 1)
        return Fail("una pulsación que llega sin ser la ventana activa no debe entregarse al juego");

    // 3) «sin_soltar» desactiva la corrección: Alt se queda pegada (así se reproduce el fallo en un ensayo).
    if (!SetTestPad(folder, getState, true, "activa sin_soltar"))
        return Fail("el mando de prueba no aplicó sin_soltar");
    PressAlt(hwnd);
    if (!SetTestPad(folder, getState, false, "sin_soltar"))
        return Fail("el mando de prueba no mantuvo sin_soltar");
    Deactivate(hwnd);
    if (g_altPresses != 2 || g_altReleases != 1)
        return Fail("con sin_soltar no se debe soltar ninguna tecla");

    // 4) Al volver la corrección, la tecla que quedó pegada se suelta en la siguiente pérdida de foco.
    if (!SetTestPad(folder, getState, true, ""))
        return Fail("el mando de prueba no volvió al modo normal");
    Deactivate(hwnd);
    if (g_altReleases != 2)
        return Fail("la tecla pegada no se soltó al volver la corrección");

    if (!WriteTestPad(folder, nullptr))
        return Fail("no se pudo borrar prueba.txt");
    long long released = 0;
    State state{};
    const ULONGLONG until = GetTickCount64() + 6000;
    while (released < 2 && GetTickCount64() < until) {
        PumpMessages();
        getState(0, &state);
        released = StatusNumber(ReadStatus(folder, dll), "teclas_soltadas=");
        Sleep(16);
    }
    if (released != 2)
        return Fail("el estado no cuenta las teclas soltadas");
    std::printf("Teclas pegadas: Alt se suelta al perder el foco (%lld) y una pulsación atrasada no llega al juego; "
                "mando de prueba: LB pulsado y soltado sin mando en la ranura.\n", released);
    return 0;
}

int CheckCopy(const wchar_t* path, const wchar_t* dll, const char* interception, const std::wstring& folder, GetStateFn* loaded) {
    HMODULE filter = LoadLibraryW(path);
    if (!filter)
        return Fail("no se pudo cargar el filtro");
    auto getState = reinterpret_cast<GetStateFn>(reinterpret_cast<void*>(GetProcAddress(filter, "XInputGetState")));
    auto getStateEx = reinterpret_cast<GetStateFn>(reinterpret_cast<void*>(GetProcAddress(filter, MAKEINTRESOURCEA(100))));
    if (!getState || !getStateEx)
        return Fail("faltan exportaciones (XInputGetState / ordinal 100)");

    State state{};
    const DWORD first = getState(0, &state); // Ranura física 2: sin mandos en la CI → no conectado o éxito.
    if (first != ERROR_SUCCESS && first != kNotConnected)
        return Fail("respuesta inesperada para el índice 0");
    for (DWORD i = 1; i < 4; i++) {
        if (getState(i, &state) != kNotConnected || getStateEx(i, &state) != kNotConnected)
            return Fail("con asignación, los índices 1–3 deben estar desconectados");
    }
    if (getState(7, &state) != kNotConnected)
        return Fail("índice fuera de rango aceptado");

    const std::string text = ReadStatus(folder, dll);
    std::printf("%ls:\n%s", dll, text.c_str());
    char expectedDll[64] = {};
    std::snprintf(expectedDll, sizeof(expectedDll), "dll=%ls.dll", dll);
    if (!std::strstr(text.c_str(), "ranura=2") || !std::strstr(text.c_str(), "real=cargada")
        || !std::strstr(text.c_str(), expectedDll) || !std::strstr(text.c_str(), "lecturas=")
        || !std::strstr(text.c_str(), interception))
        return Fail("estado incorrecto");
    *loaded = getState;
    return 0;
}

} // namespace

int wmain(int argc, wchar_t** argv) {
    if (argc < 2)
        return Fail("uso: smoke_test <filtro como xinput1_4.dll> [filtro como xinput1_3.dll]");
    const std::wstring folder = Folder();
    CreateDirectoryW((folder.substr(0, folder.rfind(L'\\'))).c_str(), nullptr);
    CreateDirectoryW(folder.c_str(), nullptr);
    WriteTestPad(folder, nullptr); // Por si quedó de una ejecución anterior interrumpida.

    char assignment[64] = {};
    std::snprintf(assignment, sizeof(assignment), "12345 1\n%lu 2\n", GetCurrentProcessId());
    if (!WriteAssignment(folder, assignment))
        return Fail("no se pudo escribir la asignación");

    const HWND gameWindow = CreateGameWindow(); // Antes de cargar el filtro: es la ventana que adoptará.
    GetStateFn getState = nullptr;
    if (const int r = CheckCopy(argv[1], L"xinput1_4", "intercepcion=activa\n", folder, &getState))
        return r;
    if (!gameWindow)
        std::printf("AVISO: no se pudo crear una ventana (¿sesión sin escritorio?); no se comprueba el puntero propio.\n");
    else if (const int r = CheckOwnCursor(gameWindow, getState, folder, L"xinput1_4"))
        return r;
    else if (const int r2 = CheckStuckKeysAndTestPad(gameWindow, getState, folder, L"xinput1_4"))
        return r2;
    if (const int r = CheckRealIsIntercepted(folder, L"xinput1_4"))
        return r;
    if (argc >= 3) {
        // Segunda copia con otro nombre en el mismo proceso: su DLL real es la de System32, que la primera
        // ya intercepta; debe delegar en ella (cambiar el índice dos veces ocultaría el mando).
        GetStateFn second = nullptr;
        if (const int r = CheckCopy(argv[2], L"xinput1_3", "intercepcion=delegada\n", folder, &second))
            return r;
    }

    // Cambio de asignación con el "juego" abierto: el estado debe reflejar la ranura nueva.
    std::snprintf(assignment, sizeof(assignment), "%lu 3\n", GetCurrentProcessId());
    if (!WriteAssignment(folder, assignment))
        return Fail("no se pudo cambiar la asignación");
    if (!PollUntil(getState, folder, L"xinput1_4", "ranura=3", 5000))
        return Fail("el estado no se actualizó tras cambiar la asignación");
    const std::string updated = ReadStatus(folder, L"xinput1_4");
    if (std::strstr(updated.c_str(), "lecturas=0\n"))
        return Fail("el estado no cuenta las lecturas");
    std::printf("Tras reasignar:\n%s", updated.c_str());
    std::printf("OK: filtro XInput cargado, ranura asignada, índices 1-3 ocultos, DLL real interceptada, "
                "estado por DLL actualizado en vivo.\n");
    return 0;
}
