// Prueba de humo del emulador de Steam (se ejecuta en la CI de Windows, sin Steam instalado).
// Comprueba: identidad/config por JSON, la API plana tal como la usa Steamworks.NET (todas sus
// exportaciones, self como primer argumento y las interfaces que pide CSteamAPIContext.Init),
// SteamInternal_ContextInit con el contrato del SDK, stats con callback y persistencia,
// almacenamiento remoto con ida y vuelta, y un intercambio P2P entre dos procesos del mismo PC con
// identidades distintas (el caso de pantalla dividida: dos instancias que se descubren en local).

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <string>

#include "flat_required_exports.h"
#include "interfaces.h"
#include "state.h"

namespace {

constexpr uint64_t kSteamIdA = 76561198000000001ull;
constexpr uint64_t kSteamIdB = 76561198000000002ull;

int Fail(const char* what) {
    std::printf("FALLO: %s\n", what);
    return 1;
}

std::wstring TempDir() {
    wchar_t temp[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, temp);
    wchar_t out[MAX_PATH] = {};
    swprintf_s(out, L"%lsSplitForgeSteamEmuTest", temp);
    return out;
}

std::string WideToUtf8Local(const std::wstring& wide) {
    if (wide.empty())
        return {};
    const int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0)
        return {};
    std::string out(static_cast<size_t>(size) - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, out.data(), size, nullptr, nullptr);
    return out;
}

bool WriteJson(const std::wstring& path, const char* content) {
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;
    DWORD written = 0;
    const BOOL ok = WriteFile(file, content, static_cast<DWORD>(std::strlen(content)), &written,
                              nullptr);
    CloseHandle(file);
    return ok && written == std::strlen(content);
}

bool FileExists(const std::wstring& path) {
    return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

void SetConfigEnv(const std::wstring& path) {
    const std::string utf8 = WideToUtf8Local(path);
    _putenv_s("SPLITFORGE_STEAMEMU_CONFIG", utf8.c_str());
}

std::string ReadTextFile(const std::wstring& path) {
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return {};
    std::string out(4096, '\0');
    DWORD read = 0;
    ReadFile(file, out.data(), 4096, &read, nullptr);
    CloseHandle(file);
    out.resize(read);
    return out;
}

bool SpawnPeer(const std::wstring& exePath, const std::wstring& dir, const std::wstring& configPath,
               HANDLE* process) {
    wchar_t commandLine[MAX_PATH * 2] = {};
    swprintf_s(commandLine, L"\"%ls\" \"%ls\" --peer", exePath.c_str(), dir.c_str());
    SetConfigEnv(configPath);
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION info{};
    const BOOL created = CreateProcessW(exePath.c_str(), commandLine, nullptr, nullptr, FALSE, 0,
                                        nullptr, nullptr, &startup, &info);
    if (!created) {
        char text[128] = {};
        snprintf(text, sizeof(text), "no se pudo lanzar el proceso par (error %lu)",
                 static_cast<unsigned long>(GetLastError()));
        Fail(text);
        return false;
    }
    CloseHandle(info.hThread);
    *process = info.hProcess;
    return true;
}

bool WaitForFile(const std::wstring& path, unsigned int timeoutMs) {
    const ULONGLONG deadline = GetTickCount64() + timeoutMs;
    while (GetTickCount64() < deadline) {
        if (FileExists(path))
            return true;
        Sleep(100);
    }
    return false;
}

struct SteamEmuApi {
    HMODULE module = nullptr;
    bool (*InitSafe)() = nullptr;
    void (*Shutdown)() = nullptr;
    void* (*Client)() = nullptr;
    sfemu::ISteamUser* (*User)() = nullptr;
    sfemu::ISteamFriends* (*Friends)() = nullptr;
    sfemu::ISteamUtils* (*Utils)() = nullptr;
    sfemu::ISteamApps* (*Apps)() = nullptr;
    sfemu::ISteamUserStats* (*UserStats)() = nullptr;
    sfemu::ISteamRemoteStorage* (*RemoteStorage)() = nullptr;
    sfemu::ISteamNetworking* (*Networking)() = nullptr;
    bool (*ManualDispatchGetNext)(int, sfemu::CallbackMsg_t*) = nullptr;
    void (*ManualDispatchFree)(int) = nullptr;
    void* (*ContextInit)(void*) = nullptr;
    void* (*FindOrCreateUserInterface)(int, const char*) = nullptr;
    void* (*NetworkingSockets)() = nullptr;
    void* (*NetworkingUtils)() = nullptr;

    bool Load(const std::wstring& dir) {
        module = LoadLibraryW((dir + L"\\steam_api64.dll").c_str());
        if (!module) {
            char msg[128] = {};
            snprintf(msg, sizeof(msg), "no se pudo cargar steam_api64.dll (error %lu)",
                     static_cast<unsigned long>(GetLastError()));
            Fail(msg);
            return false;
        }
        InitSafe = Load<decltype(InitSafe)>("SteamAPI_InitSafe");
        Shutdown = Load<decltype(Shutdown)>("SteamAPI_Shutdown");
        Client = Load<decltype(Client)>("SteamClient");
        User = Load<decltype(User)>("SteamUser");
        Friends = Load<decltype(Friends)>("SteamFriends");
        Utils = Load<decltype(Utils)>("SteamUtils");
        Apps = Load<decltype(Apps)>("SteamApps");
        UserStats = Load<decltype(UserStats)>("SteamUserStats");
        RemoteStorage = Load<decltype(RemoteStorage)>("SteamRemoteStorage");
        Networking = Load<decltype(Networking)>("SteamNetworking");
        ManualDispatchGetNext = Load<decltype(ManualDispatchGetNext)>("SteamAPI_ManualDispatch_GetNextCallback");
        ManualDispatchFree = Load<decltype(ManualDispatchFree)>("SteamAPI_ManualDispatch_FreeLastCallback");
        ContextInit = Load<decltype(ContextInit)>("SteamInternal_ContextInit");
        FindOrCreateUserInterface = Load<decltype(FindOrCreateUserInterface)>("SteamInternal_FindOrCreateUserInterface");
        NetworkingSockets = Load<decltype(NetworkingSockets)>("SteamNetworkingSockets");
        NetworkingUtils = Load<decltype(NetworkingUtils)>("SteamNetworkingUtils");
        return InitSafe && Shutdown && User && Friends && Utils && Apps && UserStats &&
               RemoteStorage && Networking && ManualDispatchGetNext && ManualDispatchFree &&
               ContextInit && FindOrCreateUserInterface && NetworkingSockets && NetworkingUtils;
    }

    template <typename Fn>
    Fn Load(const char* name) {
        return reinterpret_cast<Fn>(reinterpret_cast<void*>(GetProcAddress(module, name)));
    }
};

template <typename Fn>
Fn Export(HMODULE module, const char* name) {
    return reinterpret_cast<Fn>(reinterpret_cast<void*>(GetProcAddress(module, name)));
}

// Accesor de un juego C++ (steam_api_internal.h): la función de inicio guarda la interfaz en el hueco.
void* (*g_findUserInterface)(int, const char*) = nullptr;
int g_accessorCalls = 0;
void InitUserAccessor(void* slot) {
    ++g_accessorCalls;
    *static_cast<void**>(slot) = g_findUserInterface(1, "SteamUser023");
}

// La API plana como la usa Steamworks.NET (7 Days to Die): self como primer argumento.
int CheckFlatApi(HMODULE module) {
    for (const char* name : sfemu_flat::kSteamworksNetExports) {
        if (!GetProcAddress(module, name)) {
            char text[192] = {};
            std::snprintf(text, sizeof(text), "falta la exportación %s", name);
            return Fail(text);
        }
    }
    void* client = Export<void* (*)(const char*)>(module, "SteamInternal_CreateInterface")("SteamClient020");
    if (!client)
        return Fail("SteamInternal_CreateInterface(SteamClient020) nulo");
    // Mismas interfaces y versiones que CSteamAPIContext.Init: si una devuelve nulo, el juego no arranca.
    struct Accessor {
        const char* entry;
        const char* version;
    };
    static constexpr Accessor accessors[] = {
        {"SteamAPI_ISteamClient_GetISteamUser", "SteamUser023"},
        {"SteamAPI_ISteamClient_GetISteamFriends", "SteamFriends017"},
        {"SteamAPI_ISteamClient_GetISteamMatchmaking", "SteamMatchMaking009"},
        {"SteamAPI_ISteamClient_GetISteamMatchmakingServers", "SteamMatchMakingServers002"},
        {"SteamAPI_ISteamClient_GetISteamUserStats", "STEAMUSERSTATS_INTERFACE_VERSION012"},
        {"SteamAPI_ISteamClient_GetISteamApps", "STEAMAPPS_INTERFACE_VERSION008"},
        {"SteamAPI_ISteamClient_GetISteamNetworking", "SteamNetworking006"},
        {"SteamAPI_ISteamClient_GetISteamRemoteStorage", "STEAMREMOTESTORAGE_INTERFACE_VERSION016"},
        {"SteamAPI_ISteamClient_GetISteamScreenshots", "STEAMSCREENSHOTS_INTERFACE_VERSION003"},
        {"SteamAPI_ISteamClient_GetISteamGameSearch", "SteamMatchGameSearch001"},
        {"SteamAPI_ISteamClient_GetISteamHTTP", "STEAMHTTP_INTERFACE_VERSION003"},
        {"SteamAPI_ISteamClient_GetISteamUGC", "STEAMUGC_INTERFACE_VERSION017"},
        {"SteamAPI_ISteamClient_GetISteamAppList", "STEAMAPPLIST_INTERFACE_VERSION001"},
        {"SteamAPI_ISteamClient_GetISteamMusic", "STEAMMUSIC_INTERFACE_VERSION001"},
        {"SteamAPI_ISteamClient_GetISteamMusicRemote", "STEAMMUSICREMOTE_INTERFACE_VERSION001"},
        {"SteamAPI_ISteamClient_GetISteamHTMLSurface", "STEAMHTMLSURFACE_INTERFACE_VERSION_005"},
        {"SteamAPI_ISteamClient_GetISteamInventory", "STEAMINVENTORY_INTERFACE_V003"},
        {"SteamAPI_ISteamClient_GetISteamVideo", "STEAMVIDEO_INTERFACE_V002"},
        {"SteamAPI_ISteamClient_GetISteamParentalSettings", "STEAMPARENTALSETTINGS_INTERFACE_VERSION001"},
        {"SteamAPI_ISteamClient_GetISteamInput", "SteamInput006"},
        {"SteamAPI_ISteamClient_GetISteamParties", "SteamParties002"},
        {"SteamAPI_ISteamClient_GetISteamRemotePlay", "STEAMREMOTEPLAY_INTERFACE_VERSION001"},
    };
    using GetInterface = void* (*)(void*, int, int, const char*);
    for (const auto& accessor : accessors) {
        if (!Export<GetInterface>(module, accessor.entry)(client, 1, 1, accessor.version)) {
            char text[192] = {};
            std::snprintf(text, sizeof(text), "%s(%s) devolvió nulo", accessor.entry, accessor.version);
            return Fail(text);
        }
    }
    void* utils = Export<void* (*)(void*, int, const char*)>(module, "SteamAPI_ISteamClient_GetISteamUtils")(
        client, 1, "SteamUtils010");
    if (!utils)
        return Fail("SteamAPI_ISteamClient_GetISteamUtils devolvió nulo");
    auto findUser = Export<void* (*)(int, const char*)>(module, "SteamInternal_FindOrCreateUserInterface");
    for (const char* version : {"SteamNetworkingUtils004", "SteamNetworkingSockets012", "SteamNetworkingMessages002"}) {
        if (!findUser(1, version))
            return Fail("SteamInternal_FindOrCreateUserInterface de Networking devolvió nulo");
    }

    void* user = Export<GetInterface>(module, "SteamAPI_ISteamClient_GetISteamUser")(client, 1, 1, "SteamUser023");
    void* friends =
        Export<GetInterface>(module, "SteamAPI_ISteamClient_GetISteamFriends")(client, 1, 1, "SteamFriends017");
    void* stats = Export<GetInterface>(module, "SteamAPI_ISteamClient_GetISteamUserStats")(
        client, 1, 1, "STEAMUSERSTATS_INTERFACE_VERSION012");
    void* input = Export<GetInterface>(module, "SteamAPI_ISteamClient_GetISteamInput")(client, 1, 1, "SteamInput006");
    if (Export<uint64_t (*)(void*)>(module, "SteamAPI_ISteamUser_GetSteamID")(user) != kSteamIdA)
        return Fail("API plana: SteamID distinto al configurado");
    const char* persona = Export<const char* (*)(void*)>(module, "SteamAPI_ISteamFriends_GetPersonaName")(friends);
    if (!persona || std::strcmp(persona, "SplitForgeTestA") != 0)
        return Fail("API plana: nombre de persona incorrecto");
    if (Export<uint32_t (*)(void*)>(module, "SteamAPI_ISteamUtils_GetAppID")(utils) != 251570)
        return Fail("API plana: AppID incorrecto");

    // Sobrecargas con el sufijo del SDK (GetStatInt32 / GetStatFloat).
    auto setInt = Export<bool (*)(void*, const char*, int32_t)>(module, "SteamAPI_ISteamUserStats_SetStatInt32");
    auto getInt = Export<bool (*)(void*, const char*, int32_t*)>(module, "SteamAPI_ISteamUserStats_GetStatInt32");
    auto setFloat = Export<bool (*)(void*, const char*, float)>(module, "SteamAPI_ISteamUserStats_SetStatFloat");
    auto getFloat = Export<bool (*)(void*, const char*, float*)>(module, "SteamAPI_ISteamUserStats_GetStatFloat");
    int32_t flatInt = 0;
    float flatFloat = 0.0f;
    if (!setInt(stats, "FlatInt", 7) || !getInt(stats, "FlatInt", &flatInt) || flatInt != 7)
        return Fail("API plana: stat entero de ida y vuelta incorrecto");
    if (!setFloat(stats, "FlatFloat", 2.5f) || !getFloat(stats, "FlatFloat", &flatFloat) || flatFloat != 2.5f)
        return Fail("API plana: stat flotante de ida y vuelta incorrecto");

    // Estructura devuelta por valor (13 bytes, puntero oculto) y entradas neutras de lo no implementado.
    const auto analog = Export<sfemu::InputAnalogActionData_t (*)(void*, uint64_t, uint64_t)>(
        module, "SteamAPI_ISteamInput_GetAnalogActionData")(input, 1, 1);
    if (analog.bActive)
        return Fail("API plana: GetAnalogActionData activo sin mandos");
    if (Export<uint64_t (*)(void*)>(module, "SteamAPI_ISteamHTMLSurface_CreateBrowser")(nullptr) != 0)
        return Fail("API plana: la entrada neutra no devolvió 0");
    const char* glyph =
        Export<const char* (*)(void*, int)>(module, "SteamAPI_ISteamInput_GetGlyphForXboxOrigin")(input, 0);
    if (!glyph || glyph[0] != 0)
        return Fail("API plana: la entrada neutra de cadena no devolvió una cadena vacía");

    // Despacho manual de un call result, como Steamworks.NET: SteamAPICallCompleted_t (703) y
    // después SteamAPI_ManualDispatch_GetAPICallResult. Es lo que espera el inicio de sesión de 7DTD.
    const uint64_t call =
        Export<uint64_t (*)(void*, const void*, int)>(module, "SteamAPI_ISteamUser_RequestEncryptedAppTicket")(
            user, nullptr, 0);
    if (call == 0)
        return Fail("RequestEncryptedAppTicket no devolvió una llamada");
    auto next = Export<bool (*)(int, sfemu::CallbackMsg_t*)>(module, "SteamAPI_ManualDispatch_GetNextCallback");
    auto release = Export<void (*)(int)>(module, "SteamAPI_ManualDispatch_FreeLastCallback");
    auto result = Export<bool (*)(int, uint64_t, void*, int, int, bool*)>(
        module, "SteamAPI_ManualDispatch_GetAPICallResult");
    bool delivered = false;
    sfemu::CallbackMsg_t message{};
    for (int i = 0; i < 100 && !delivered && next(1, &message); i++) {
        if (message.m_iCallback == sfemu::k_iCallback_SteamAPICallCompleted) {
            const auto* completed = reinterpret_cast<const sfemu::SteamAPICallCompleted_t*>(message.m_pubParam);
            if (completed->m_hAsyncCall == call) {
                if (completed->m_iCallback != sfemu::k_iCallback_EncryptedAppTicketResponse)
                    return Fail("call result anunciado con un callback distinto");
                sfemu::EncryptedAppTicketResponse_t response{};
                bool failed = true;
                if (!result(1, call, &response, sizeof(response), completed->m_iCallback, &failed) || failed ||
                    response.m_eResult != sfemu::k_EResultOK)
                    return Fail("GetAPICallResult no entregó EncryptedAppTicketResponse_t");
                delivered = true;
            }
        } else if (message.m_iCallback == sfemu::k_iCallback_EncryptedAppTicketResponse) {
            return Fail("call result entregado como callback normal (sin SteamAPICallCompleted_t)");
        }
        release(1);
    }
    if (!delivered)
        return Fail("el call result de RequestEncryptedAppTicket no llegó por el despacho manual");
    uint8_t ticket[256] = {};
    uint32_t ticketSize = 0;
    if (!Export<bool (*)(void*, void*, int, uint32_t*)>(module, "SteamAPI_ISteamUser_GetEncryptedAppTicket")(
            user, ticket, sizeof(ticket), &ticketSize) ||
        ticketSize == 0)
        return Fail("GetEncryptedAppTicket vacío");
    return 0;
}

int RunPeer(const std::wstring& dir) {
    SteamEmuApi api;
    if (!api.Load(dir))
        return Fail("par: no se pudo cargar steam_api64.dll");
    if (!api.InitSafe())
        return Fail("par: SteamAPI_InitSafe falló");
    if (api.User()->GetSteamID() != kSteamIdB)
        return Fail("par: SteamID distinto al configurado");
    if (std::strcmp(api.Friends()->GetPersonaName(), "SplitForgeTestB") != 0)
        return Fail("par: nombre de persona incorrecto");
    api.UserStats()->RequestCurrentStats();
    sfemu::CallbackMsg_t message{};
    bool statsOk = false;
    for (int i = 0; i < 50 && !statsOk; i++) {
        if (api.ManualDispatchGetNext(1, &message)) {
            statsOk = message.m_iCallback == sfemu::k_iCallback_UserStatsReceived;
            api.ManualDispatchFree(1);
        } else {
            Sleep(100);
        }
    }
    if (!statsOk)
        return Fail("par: no llegó el callback de stats");
    const std::wstring ready = TempDir() + L"\\ready.txt";
    if (!WriteJson(ready, "listo"))
        return Fail("par: no se pudo escribir ready");
    const std::wstring ack = TempDir() + L"\\ack.txt";
    bool received = false;
    for (int i = 0; i < 200 && !received; i++) {
        uint32_t size = 0;
        if (api.Networking()->IsP2PPacketAvailable(&size, 0)) {
            char buffer[64] = {};
            uint32_t read = 0;
            uint64_t sender = 0;
            if (api.Networking()->ReadP2PPacket(buffer, sizeof(buffer), &read, &sender, 0) &&
                read == 4 && std::memcmp(buffer, "PING", 4) == 0 && sender == kSteamIdA) {
                received = true;
                api.Networking()->SendP2PPacket(sender, "PONG", 4, sfemu::k_EP2PSendReliable, 0);
                WriteJson(ack, "recibido");
            }
        } else {
            Sleep(100);
        }
    }
    if (!received)
        return Fail("par: no recibió el PING del padre");
    Sleep(2000);
    api.Shutdown();
    return 0;
}

int RunParent(const std::wstring& dir, const std::wstring& exePath) {
    const std::wstring root = TempDir();
    if (!CreateDirectoryW(root.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
        return Fail("no se pudo crear el directorio temporal");
    const std::wstring storageA = root + L"\\A";
    const std::wstring storageB = root + L"\\B";
    CreateDirectoryW(storageA.c_str(), nullptr);
    CreateDirectoryW(storageB.c_str(), nullptr);
    const std::wstring configA = root + L"\\configA.json";
    const std::wstring configB = root + L"\\configB.json";
    const std::string jsonA =
        "{\"app_id\":251570,\"steam_id\":\"76561198000000001\",\"persona_name\":"
        "\"SplitForgeTestA\",\"language\":\"spanish\",\"ip_country\":\"ES\",\"storage_dir\":\"";
    std::string storageA8 = WideToUtf8Local(storageA);
    std::string storageB8 = WideToUtf8Local(storageB);
    for (char& c : storageA8) {
        if (c == '\\')
            c = '/';
    }
    for (char& c : storageB8) {
        if (c == '\\')
            c = '/';
    }
    const std::string fullA = jsonA + storageA8 + "\",\"net_port\":27117,\"net_discovery_seconds\":1}";
    const std::string fullB = "{\"app_id\":251570,\"steam_id\":\"76561198000000002\",\"persona_name\":"
                              "\"SplitForgeTestB\",\"storage_dir\":\"" +
                              storageB8 + "\",\"net_port\":27117,\"net_discovery_seconds\":1}";
    if (!WriteJson(configA, fullA.c_str()) || !WriteJson(configB, fullB.c_str()))
        return Fail("no se pudieron escribir las configuraciones");
    SetConfigEnv(configA);

    SteamEmuApi api;
    if (!api.Load(dir))
        return Fail("no se pudo cargar steam_api64.dll");
    if (!api.InitSafe())
        return Fail("SteamAPI_InitSafe falló");
    if (api.User()->GetSteamID() != kSteamIdA)
        return Fail("SteamID distinto al configurado");
    if (std::strcmp(api.Friends()->GetPersonaName(), "SplitForgeTestA") != 0)
        return Fail("nombre de persona incorrecto");
    if (api.Utils()->GetAppID() != 251570)
        return Fail("AppID incorrecto");
    if (!api.Apps()->BIsSubscribedApp(251570) || api.Apps()->BIsSubscribedApp(999999))
        return Fail("propiedad de apps incorrecta");

    // Accesores de los juegos C++ (steam_api_internal.h): { función de inicio, contador, hueco }.
    g_findUserInterface = api.FindOrCreateUserInterface;
    void* userAccessor[3] = {reinterpret_cast<void*>(&InitUserAccessor), nullptr, nullptr};
    auto** userSlot = static_cast<void**>(api.ContextInit(userAccessor));
    if (!userSlot || *userSlot != static_cast<void*>(api.User()) || g_accessorCalls != 1)
        return Fail("ContextInit no resolvió el accesor de SteamUser");
    if (api.ContextInit(userAccessor) != userSlot || g_accessorCalls != 1)
        return Fail("ContextInit no reutilizó el accesor ya resuelto");
    if (api.NetworkingSockets() == nullptr || api.NetworkingUtils() == nullptr)
        return Fail("accesores planos de Networking nulos");

    api.UserStats()->RequestCurrentStats();
    sfemu::CallbackMsg_t message{};
    bool statsOk = false;
    for (int i = 0; i < 50 && !statsOk; i++) {
        if (api.ManualDispatchGetNext(1, &message)) {
            if (message.m_iCallback == sfemu::k_iCallback_UserStatsReceived) {
                const auto* result = reinterpret_cast<const sfemu::UserStatsReceived_t*>(message.m_pubParam);
                statsOk = result->m_eResult == sfemu::k_EResultOK;
            }
            api.ManualDispatchFree(1);
        } else {
            Sleep(100);
        }
    }
    if (!statsOk)
        return Fail("no llegó el callback de stats");
    api.UserStats()->SetStat("Kills", 42);
    int32_t kills = 0;
    if (!api.UserStats()->GetStat("Kills", &kills) || kills != 42)
        return Fail("stat de ida y vuelta incorrecto");
    if (!api.UserStats()->StoreStats())
        return Fail("StoreStats falló");
    if (const int flat = CheckFlatApi(api.module); flat != 0)
        return flat;
    const std::wstring statsFile = storageA + L"\\stats.json";
    if (!FileExists(statsFile))
        return Fail("stats.json no persistido");

    const char payload[] = "Hola SplitForge";
    if (!api.RemoteStorage()->FileWrite("hello.txt", payload, sizeof(payload)))
        return Fail("FileWrite falló");
    if (!api.RemoteStorage()->FileExists("hello.txt"))
        return Fail("FileExists falló");
    char buffer[64] = {};
    const int32_t read = api.RemoteStorage()->FileRead("hello.txt", buffer, sizeof(buffer));
    if (read != static_cast<int32_t>(sizeof(payload)) ||
        std::memcmp(buffer, payload, sizeof(payload)) != 0)
        return Fail("lectura remota distinta a lo escrito");
    if (!api.RemoteStorage()->FileDelete("hello.txt") || api.RemoteStorage()->FileExists("hello.txt"))
        return Fail("FileDelete falló");
    char userData[260] = {};
    if (!api.User()->GetUserDataFolder(userData, sizeof(userData)) || userData[0] == 0)
        return Fail("GetUserDataFolder vacío");

    const std::wstring ready = root + L"\\ready.txt";
    const std::wstring ack = root + L"\\ack.txt";
    DeleteFileW(ready.c_str());
    DeleteFileW(ack.c_str());
    HANDLE child = nullptr;
    if (!SpawnPeer(exePath, dir, configB, &child))
        return 1;
    SetConfigEnv(configA);
    if (!WaitForFile(ready, 10000))
        return Fail("el par no informó que estaba listo");
    bool pongOk = false;
    for (int i = 0; i < 100 && !pongOk; i++) {
        api.Networking()->SendP2PPacket(kSteamIdB, "PING", 4, sfemu::k_EP2PSendReliable, 0);
        for (int j = 0; j < 5 && !pongOk; j++) {
            uint32_t size = 0;
            if (api.Networking()->IsP2PPacketAvailable(&size, 0)) {
                char pong[64] = {};
                uint32_t pongRead = 0;
                uint64_t sender = 0;
                if (api.Networking()->ReadP2PPacket(pong, sizeof(pong), &pongRead, &sender, 0) &&
                    pongRead == 4 && std::memcmp(pong, "PONG", 4) == 0 && sender == kSteamIdB) {
                    pongOk = true;
                }
            } else {
                Sleep(200);
            }
        }
        if (!FileExists(ack))
            Sleep(200);
    }
    if (!pongOk && !FileExists(ack))
        return Fail("no hubo intercambio P2P entre los dos procesos");
    const std::string ackText = ReadTextFile(ack);
    if (ackText.find("recibido") == std::string::npos)
        return Fail("el par no confirmó el PING");
    api.Shutdown();
    WaitForSingleObject(child, 15000);
    DWORD exitCode = 1;
    GetExitCodeProcess(child, &exitCode);
    CloseHandle(child);
    if (exitCode != 0)
        return Fail("el proceso par terminó con error");
    std::printf("OK: identidad, API plana de Steamworks.NET, stats, almacenamiento y P2P local entre dos procesos.\n");
    return 0;
}

} // namespace

int wmain(int argc, wchar_t** argv) {
    if (argc < 2)
        return Fail("uso: smoke_test <directorio con steam_api64.dll> [--peer]");
    const std::wstring dir = argv[1];
    if (argc >= 3 && wcscmp(argv[2], L"--peer") == 0)
        return RunPeer(dir);
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    return RunParent(dir, exePath);
}
