#include "impls.h"
#include "interfaces.h"
#include "state.h"

#include <windows.h>

#include <cstdio>
#include <cstring>
#include <ctime>

namespace sfemu {

namespace {

bool IsUserVersion(const char* version) {
    return version && std::strncmp(version, "SteamUser", 9) == 0;
}

bool IsFriendsVersion(const char* version) {
    return version && std::strncmp(version, "SteamFriends", 12) == 0;
}

bool IsUtilsVersion(const char* version) {
    return version && std::strncmp(version, "SteamUtils", 10) == 0;
}

bool IsAppsVersion(const char* version) {
    return version && std::strncmp(version, "STEAMAPPS_INTERFACE_VERSION", 27) == 0;
}

uint64_t GameIdFromApp(AppId_t appId) {
    return static_cast<uint64_t>(appId);
}

class SteamClientImpl final : public ISteamClient {
public:
    HSteamPipe CreateSteamPipe() override { return 1; }
    bool BReleaseSteamPipe(HSteamPipe) override { return true; }
    HSteamUser ConnectToGlobalUser(HSteamPipe) override { return 1; }
    HSteamUser CreateLocalUser(HSteamPipe*, EAccountType) override { return 1; }
    void ReleaseUser(HSteamPipe, HSteamUser) override {}
    ISteamUser* GetISteamUser(HSteamUser, HSteamPipe, const char* version) override {
        return IsUserVersion(version) ? UserInstance() : nullptr;
    }
    ISteamGameServer* GetISteamGameServer(HSteamUser, HSteamPipe, const char*) override {
        return GameServerInstance();
    }
    void SetLocalIPBinding(const SteamIPAddress_t&, uint16_t) override {}
    ISteamFriends* GetISteamFriends(HSteamUser, HSteamPipe, const char* version) override {
        return IsFriendsVersion(version) ? FriendsInstance() : nullptr;
    }
    ISteamUtils* GetISteamUtils(HSteamPipe, const char* version) override {
        return IsUtilsVersion(version) ? UtilsInstance() : nullptr;
    }
    ISteamMatchmaking* GetISteamMatchmaking(HSteamUser, HSteamPipe, const char*) override {
        return MatchmakingInstance();
    }
    ISteamMatchmakingServers* GetISteamMatchmakingServers(HSteamUser, HSteamPipe, const char*) override {
        return MatchmakingServersInstance();
    }
    void* GetISteamGenericInterface(HSteamUser, HSteamPipe, const char* version) override {
        return ResolveGeneric(version);
    }
    ISteamUserStats* GetISteamUserStats(HSteamUser, HSteamPipe, const char*) override {
        return UserStatsInstance();
    }
    ISteamGameServerStats* GetISteamGameServerStats(HSteamUser, HSteamPipe, const char*) override {
        return GameServerStatsInstance();
    }
    ISteamApps* GetISteamApps(HSteamUser, HSteamPipe, const char* version) override {
        return IsAppsVersion(version) ? AppsInstance() : nullptr;
    }
    ISteamNetworking* GetISteamNetworking(HSteamUser, HSteamPipe, const char*) override {
        return NetworkingInstance();
    }
    ISteamRemoteStorage* GetISteamRemoteStorage(HSteamUser, HSteamPipe, const char*) override {
        return RemoteStorageInstance();
    }
    ISteamScreenshots* GetISteamScreenshots(HSteamUser, HSteamPipe, const char*) override {
        return ScreenshotsInstance();
    }
    void* GetISteamGameSearch(HSteamUser, HSteamPipe, const char*) override {
        return GameSearchInstance();
    }
    void RunFrame() override {}
    uint32_t GetIPCCallCount() override {
        EmuState& emu = Emu();
        return ++emu.ipcCallCount;
    }
    void SetWarningMessageHook(void* hook) override { Emu().warningHook = hook; }
    bool BShutdownIfAllPipesClosed() override { return true; }
    ISteamHTTP* GetISteamHTTP(HSteamUser, HSteamPipe, const char*) override { return HttpInstance(); }
    ISteamController* GetISteamController(HSteamUser, HSteamPipe, const char*) override {
        return ControllerInstance();
    }
    ISteamUGC* GetISteamUGC(HSteamUser, HSteamPipe, const char*) override { return UgcInstance(); }
    ISteamAppList* GetISteamAppList(HSteamUser, HSteamPipe, const char*) override { return AppListInstance(); }
    ISteamMusic* GetISteamMusic(HSteamUser, HSteamPipe, const char*) override { return MusicInstance(); }
    void* GetISteamMusicRemote(HSteamUser, HSteamPipe, const char*) override {
        return MusicRemoteInstance();
    }
    void* GetISteamHTMLSurface(HSteamUser, HSteamPipe, const char*) override {
        return HtmlSurfaceInstance();
    }
    void Set_SteamAPI_CPostAPIResultInProcess(void*) override {}
    void Remove_SteamAPI_CPostAPIResultInProcess(void*) override {}
    void Set_SteamAPI_CCheckCallbackRegisteredInProcess(void*) override {}
    ISteamInventory* GetISteamInventory(HSteamUser, HSteamPipe, const char*) override {
        return InventoryInstance();
    }
    ISteamVideo* GetISteamVideo(HSteamUser, HSteamPipe, const char*) override { return VideoInstance(); }
    ISteamParentalSettings* GetISteamParentalSettings(HSteamUser, HSteamPipe, const char*) override {
        return ParentalSettingsInstance();
    }
    ISteamInput* GetISteamInput(HSteamUser, HSteamPipe, const char*) override { return InputInstance(); }
    void* GetISteamParties(HSteamUser, HSteamPipe, const char*) override { return PartiesInstance(); }
    void* GetISteamRemotePlay(HSteamUser, HSteamPipe, const char*) override {
        return RemotePlayInstance();
    }
};

class SteamUserImpl final : public ISteamUser {
public:
    HSteamUser GetHSteamUser() override { return 1; }
    bool BLoggedOn() override { return Emu().initialized; }
    CSteamID GetSteamID() override { return Emu().config.steamId; }
    int InitiateGameConnection_DEPRECATED(void*, int, CSteamID, uint32_t, uint16_t, bool) override { return 0; }
    void TerminateGameConnection_DEPRECATED(uint32_t, uint16_t) override {}
    void TrackAppUsageEvent(CGameID, int, const char*) override {}
    bool GetUserDataFolder(char* buffer, int size) override {
        EmuState& emu = Emu();
        if (!buffer || size <= 0 || emu.userDataDir.empty())
            return false;
        std::snprintf(buffer, static_cast<size_t>(size), "%s", emu.userDataDir.c_str());
        return true;
    }
    void StartVoiceRecording() override {}
    void StopVoiceRecording() override {}
    EVoiceResult GetAvailableVoice(uint32_t* compressed, uint32_t* uncompressed, uint32_t) override {
        if (compressed)
            *compressed = 0;
        if (uncompressed)
            *uncompressed = 0;
        return k_EVoiceResultNotRecording;
    }
    EVoiceResult GetVoice(bool, void*, uint32_t, uint32_t* written, bool, void*, uint32_t, uint32_t*,
                          uint32_t) override {
        if (written)
            *written = 0;
        return k_EVoiceResultNotRecording;
    }
    EVoiceResult DecompressVoice(const void*, uint32_t, void*, uint32_t, uint32_t*, uint32_t) override {
        return k_EVoiceResultNotRecording;
    }
    uint32_t GetVoiceOptimalSampleRate() override { return 48000; }
    HAuthTicket GetAuthSessionTicket(void* ticket, int size, uint32_t* ticketSize) override {
        EmuState& emu = Emu();
        uint32_t length = static_cast<uint32_t>(size > 0 && size < 64 ? size : 16);
        uint8_t generated[64] = {};
        MakeTicket(generated, sizeof(generated));
        if (ticket && size > 0)
            std::memcpy(ticket, generated, length);
        if (ticketSize)
            *ticketSize = length;
        const HAuthTicket handle = emu.nextAuthTicket++;
        emu.authTickets[handle] = emu.config.steamId;
        GetAuthSessionTicketResponse_t response{};
        response.m_hAuthTicket = handle;
        response.m_eResult = k_EResultOK;
        emu.QueueCallback(k_iCallback_GetAuthSessionTicketResponse, &response, sizeof(response));
        return handle;
    }
    EBeginAuthSessionResult BeginAuthSession(const void*, int, CSteamID) override {
        return k_EBeginAuthSessionResultOK;
    }
    void EndAuthSession(CSteamID) override {}
    void CancelAuthTicket(HAuthTicket ticket) override { Emu().authTickets.erase(ticket); }
    EUserHasLicenseForAppResult UserHasLicenseForApp(CSteamID, AppId_t appId) override {
        EmuState& emu = Emu();
        if (appId == emu.config.appId)
            return k_EUserHasLicenseResultHasLicense;
        for (const AppId_t dlc : emu.config.dlc) {
            if (dlc == appId)
                return k_EUserHasLicenseResultHasLicense;
        }
        return k_EUserHasLicenseResultDoesNotHaveLicense;
    }
    bool BIsBehindNAT() override { return false; }
    void AdvertiseGame(CSteamID, uint32_t, uint16_t) override {}
    // El ticket cifrado es local: ningún servicio externo (p. ej. EOS) lo dará por válido.
    SteamAPICall_t RequestEncryptedAppTicket(void*, int) override {
        EncryptedAppTicketResponse_t response{};
        response.m_eResult = k_EResultOK;
        return Emu().QueueCallResult(k_iCallback_EncryptedAppTicketResponse, &response, sizeof(response));
    }
    bool GetEncryptedAppTicket(void* ticket, int size, uint32_t* ticketSize) override {
        constexpr uint32_t length = 128;
        if (ticketSize)
            *ticketSize = length;
        if (!ticket || size < static_cast<int>(length))
            return false;
        MakeTicket(static_cast<uint8_t*>(ticket), length);
        return true;
    }
    int GetGameBadgeLevel(int, bool) override { return 1; }
    int GetPlayerSteamLevel() override { return 1; }
    SteamAPICall_t RequestStoreAuthURL(const char*) override { return 0; }
    bool BIsPhoneVerified() override { return false; }
    bool BIsTwoFactorEnabled() override { return false; }
    bool BIsPhoneIdentifying() override { return false; }
    bool BIsPhoneRequiringVerification() override { return false; }
    int GetMarketEligibility() override { return 0; }
    // SDK: llamada asíncrona; el resultado (DurationControl_t, callback 167) llega como call result.
    SteamAPICall_t GetDurationControl() override {
        struct DurationControl {
            EResult m_eResult;
            AppId_t m_appid;
            bool m_bApplicable;
            int32_t m_csecsLast5h;
            int m_progress;
            int m_notification;
            int32_t m_csecsToday;
            int32_t m_csecsRemaining;
        };
        DurationControl result{};
        result.m_eResult = k_EResultOK;
        result.m_appid = Emu().config.appId;
        return Emu().QueueCallResult(167, &result, sizeof(result));
    }
    bool BSetDurationControlOnlineState(int) override { return false; }
    HAuthTicket GetAuthTicketForWebApi(const char*) override {
        EmuState& emu = Emu();
        const HAuthTicket handle = emu.nextAuthTicket++;
        emu.authTickets[handle] = emu.config.steamId;
        GetTicketForWebApiResponse_t response{};
        response.m_hAuthTicket = handle;
        response.m_eResult = k_EResultOK;
        response.m_cubTicket = 64;
        MakeTicket(response.m_rgubTicket, static_cast<size_t>(response.m_cubTicket));
        emu.QueueCallback(k_iCallback_GetTicketForWebApiResponse, &response, sizeof(response));
        return handle;
    }
};

class SteamFriendsImpl final : public ISteamFriends {
public:
    const char* GetPersonaName() override { return Emu().config.personaName.c_str(); }
    SteamAPICall_t SetPersonaName(const char* name) override {
        EmuState& emu = Emu();
        if (name && *name)
            emu.config.personaName = name;
        return 0;
    }
    EPersonaState GetPersonaState() override { return k_EPersonaStateOnline; }
    int GetFriendCount(int) override { return 0; }
    CSteamID GetFriendByIndex(int, int) override { return k_steamIDNil; }
    EFriendRelationship GetFriendRelationship(CSteamID) override { return k_EFriendRelationshipNone; }
    EPersonaState GetFriendPersonaState(CSteamID) override { return k_EPersonaStateOffline; }
    const char* GetFriendPersonaName(CSteamID) override { return ""; }
    bool GetFriendGamePlayed(CSteamID, FriendGameInfo_t*) override { return false; }
    const char* GetFriendPersonaNameHistory(CSteamID, int) override { return ""; }
    int GetFriendSteamLevel(CSteamID) override { return 0; }
    const char* GetPlayerNickname(CSteamID) override { return GetPersonaName(); }
    int GetFriendsGroupCount() override { return 0; }
    FriendsGroupID_t GetFriendsGroupIDByIndex(int) override { return -1; }
    const char* GetFriendsGroupName(FriendsGroupID_t) override { return ""; }
    int GetFriendsGroupMembersCount(FriendsGroupID_t) override { return 0; }
    void GetFriendsGroupMembersList(FriendsGroupID_t, CSteamID*, int) override {}
    bool HasFriend(CSteamID, int) override { return false; }
    int GetClanCount() override { return 0; }
    CSteamID GetClanByIndex(int) override { return k_steamIDNil; }
    const char* GetClanName(CSteamID) override { return ""; }
    const char* GetClanTag(CSteamID) override { return ""; }
    bool GetClanActivityCounts(CSteamID, int* online, int* inGame, int* chatting) override {
        if (online)
            *online = 0;
        if (inGame)
            *inGame = 0;
        if (chatting)
            *chatting = 0;
        return false;
    }
    SteamAPICall_t DownloadClanActivityCounts(CSteamID*, int) override { return 0; }
    int GetFriendCountFromSource(CSteamID) override { return 0; }
    CSteamID GetFriendFromSourceByIndex(CSteamID, int) override { return k_steamIDNil; }
    bool IsUserInSource(CSteamID, CSteamID) override { return false; }
    void SetInGameVoiceSpeaking(CSteamID, bool) override {}
    void ActivateGameOverlay(const char*) override {}
    void ActivateGameOverlayToUser(const char*, CSteamID) override {}
    void ActivateGameOverlayToWebPage(const char*, int) override {}
    void ActivateGameOverlayToStore(AppId_t, int) override {}
    void SetPlayedWith(CSteamID) override {}
    void ActivateGameOverlayInviteDialog(CSteamID) override {}
    int GetSmallFriendAvatar(CSteamID) override { return 0; }
    int GetMediumFriendAvatar(CSteamID) override { return 0; }
    int GetLargeFriendAvatar(CSteamID) override { return 0; }
    bool RequestUserInformation(CSteamID steamId, bool) override {
        EmuState& emu = Emu();
        PersonaStateChange_t change{};
        change.m_ulSteamID = steamId;
        change.m_nChangeFlags = 1;
        emu.QueueCallback(k_iCallback_PersonaStateChange, &change, sizeof(change));
        return true;
    }
    SteamAPICall_t RequestClanOfficerList(CSteamID) override { return 0; }
    CSteamID GetClanOwner(CSteamID) override { return k_steamIDNil; }
    int GetClanOfficerCount(CSteamID) override { return 0; }
    CSteamID GetClanOfficerByIndex(CSteamID, int) override { return k_steamIDNil; }
    uint32_t GetUserRestrictions() override { return 0; }
    bool SetRichPresence(const char* key, const char* value) override {
        EmuState& emu = Emu();
        if (!key || !value)
            return false;
        emu.richPresence[key] = value;
        return true;
    }
    void ClearRichPresence() override { Emu().richPresence.clear(); }
    const char* GetFriendRichPresence(CSteamID steamId, const char* key) override {
        EmuState& emu = Emu();
        if (!key || steamId != emu.config.steamId)
            return "";
        const auto it = emu.richPresence.find(key);
        return it == emu.richPresence.end() ? "" : it->second.c_str();
    }
    int GetFriendRichPresenceKeyCount(CSteamID steamId) override {
        return steamId == Emu().config.steamId ? static_cast<int>(Emu().richPresence.size()) : 0;
    }
    const char* GetFriendRichPresenceKeyByIndex(CSteamID steamId, int index) override {
        EmuState& emu = Emu();
        if (steamId != emu.config.steamId || index < 0)
            return "";
        int i = 0;
        for (const auto& [key, value] : emu.richPresence) {
            (void)value;
            if (i++ == index)
                return key.c_str();
        }
        return "";
    }
    void RequestFriendRichPresence(CSteamID) override {}
    bool InviteUserToGame(CSteamID, const char*) override { return false; }
    int GetCoplayFriendCount() override { return 0; }
    CSteamID GetCoplayFriend(int) override { return k_steamIDNil; }
    int GetFriendCoplayTime(CSteamID) override { return 0; }
    AppId_t GetFriendCoplayGame(CSteamID) override { return 0; }
    SteamAPICall_t JoinClanChatRoom(CSteamID) override { return 0; }
    bool LeaveClanChatRoom(CSteamID) override { return false; }
    int GetClanChatMemberCount(CSteamID) override { return 0; }
    CSteamID GetChatMemberByIndex(CSteamID, int) override { return k_steamIDNil; }
    bool SendClanChatMessage(CSteamID, const char*) override { return false; }
    int GetClanChatMessage(CSteamID, int, void*, int, int*, CSteamID*) override { return 0; }
    bool IsClanChatAdmin(CSteamID, CSteamID) override { return false; }
    bool IsClanChatWindowOpenInSteam(CSteamID) override { return false; }
    bool OpenClanChatWindowInSteam(CSteamID) override { return false; }
    bool CloseClanChatWindowInSteam(CSteamID) override { return false; }
    bool SetListenForFriendsMessages(bool) override { return false; }
    bool ReplyToFriendMessage(CSteamID, const char*) override { return false; }
    int GetFriendMessage(CSteamID, int, void*, int, int*) override { return 0; }
    SteamAPICall_t GetFollowerCount(CSteamID) override { return 0; }
    SteamAPICall_t IsFollowing(CSteamID) override { return 0; }
    SteamAPICall_t EnumerateFollowingList(uint32_t) override { return 0; }
    bool IsClanPublic(CSteamID) override { return false; }
    bool IsClanOfficialGameGroup(CSteamID) override { return false; }
    int GetNumChatsWithUnreadPriorityMessages() override { return 0; }
    void ActivateGameOverlayRemotePlayTogetherInviteDialog(CSteamID) override {}
};

class SteamUtilsImpl final : public ISteamUtils {
public:
    uint32_t GetSecondsSinceAppActive() override {
        return static_cast<uint32_t>((GetTickCount64() - Emu().initTicks) / 1000);
    }
    uint32_t GetSecondsSinceComputerActive() override {
        return static_cast<uint32_t>(GetTickCount64() / 1000);
    }
    EUniverse GetConnectedUniverse() override { return k_EUniversePublic; }
    uint32_t GetServerRealTime() override { return static_cast<uint32_t>(time(nullptr)); }
    const char* GetIPCountry() override { return Emu().config.ipCountry.c_str(); }
    bool GetImageSize(int, uint32_t* width, uint32_t* height) override {
        if (width)
            *width = 0;
        if (height)
            *height = 0;
        return false;
    }
    bool GetImageRGBA(int, uint8_t*, int) override { return false; }
    bool GetCSERIPPort(uint32_t* ip, uint16_t* port) override {
        if (ip)
            *ip = 0;
        if (port)
            *port = 0;
        return false;
    }
    uint8_t GetCurrentBatteryPower() override { return 255; }
    AppId_t GetAppID() override { return Emu().config.appId; }
    void SetOverlayNotificationPosition(int) override {}
    bool IsAPICallCompleted(SteamAPICall_t call, bool* failed) override {
        EmuState& emu = Emu();
        if (failed)
            *failed = false;
        return emu.apiResults.count(call) > 0;
    }
    int GetAPICallFailureReason(SteamAPICall_t call) override {
        return Emu().apiResults.count(call) > 0 ? k_ESteamAPICallFailureNone
                                                : k_ESteamAPICallFailureInvalidHandle;
    }
    bool GetAPICallResult(SteamAPICall_t call, void* out, int size, int expected, bool* failed) override {
        return Emu().GetApiResult(call, out, static_cast<size_t>(size > 0 ? size : 0), expected, failed);
    }
    uint32_t GetIPCCallCount() override { return ++Emu().ipcCallCount; }
    void SetWarningMessageHook(void* hook) override { Emu().warningHook = hook; }
    bool IsOverlayEnabled() override { return false; }
    bool BOverlayNeedsPresent() override { return false; }
    SteamAPICall_t CheckFileSignature(const char*) override { return 0; }
    bool ShowGamepadTextInput(int, int, const char*, uint32_t, const char*) override { return false; }
    uint32_t GetEnteredGamepadTextLength() override { return 0; }
    bool GetEnteredGamepadTextInput(char*, uint32_t) override { return false; }
    const char* GetSteamUILanguage() override { return Emu().config.language.c_str(); }
    bool IsSteamRunningInVR() override { return false; }
    void SetOverlayNotificationInset(int, int) override {}
    bool IsSteamInBigPictureMode() override { return false; }
    void StartVrDashboard() override {}
    bool IsVRHeadsetStreamingEnabled() override { return false; }
    void SetVRHeadsetStreamingEnabled(bool) override {}
    bool IsSteamChinaLauncher() override { return false; }
    bool InitFilterText(uint32_t) override { return true; }
    int FilterText(int, CSteamID, const char* input, char* output, uint32_t size) override {
        if (!input || !output || size == 0)
            return 0;
        std::strncpy(output, input, size - 1);
        output[size - 1] = 0;
        return 0;
    }
    int GetIPv6ConnectivityState(int) override { return 0; }
    bool IsSteamRunningOnSteamDeck() override { return false; }
};

class SteamAppsImpl final : public ISteamApps {
public:
    bool BIsSubscribed() override { return true; }
    bool BIsLowViolence() override { return false; }
    bool BIsCybercafe() override { return false; }
    bool BIsVACBanned() override { return false; }
    const char* GetCurrentGameLanguage() override { return Emu().config.language.c_str(); }
    const char* GetAvailableGameLanguages() override { return "english"; }
    bool BIsSubscribedApp(AppId_t appId) override {
        EmuState& emu = Emu();
        if (appId == emu.config.appId)
            return true;
        for (const AppId_t dlc : emu.config.dlc) {
            if (dlc == appId)
                return true;
        }
        return false;
    }
    bool BIsDlcInstalled(AppId_t appId) override {
        for (const AppId_t dlc : Emu().config.dlc) {
            if (dlc == appId)
                return true;
        }
        return false;
    }
    uint32_t GetEarliestPurchaseUnixTime(AppId_t) override { return 0; }
    bool BIsSubscribedFromFreeWeekend() override { return false; }
    int GetDLCCount() override { return static_cast<int>(Emu().config.dlc.size()); }
    bool BGetDLCDataByIndex(int index, AppId_t* appId, bool* available, char* name, int nameSize) override {
        EmuState& emu = Emu();
        if (index < 0 || index >= static_cast<int>(emu.config.dlc.size()))
            return false;
        if (appId)
            *appId = emu.config.dlc[static_cast<size_t>(index)];
        if (available)
            *available = true;
        if (name && nameSize > 0)
            name[0] = 0;
        return true;
    }
    void InstallDLC(AppId_t) override {}
    void UninstallDLC(AppId_t) override {}
    void RequestAppProofOfPurchaseKey(AppId_t) override {}
    bool GetCurrentBetaName(char*, int) override { return false; }
    bool MarkContentCorrupt(bool) override { return false; }
    uint32_t GetInstalledDepots(AppId_t, DepotId_t*, uint32_t) override { return 0; }
    uint32_t GetAppInstallDir(AppId_t, char*, uint32_t) override { return 0; }
    bool BIsAppInstalled(AppId_t appId) override { return appId == Emu().config.appId; }
    CSteamID GetAppOwner() override { return Emu().config.steamId; }
    const char* GetLaunchQueryParam(const char*) override { return ""; }
    bool GetDlcDownloadProgress(AppId_t, uint64_t* downloaded, uint64_t* total) override {
        if (downloaded)
            *downloaded = 0;
        if (total)
            *total = 0;
        return true;
    }
    int GetAppBuildId() override { return 0; }
    void RequestAllProofOfPurchaseKeys() override {}
    SteamAPICall_t GetFileDetails(const char*) override { return 0; }
    int GetLaunchCommandLine(char*, int) override { return 0; }
    bool BIsSubscribedFromFamilySharing() override { return false; }
    bool BIsTimedTrial(uint32_t*, uint32_t*) override { return false; }
    bool SetDlcContext(AppId_t) override { return false; }
    int GetNumBetas(int* available, int* privateBetas) override {
        if (available)
            *available = 0;
        if (privateBetas)
            *privateBetas = 0;
        return 0;
    }
    bool GetBetaInfo(int, char*, int, char*, int, bool*, char*, int) override { return false; }
    bool SetActiveBeta(const char*, bool) override { return false; }
};

} // namespace

ISteamClient* ClientInstance() {
    static SteamClientImpl impl;
    return &impl;
}

ISteamUser* UserInstance() {
    static SteamUserImpl impl;
    return &impl;
}

ISteamFriends* FriendsInstance() {
    static SteamFriendsImpl impl;
    return &impl;
}

ISteamUtils* UtilsInstance() {
    static SteamUtilsImpl impl;
    return &impl;
}

ISteamApps* AppsInstance() {
    static SteamAppsImpl impl;
    return &impl;
}

} // namespace sfemu
