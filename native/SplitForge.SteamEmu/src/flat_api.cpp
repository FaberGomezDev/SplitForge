#include "impls.h"
#include "interfaces.h"
#include "state.h"

#include <windows.h>

#include <algorithm>
#include <cstring>

#define S_API

namespace sfemu {

namespace {

HMODULE g_self = nullptr;

std::string ConfigPath() {
    wchar_t overridePath[MAX_PATH] = {};
    if (GetEnvironmentVariableW(L"SPLITFORGE_STEAMEMU_CONFIG", overridePath, MAX_PATH) > 0)
        return WideToUtf8(overridePath);
    wchar_t modulePath[MAX_PATH] = {};
    const DWORD length = GetModuleFileNameW(g_self, modulePath, MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
        return {};
    std::string path = WideToUtf8(modulePath);
    const size_t slash = path.find_last_of("\\/");
    if (slash == std::string::npos)
        return {};
    return path.substr(0, slash + 1) + "SplitForge.SteamEmu.json";
}

void DispatchBoth() {
    Emu().DispatchRunCallbacks();
}

bool ShutdownInternal() {
    EmuState& emu = Emu();
    if (!emu.initialized)
        return true;
    emu.SaveStats();
    emu.net.Stop();
    emu.initialized = false;
    return true;
}

} // namespace

void* ResolveGeneric(const char* version) {
    if (!version)
        return nullptr;
    // Coincidencia por familia: cada juego puede pedir una revisión distinta del SDK.
    // El orden importa: los prefijos más específicos van antes de "SteamNetworking".
    if (std::strncmp(version, "SteamNetworkingSockets", 22) == 0)
        return NetworkingSocketsInstance();
    if (std::strncmp(version, "SteamNetworkingUtils", 20) == 0)
        return NetworkingUtilsInstance();
    if (std::strncmp(version, "SteamNetworkingMessages", 23) == 0)
        return NetworkingMessagesInstance();
    if (std::strncmp(version, "SteamNetworking", 15) == 0)
        return NetworkingInstance();
    if (std::strncmp(version, "SteamClient", 11) == 0)
        return ClientInstance();
    if (std::strncmp(version, "SteamUser", 9) == 0)
        return UserInstance();
    if (std::strncmp(version, "SteamFriends", 12) == 0)
        return FriendsInstance();
    if (std::strncmp(version, "SteamUtils", 10) == 0)
        return UtilsInstance();
    if (std::strncmp(version, "STEAMUSERSTATS_INTERFACE_VERSION", 32) == 0)
        return UserStatsInstance();
    if (std::strncmp(version, "SteamMatchMakingServers", 23) == 0)
        return MatchmakingServersInstance();
    if (std::strncmp(version, "SteamMatchMaking", 16) == 0)
        return MatchmakingInstance();
    if (std::strncmp(version, "SteamMatchGameSearch", 20) == 0)
        return GameSearchInstance();
    if (std::strncmp(version, "STEAMREMOTESTORAGE_INTERFACE_VERSION", 36) == 0)
        return RemoteStorageInstance();
    if (std::strncmp(version, "STEAMAPPS_INTERFACE_VERSION", 27) == 0)
        return AppsInstance();
    if (std::strncmp(version, "STEAMUGC_INTERFACE_VERSION", 26) == 0)
        return UgcInstance();
    if (std::strncmp(version, "STEAMINVENTORY_INTERFACE_V", 26) == 0)
        return InventoryInstance();
    if (std::strncmp(version, "STEAMHTTP_INTERFACE_VERSION", 27) == 0)
        return HttpInstance();
    if (std::strncmp(version, "STEAMSCREENSHOTS_INTERFACE_VERSION", 34) == 0)
        return ScreenshotsInstance();
    if (std::strncmp(version, "SteamController", 15) == 0)
        return ControllerInstance();
    if (std::strncmp(version, "SteamInput", 10) == 0)
        return InputInstance();
    if (std::strncmp(version, "STEAMMUSICREMOTE_INTERFACE_VERSION", 34) == 0)
        return MusicRemoteInstance();
    if (std::strncmp(version, "STEAMMUSIC_INTERFACE_VERSION", 28) == 0)
        return MusicInstance();
    if (std::strncmp(version, "STEAMHTMLSURFACE_INTERFACE_VERSION", 34) == 0)
        return HtmlSurfaceInstance();
    if (std::strncmp(version, "STEAMVIDEO_INTERFACE_V", 22) == 0)
        return VideoInstance();
    if (std::strncmp(version, "STEAMPARENTALSETTINGS_INTERFACE_VERSION", 39) == 0)
        return ParentalSettingsInstance();
    if (std::strncmp(version, "STEAMAPPLIST_INTERFACE_VERSION", 30) == 0)
        return AppListInstance();
    if (std::strncmp(version, "SteamParties", 12) == 0)
        return PartiesInstance();
    if (std::strncmp(version, "STEAMREMOTEPLAY_INTERFACE_VERSION", 34) == 0)
        return RemotePlayInstance();
    if (std::strncmp(version, "SteamGameServerStats", 20) == 0)
        return GameServerStatsInstance();
    if (std::strncmp(version, "SteamGameServer", 15) == 0)
        return GameServerInstance();
    return nullptr;
}

} // namespace sfemu

using namespace sfemu;

extern "C" {

S_API bool SteamAPI_Init() {
    EmuState& emu = Emu();
    emu.Log("API: SteamAPI_Init pid=%lu tid=%lu yaIniciado=%d", GetCurrentProcessId(),
            GetCurrentThreadId(), emu.initialized ? 1 : 0);
    if (emu.initialized)
        return true;
    const std::string path = ConfigPath();
    if (!path.empty())
        LoadConfig(emu.config, path);
    if (!emu.EnsureDirs()) {
        emu.Log("API: SteamAPI_Init -> false (sin dirs)");
        return false;
    }
    emu.initTicks = GetTickCount64();
    emu.initialized = true;
    const bool netOk = emu.net.Start(emu.config.multicastAddress, emu.config.netPort,
                                     emu.config.steamId, emu.config.appId, emu.config.discoverySeconds);
    emu.Log("SplitForge.SteamEmu iniciado: app=%u steamid=%llu persona=%s config=%s red=%s",
            emu.config.appId, static_cast<unsigned long long>(emu.config.steamId),
            emu.config.personaName.c_str(), path.c_str(), netOk ? "OK" : "NO");
    return true;
}

S_API bool SteamAPI_InitSafe() {
    Emu().Log("API: SteamAPI_InitSafe pid=%lu tid=%lu", GetCurrentProcessId(), GetCurrentThreadId());
    return SteamAPI_Init();
}

S_API void SteamAPI_Shutdown() {
    Emu().Log("API: SteamAPI_Shutdown pid=%lu tid=%lu", GetCurrentProcessId(), GetCurrentThreadId());
    ShutdownInternal();
}

S_API bool SteamAPI_RestartAppIfNecessary(uint32_t) {
    return false;
}

S_API void SteamAPI_ReleaseCurrentThreadMemory() {}

S_API void SteamAPI_WriteMiniDump(uint32_t, void*, uint32_t) {}

S_API void SteamAPI_SetMiniDumpComment(const char*) {}

S_API void SteamAPI_UseBreakpadCrashHandler(void*, void*, const char*, unsigned int) {}

S_API void SteamAPI_SetBreakpadAppID(uint32_t) {}

S_API void SteamAPI_RegisterCallback(void* object, int iCallback) {
    EmuState& emu = Emu();
    for (const auto& existing : emu.callbacks) {
        if (existing.object == object && existing.iCallback == iCallback)
            return;
    }
    emu.callbacks.push_back({object, iCallback});
}

S_API void SteamAPI_UnregisterCallback(void* object) {
    EmuState& emu = Emu();
    emu.callbacks.erase(
        std::remove_if(emu.callbacks.begin(), emu.callbacks.end(),
                       [object](const RegisteredCallback& entry) { return entry.object == object; }),
        emu.callbacks.end());
}

S_API void SteamAPI_RegisterCallResult(void* object, uint64_t apiCall) {
    EmuState& emu = Emu();
    for (const auto& existing : emu.callResults) {
        if (existing.object == object && existing.apiCall == apiCall)
            return;
    }
    emu.callResults.push_back({object, apiCall});
}

S_API void SteamAPI_UnregisterCallResult(void* object, uint64_t apiCall) {
    EmuState& emu = Emu();
    emu.callResults.erase(
        std::remove_if(emu.callResults.begin(), emu.callResults.end(),
                       [object, apiCall](const RegisteredCallResult& entry) {
                           return entry.object == object && entry.apiCall == apiCall;
                       }),
        emu.callResults.end());
}

S_API void SteamAPI_RegisterGlobalCallback(void* object, int iCallback) {
    EmuState& emu = Emu();
    for (const auto& existing : emu.globalCallbacks) {
        if (existing.object == object && existing.iCallback == iCallback)
            return;
    }
    emu.globalCallbacks.push_back({object, iCallback});
}

S_API void SteamAPI_UnregisterGlobalCallback(void* object) {
    EmuState& emu = Emu();
    emu.globalCallbacks.erase(
        std::remove_if(emu.globalCallbacks.begin(), emu.globalCallbacks.end(),
                       [object](const RegisteredCallback& entry) { return entry.object == object; }),
        emu.globalCallbacks.end());
}

S_API void SteamAPI_RegisterGlobalCallResult(void* object, uint64_t apiCall) {
    EmuState& emu = Emu();
    for (const auto& existing : emu.globalCallResults) {
        if (existing.object == object && existing.apiCall == apiCall)
            return;
    }
    emu.globalCallResults.push_back({object, apiCall});
}

S_API void SteamAPI_UnregisterGlobalCallResult(void* object, uint64_t apiCall) {
    EmuState& emu = Emu();
    emu.globalCallResults.erase(
        std::remove_if(emu.globalCallResults.begin(), emu.globalCallResults.end(),
                       [object, apiCall](const RegisteredCallResult& entry) {
                           return entry.object == object && entry.apiCall == apiCall;
                       }),
        emu.globalCallResults.end());
}

S_API void SteamAPI_RunCallbacks() {
    DispatchBoth();
}

S_API bool SteamAPI_IsSteamRunning() {
    return Emu().initialized;
}

S_API int SteamAPI_GetHSteamUser() {
    return 1;
}

S_API int SteamAPI_GetHSteamPipe() {
    return 1;
}

S_API void SteamAPI_ManualDispatch_Init() {}

S_API void SteamAPI_ManualDispatch_RunFrame(int) {}

S_API bool SteamAPI_ManualDispatch_GetNextCallback(int, CallbackMsg_t* message) {
    return Emu().PopManual(message);
}

S_API void SteamAPI_ManualDispatch_FreeLastCallback(int) {
    Emu().FreeLastManual();
}

S_API bool SteamAPI_ManualDispatch_GetAPICallResult(int, uint64_t apiCall, void* out, int size,
                                                    int expected, bool* failed) {
    return Emu().GetApiResult(apiCall, out, static_cast<size_t>(size > 0 ? size : 0), expected, failed);
}

S_API bool SteamAPI_GetSteamInstallPath(char*, uint32_t) {
    return false;
}

S_API void* SteamInternal_ContextInit(void* data) {
    if (!data)
        return nullptr;
    // La estructura que pasa Steamworks.NET: puntero de función (sin usar) + cadena de
    // versiones separadas por '?' (p. ej. "SteamClient020?SteamUser023?SteamFriends017?…").
    // Se devuelve la interfaz del PRIMER segmento reconocible de la cadena.
    struct ContextInitData {
        void* pFn;
        const char* pchVersionString;
    };
    auto* context = static_cast<ContextInitData*>(data);
    if (!context->pchVersionString)
        return nullptr;
    std::string chain = context->pchVersionString;
    size_t start = 0;
    while (start <= chain.size()) {
        const size_t end = chain.find('?', start);
        const std::string segment = chain.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (!segment.empty()) {
            if (void* found = ResolveGeneric(segment.c_str()))
                return found;
        }
        if (end == std::string::npos)
            break;
        start = end + 1;
    }
    return nullptr;
}

S_API void* SteamInternal_CreateInterface(const char* version) {
    return ResolveGeneric(version);
}

S_API void* SteamInternal_FindOrCreateUserInterface(int, const char* version) {
    return ResolveGeneric(version);
}

S_API void* SteamInternal_FindOrCreateGameServerInterface(int, int, const char* version) {
    return ResolveGeneric(version);
}

S_API void* SteamInternal_FindOrCreateGlobalInterface(int, const char* version) {
    return ResolveGeneric(version);
}

S_API bool SteamGameServer_Init(uint32_t, uint16_t, uint16_t, uint16_t, int, const char*) {
    Emu().gameServerInitialized = true;
    return true;
}

S_API bool SteamGameServer_InitSafe(uint32_t, uint16_t, uint16_t, uint16_t, int, const char*) {
    return SteamGameServer_Init(0, 0, 0, 0, 0, nullptr);
}

S_API void SteamGameServer_Shutdown() {
    Emu().gameServerInitialized = false;
}

S_API void SteamGameServer_RunCallbacks() {
    DispatchBoth();
}

S_API bool SteamGameServer_BSecure() {
    return false;
}

S_API int SteamGameServer_GetHSteamUser() {
    return 1;
}

S_API int SteamGameServer_GetHSteamPipe() {
    return 1;
}

S_API uint64_t SteamGameServer_GetSteamID() {
    return MakeSteamID(0, 1, k_EAccountTypeGameServer);
}

S_API void SteamGameServer_RegisterCallback(void* object, int iCallback) {
    SteamAPI_RegisterCallback(object, iCallback);
}

S_API void SteamGameServer_UnregisterCallback(void* object) {
    SteamAPI_UnregisterCallback(object);
}

S_API void SteamGameServer_RegisterCallResult(void* object, uint64_t apiCall) {
    SteamAPI_RegisterCallResult(object, apiCall);
}

S_API void SteamGameServer_UnregisterCallResult(void* object, uint64_t apiCall) {
    SteamAPI_UnregisterCallResult(object, apiCall);
}

S_API void SteamGameServer_ManualDispatch_Init() {}

S_API void SteamGameServer_ManualDispatch_RunFrame(int) {}

S_API bool SteamGameServer_ManualDispatch_GetNextCallback(int, CallbackMsg_t* message) {
    return Emu().PopManual(message);
}

S_API void SteamGameServer_ManualDispatch_FreeLastCallback(int) {
    Emu().FreeLastManual();
}

S_API bool SteamGameServer_ManualDispatch_GetAPICallResult(int, uint64_t apiCall, void* out, int size,
                                                           int expected, bool* failed) {
    return Emu().GetApiResult(apiCall, out, static_cast<size_t>(size > 0 ? size : 0), expected, failed);
}

// Accesores planos (estilo steam_api_flat.h).
S_API ISteamClient* SteamClient() { return ClientInstance(); }
S_API ISteamUser* SteamUser() { return UserInstance(); }
S_API ISteamFriends* SteamFriends() { return FriendsInstance(); }
S_API ISteamUtils* SteamUtils() { return UtilsInstance(); }
S_API ISteamMatchmaking* SteamMatchmaking() { return MatchmakingInstance(); }
S_API ISteamMatchmakingServers* SteamMatchmakingServers() { return MatchmakingServersInstance(); }
S_API ISteamUserStats* SteamUserStats() { return UserStatsInstance(); }
S_API ISteamApps* SteamApps() { return AppsInstance(); }
S_API ISteamNetworking* SteamNetworking() { return NetworkingInstance(); }
S_API ISteamRemoteStorage* SteamRemoteStorage() { return RemoteStorageInstance(); }
S_API ISteamScreenshots* SteamScreenshots() { return ScreenshotsInstance(); }
S_API ISteamHTTP* SteamHTTP() { return HttpInstance(); }
S_API ISteamController* SteamController() { return ControllerInstance(); }
S_API ISteamUGC* SteamUGC() { return UgcInstance(); }
S_API ISteamAppList* SteamAppList() { return AppListInstance(); }
S_API ISteamMusic* SteamMusic() { return MusicInstance(); }
S_API void* SteamMusicRemote() { return MusicRemoteInstance(); }
S_API void* SteamHTMLSurface() { return HtmlSurfaceInstance(); }
S_API ISteamInventory* SteamInventory() { return InventoryInstance(); }
S_API ISteamVideo* SteamVideo() { return VideoInstance(); }
S_API ISteamParentalSettings* SteamParentalSettings() { return ParentalSettingsInstance(); }
S_API ISteamInput* SteamInput() { return InputInstance(); }
S_API void* SteamParties() { return PartiesInstance(); }
S_API void* SteamRemotePlay() { return RemotePlayInstance(); }
S_API void* SteamTimeline() { return nullptr; }
S_API void* SteamGameSearch() { return GameSearchInstance(); }
S_API void* SteamNetworkingSockets() { return NetworkingSocketsInstance(); }
S_API void* SteamNetworkingUtils() { return NetworkingUtilsInstance(); }
S_API void* SteamNetworkingMessages() { return NetworkingMessagesInstance(); }
S_API void* SteamNetworkingFakeUDPPort() { return nullptr; }

// Accesores de servidor de juego.
S_API ISteamGameServer* SteamGameServer() { return GameServerInstance(); }
S_API ISteamClient* SteamGameServerClient() { return ClientInstance(); }
S_API ISteamUtils* SteamGameServerUtils() { return UtilsInstance(); }
S_API void* SteamGameServerNetworkingUtils() { return NetworkingUtilsInstance(); }
S_API ISteamUGC* SteamGameServerUGC() { return UgcInstance(); }
S_API ISteamHTTP* SteamGameServerHTTP() { return HttpInstance(); }
S_API ISteamInventory* SteamGameServerInventory() { return InventoryInstance(); }
S_API void* SteamGameServerNetworkingSockets() { return NetworkingSocketsInstance(); }
S_API ISteamGameServerStats* SteamGameServerStats() { return GameServerStatsInstance(); }
S_API ISteamNetworking* SteamGameServerNetworking() { return NetworkingInstance(); }
S_API ISteamApps* SteamGameServerApps() { return AppsInstance(); }
S_API ISteamInput* SteamGameServerInput() { return InputInstance(); }
S_API ISteamMatchmaking* SteamGameServerMatchmaking() { return MatchmakingInstance(); }
S_API void* SteamGameServerTimeline() { return nullptr; }
S_API void* SteamGameServerNetworkingMessages() { return NetworkingMessagesInstance(); }

// Accesores clásicos ISteamClient.
S_API ISteamUser* SteamAPI_ISteamClient_GetISteamUser(int user, int pipe, const char* version) {
    return ClientInstance()->GetISteamUser(user, pipe, version);
}
S_API ISteamGameServer* SteamAPI_ISteamClient_GetISteamGameServer(int user, int pipe, const char* version) {
    return ClientInstance()->GetISteamGameServer(user, pipe, version);
}
S_API ISteamFriends* SteamAPI_ISteamClient_GetISteamFriends(int user, int pipe, const char* version) {
    return ClientInstance()->GetISteamFriends(user, pipe, version);
}
S_API ISteamUtils* SteamAPI_ISteamClient_GetISteamUtils(int pipe, const char* version) {
    return ClientInstance()->GetISteamUtils(pipe, version);
}
S_API ISteamMatchmaking* SteamAPI_ISteamClient_GetISteamMatchmaking(int user, int pipe,
                                                                    const char* version) {
    return ClientInstance()->GetISteamMatchmaking(user, pipe, version);
}
S_API ISteamMatchmakingServers* SteamAPI_ISteamClient_GetISteamMatchmakingServers(int user, int pipe,
                                                                                  const char* version) {
    return ClientInstance()->GetISteamMatchmakingServers(user, pipe, version);
}
S_API void* SteamAPI_ISteamClient_GetISteamGenericInterface(int user, int pipe, const char* version) {
    return ClientInstance()->GetISteamGenericInterface(user, pipe, version);
}
S_API ISteamUserStats* SteamAPI_ISteamClient_GetISteamUserStats(int user, int pipe,
                                                                const char* version) {
    return ClientInstance()->GetISteamUserStats(user, pipe, version);
}
S_API ISteamGameServerStats* SteamAPI_ISteamClient_GetISteamGameServerStats(int user, int pipe,
                                                                            const char* version) {
    return ClientInstance()->GetISteamGameServerStats(user, pipe, version);
}
S_API ISteamApps* SteamAPI_ISteamClient_GetISteamApps(int user, int pipe, const char* version) {
    return ClientInstance()->GetISteamApps(user, pipe, version);
}
S_API ISteamNetworking* SteamAPI_ISteamClient_GetISteamNetworking(int user, int pipe,
                                                                  const char* version) {
    return ClientInstance()->GetISteamNetworking(user, pipe, version);
}
S_API ISteamRemoteStorage* SteamAPI_ISteamClient_GetISteamRemoteStorage(int user, int pipe,
                                                                        const char* version) {
    return ClientInstance()->GetISteamRemoteStorage(user, pipe, version);
}
S_API ISteamScreenshots* SteamAPI_ISteamClient_GetISteamScreenshots(int user, int pipe,
                                                                    const char* version) {
    return ClientInstance()->GetISteamScreenshots(user, pipe, version);
}
S_API void* SteamAPI_ISteamClient_GetISteamGameSearch(int user, int pipe, const char* version) {
    return ClientInstance()->GetISteamGameSearch(user, pipe, version);
}S_API ISteamHTTP* SteamAPI_ISteamClient_GetISteamHTTP(int user, int pipe, const char* version) {
    return ClientInstance()->GetISteamHTTP(user, pipe, version);
}
S_API ISteamController* SteamAPI_ISteamClient_GetISteamController(int user, int pipe,
                                                                  const char* version) {
    return ClientInstance()->GetISteamController(user, pipe, version);
}
S_API ISteamUGC* SteamAPI_ISteamClient_GetISteamUGC(int user, int pipe, const char* version) {
    return ClientInstance()->GetISteamUGC(user, pipe, version);
}
S_API ISteamAppList* SteamAPI_ISteamClient_GetISteamAppList(int user, int pipe, const char* version) {
    return ClientInstance()->GetISteamAppList(user, pipe, version);
}
S_API ISteamMusic* SteamAPI_ISteamClient_GetISteamMusic(int user, int pipe, const char* version) {
    return ClientInstance()->GetISteamMusic(user, pipe, version);
}
S_API void* SteamAPI_ISteamClient_GetISteamMusicRemote(int user, int pipe, const char* version) {
    return ClientInstance()->GetISteamMusicRemote(user, pipe, version);
}
S_API void* SteamAPI_ISteamClient_GetISteamHTMLSurface(int user, int pipe, const char* version) {
    return ClientInstance()->GetISteamHTMLSurface(user, pipe, version);
}S_API ISteamInventory* SteamAPI_ISteamClient_GetISteamInventory(int user, int pipe,
                                                                const char* version) {
    return ClientInstance()->GetISteamInventory(user, pipe, version);
}
S_API ISteamVideo* SteamAPI_ISteamClient_GetISteamVideo(int user, int pipe, const char* version) {
    return ClientInstance()->GetISteamVideo(user, pipe, version);
}
S_API ISteamParentalSettings* SteamAPI_ISteamClient_GetISteamParentalSettings(int user, int pipe,
                                                                              const char* version) {
    return ClientInstance()->GetISteamParentalSettings(user, pipe, version);
}
S_API ISteamInput* SteamAPI_ISteamClient_GetISteamInput(int user, int pipe, const char* version) {
    return ClientInstance()->GetISteamInput(user, pipe, version);
}
S_API void* SteamAPI_ISteamClient_GetISteamParties(int user, int pipe, const char* version) {
    return ClientInstance()->GetISteamParties(user, pipe, version);
}
S_API void* SteamAPI_ISteamClient_GetISteamRemotePlay(int user, int pipe, const char* version) {
    return ClientInstance()->GetISteamRemotePlay(user, pipe, version);
}
S_API void* SteamAPI_ISteamClient_GetISteamTimeline(int, int, const char*) { return nullptr; }
S_API void* SteamAPI_ISteamClient_GetISteamNetworkingSockets(int, int, const char*) {
    return NetworkingSocketsInstance();
}
S_API void* SteamAPI_ISteamClient_GetISteamNetworkingUtils(int, int, const char*) {
    return NetworkingUtilsInstance();
}
S_API void* SteamAPI_ISteamClient_GetISteamNetworkingMessages(int, int, const char*) {
    return NetworkingMessagesInstance();
}
S_API void* SteamAPI_ISteamClient_GetISteamNetworkingFakeUDPPort(int, int, const char*) {
    return nullptr;
}

// Accesores versionados: se resuelven con GetProcAddress por nombre exacto.
S_API ISteamUser* SteamAPI_SteamUser_v023() { return UserInstance(); }
S_API ISteamFriends* SteamAPI_SteamFriends_v017() { return FriendsInstance(); }
S_API ISteamUtils* SteamAPI_SteamUtils_v010() { return UtilsInstance(); }
S_API ISteamUserStats* SteamAPI_SteamUserStats_v012() { return UserStatsInstance(); }
S_API ISteamMatchmaking* SteamAPI_SteamMatchmaking_v009() { return MatchmakingInstance(); }
S_API ISteamMatchmakingServers* SteamAPI_SteamMatchmakingServers_v002() { return MatchmakingServersInstance(); }
S_API ISteamNetworking* SteamAPI_SteamNetworking_v006() { return NetworkingInstance(); }
S_API ISteamRemoteStorage* SteamAPI_SteamRemoteStorage_v016() { return RemoteStorageInstance(); }
S_API ISteamScreenshots* SteamAPI_SteamScreenshots_v003() { return ScreenshotsInstance(); }
S_API ISteamHTTP* SteamAPI_SteamHTTP_v003() { return HttpInstance(); }
S_API ISteamController* SteamAPI_SteamController_v008() { return ControllerInstance(); }
S_API ISteamUGC* SteamAPI_SteamUGC_v020() { return UgcInstance(); }
S_API ISteamAppList* SteamAPI_SteamAppList_v001() { return AppListInstance(); }
S_API ISteamMusic* SteamAPI_SteamMusic_v001() { return MusicInstance(); }
S_API void* SteamAPI_SteamMusicRemote_v001() { return MusicRemoteInstance(); }
S_API void* SteamAPI_SteamHTMLSurface_v005() { return HtmlSurfaceInstance(); }
S_API ISteamInventory* SteamAPI_SteamInventory_v003() { return InventoryInstance(); }
S_API ISteamVideo* SteamAPI_SteamVideo_v002() { return VideoInstance(); }
S_API ISteamParentalSettings* SteamAPI_SteamParentalSettings_v001() { return ParentalSettingsInstance(); }
S_API ISteamInput* SteamAPI_SteamInput_v006() { return InputInstance(); }
S_API void* SteamAPI_SteamParties_v002() { return PartiesInstance(); }
S_API void* SteamAPI_SteamRemotePlay_v002() { return RemotePlayInstance(); }
S_API void* SteamAPI_SteamTimeline_v001() { return nullptr; }
S_API void* SteamAPI_SteamGameSearch_v001() { return GameSearchInstance(); }
S_API ISteamGameServer* SteamAPI_SteamGameServer_v016() { return GameServerInstance(); }
S_API ISteamGameServer* SteamAPI_SteamGameServer_v015() { return GameServerInstance(); }
S_API ISteamGameServerStats* SteamAPI_SteamGameServerStats_v001() { return GameServerStatsInstance(); }
S_API void* SteamAPI_SteamNetworkingSockets_v012() { return NetworkingSocketsInstance(); }
S_API void* SteamAPI_SteamNetworkingUtils_v004() { return NetworkingUtilsInstance(); }
S_API void* SteamAPI_SteamNetworkingMessages_v002() { return NetworkingMessagesInstance(); }
S_API void* SteamAPI_SteamNetworkingFakeUDPPort_v001() { return nullptr; }

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_self = instance;
        DisableThreadLibraryCalls(instance);
    }
    if (reason == DLL_PROCESS_DETACH) {
        ShutdownInternal();
    }
    return TRUE;
}

} // extern "C"
