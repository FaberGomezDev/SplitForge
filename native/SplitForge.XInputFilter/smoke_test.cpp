// Prueba de humo del filtro XInput (se ejecuta en la CI de Windows, sin mandos conectados).
// Carga el filtro con nombre xinput1_4.dll, le asigna la ranura 2 a este proceso y comprueba:
//  - que encuentra y carga la DLL real de System32,
//  - que solo existe el índice 0 para el juego (los demás: no conectado sin consultar la real),
//  - que escribe su archivo de estado con la ranura leída.

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

} // namespace

int wmain(int argc, wchar_t** argv) {
    if (argc < 2)
        return Fail("uso: smoke_test <ruta del filtro renombrado como xinput1_4.dll>");
    const std::wstring folder = Folder();
    CreateDirectoryW((folder.substr(0, folder.rfind(L'\\'))).c_str(), nullptr);
    CreateDirectoryW(folder.c_str(), nullptr);

    const DWORD pid = GetCurrentProcessId();
    FILE* f = nullptr;
    if (_wfopen_s(&f, (folder + L"\\asignacion.txt").c_str(), L"w") != 0 || !f)
        return Fail("no se pudo escribir la asignación");
    std::fprintf(f, "12345 1\n%lu 2\n", pid);
    std::fclose(f);

    HMODULE filter = LoadLibraryW(argv[1]);
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

    wchar_t status[64] = {};
    swprintf_s(status, L"\\estado-%lu.txt", pid);
    if (_wfopen_s(&f, (folder + status).c_str(), L"r") != 0 || !f)
        return Fail("el filtro no escribió su estado");
    char text[256] = {};
    const size_t n = std::fread(text, 1, sizeof(text) - 1, f);
    std::fclose(f);
    text[n] = 0;
    std::printf("%s", text);
    if (!std::strstr(text, "ranura=2") || !std::strstr(text, "real=cargada") || !std::strstr(text, "dll=xinput1_4.dll"))
        return Fail("estado incorrecto");
    std::printf("OK: filtro XInput cargado, ranura 2 asignada, índices 1-3 ocultos.\n");
    return 0;
}
