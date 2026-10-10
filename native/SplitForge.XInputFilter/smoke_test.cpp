// Prueba de humo del filtro XInput (se ejecuta en la CI de Windows, sin mandos conectados).
// Carga el filtro con nombre xinput1_4.dll (y, si se pasa, también como xinput1_3.dll, igual que 7DTD,
// que carga ambas), le asigna la ranura 2 a este proceso y comprueba:
//  - que encuentra y carga la DLL real de System32 (nunca otra copia del filtro),
//  - que solo existe el índice 0 para el juego (los demás: no conectado sin consultar la real),
//  - que cada copia escribe su propio estado-PID-<dll>.txt con la ranura y las lecturas,
//  - que el estado se actualiza cuando SplitForge cambia la asignación con el juego abierto.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdio>
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

int CheckCopy(const wchar_t* path, const wchar_t* dll, const std::wstring& folder, GetStateFn* loaded) {
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
        || !std::strstr(text.c_str(), expectedDll) || !std::strstr(text.c_str(), "lecturas="))
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

    char assignment[64] = {};
    std::snprintf(assignment, sizeof(assignment), "12345 1\n%lu 2\n", GetCurrentProcessId());
    if (!WriteAssignment(folder, assignment))
        return Fail("no se pudo escribir la asignación");

    GetStateFn getState = nullptr;
    if (const int r = CheckCopy(argv[1], L"xinput1_4", folder, &getState))
        return r;
    if (argc >= 3) {
        // Segunda copia con otro nombre en el mismo proceso: su DLL real debe ser la de System32,
        // no la copia xinput1_4.dll ya cargada (eso aplicaría la asignación dos veces).
        GetStateFn second = nullptr;
        if (const int r = CheckCopy(argv[2], L"xinput1_3", folder, &second))
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
    std::printf("OK: filtro XInput cargado, ranura asignada, índices 1-3 ocultos, estado por DLL actualizado en vivo.\n");
    return 0;
}
