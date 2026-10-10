#pragma once

// Clases de interfaz con el MISMO orden de funciones virtuales que los encabezados públicos
// del Steamworks SDK. El orden de las vtables es el contrato ABI: cualquier cambio rompe los juegos.
// Todas las implementaciones viven en los archivos interfaces_*.cpp.

#include "steamemu_types.h"

namespace sfemu {

constexpr const char* STEAMCLIENT_INTERFACE_VERSION = "SteamClient020";
constexpr const char* STEAMUSER_INTERFACE_VERSION = "SteamUser023";
constexpr const char* STEAMFRIENDS_INTERFACE_VERSION = "SteamFriends017";
constexpr const char* STEAMUTILS_INTERFACE_VERSION = "SteamUtils010";
constexpr const char* STEAMUSERSTATS_INTERFACE_VERSION = "STEAMUSERSTATS_INTERFACE_VERSION012";
constexpr const char* STEAMMATCHMAKING_INTERFACE_VERSION = "SteamMatchMaking009";
constexpr const char* STEAMMATCHMAKINGSERVERS_INTERFACE_VERSION = "SteamMatchMakingServers002";
constexpr const char* STEAMNETWORKING_INTERFACE_VERSION = "SteamNetworking006";
constexpr const char* STEAMREMOTESTORAGE_INTERFACE_VERSION = "STEAMREMOTESTORAGE_INTERFACE_VERSION016";
constexpr const char* STEAMAPPS_INTERFACE_VERSION = "STEAMAPPS_INTERFACE_VERSION008";
constexpr const char* STEAMUGC_INTERFACE_VERSION = "STEAMUGC_INTERFACE_VERSION020";
constexpr const char* STEAMINVENTORY_INTERFACE_VERSION = "STEAMINVENTORY_INTERFACE_V003";
constexpr const char* STEAMHTTP_INTERFACE_VERSION = "STEAMHTTP_INTERFACE_VERSION003";
constexpr const char* STEAMSCREENSHOTS_INTERFACE_VERSION = "STEAMSCREENSHOTS_INTERFACE_VERSION003";
constexpr const char* STEAMCONTROLLER_INTERFACE_VERSION = "SteamController008";
constexpr const char* STEAMINPUT_INTERFACE_VERSION = "SteamInput006";
constexpr const char* STEAMMUSIC_INTERFACE_VERSION = "STEAMMUSIC_INTERFACE_VERSION001";
constexpr const char* STEAMVIDEO_INTERFACE_VERSION = "STEAMVIDEO_INTERFACE_V002";
constexpr const char* STEAMPARENTALSETTINGS_INTERFACE_VERSION = "STEAMPARENTALSETTINGS_INTERFACE_VERSION001";
constexpr const char* STEAMAPPLIST_INTERFACE_VERSION = "STEAMAPPLIST_INTERFACE_VERSION001";
constexpr const char* STEAMGAMESERVER_INTERFACE_VERSION = "SteamGameServer016";
constexpr const char* STEAMGAMESERVERSTATS_INTERFACE_VERSION = "SteamGameServerStats001";

class ISteamClient {
public:
    virtual HSteamPipe CreateSteamPipe() = 0;
    virtual bool BReleaseSteamPipe(HSteamPipe) = 0;
    virtual HSteamUser ConnectToGlobalUser(HSteamPipe) = 0;
    virtual HSteamUser CreateLocalUser(HSteamPipe*, EAccountType) = 0;
    virtual void ReleaseUser(HSteamPipe, HSteamUser) = 0;
    virtual class ISteamUser* GetISteamUser(HSteamUser, HSteamPipe, const char*) = 0;
    virtual class ISteamGameServer* GetISteamGameServer(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void SetLocalIPBinding(const SteamIPAddress_t&, uint16_t) = 0;
    virtual class ISteamFriends* GetISteamFriends(HSteamUser, HSteamPipe, const char*) = 0;
    virtual class ISteamUtils* GetISteamUtils(HSteamPipe, const char*) = 0;
    virtual class ISteamMatchmaking* GetISteamMatchmaking(HSteamUser, HSteamPipe, const char*) = 0;
    virtual class ISteamMatchmakingServers* GetISteamMatchmakingServers(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamGenericInterface(HSteamUser, HSteamPipe, const char*) = 0;
    virtual class ISteamUserStats* GetISteamUserStats(HSteamUser, HSteamPipe, const char*) = 0;
    virtual class ISteamGameServerStats* GetISteamGameServerStats(HSteamUser, HSteamPipe, const char*) = 0;
    virtual class ISteamApps* GetISteamApps(HSteamUser, HSteamPipe, const char*) = 0;
    virtual class ISteamNetworking* GetISteamNetworking(HSteamUser, HSteamPipe, const char*) = 0;
    virtual class ISteamRemoteStorage* GetISteamRemoteStorage(HSteamUser, HSteamPipe, const char*) = 0;
    virtual class ISteamScreenshots* GetISteamScreenshots(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamGameSearch(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void RunFrame() = 0;
    virtual uint32_t GetIPCCallCount() = 0;
    virtual void SetWarningMessageHook(void*) = 0;
    virtual bool BShutdownIfAllPipesClosed() = 0;
    virtual class ISteamHTTP* GetISteamHTTP(HSteamUser, HSteamPipe, const char*) = 0;
    virtual class ISteamController* GetISteamController(HSteamUser, HSteamPipe, const char*) = 0;
    virtual class ISteamUGC* GetISteamUGC(HSteamUser, HSteamPipe, const char*) = 0;
    virtual class ISteamAppList* GetISteamAppList(HSteamUser, HSteamPipe, const char*) = 0;
    virtual class ISteamMusic* GetISteamMusic(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamMusicRemote(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamHTMLSurface(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void Set_SteamAPI_CPostAPIResultInProcess(void*) = 0;
    virtual void Remove_SteamAPI_CPostAPIResultInProcess(void*) = 0;
    virtual void Set_SteamAPI_CCheckCallbackRegisteredInProcess(void*) = 0;
    virtual class ISteamInventory* GetISteamInventory(HSteamUser, HSteamPipe, const char*) = 0;
    virtual class ISteamVideo* GetISteamVideo(HSteamUser, HSteamPipe, const char*) = 0;
    virtual class ISteamParentalSettings* GetISteamParentalSettings(HSteamUser, HSteamPipe, const char*) = 0;
    virtual class ISteamInput* GetISteamInput(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamParties(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamRemotePlay(HSteamUser, HSteamPipe, const char*) = 0;
};

class ISteamUser {
public:
    virtual HSteamUser GetHSteamUser() = 0;
    virtual bool BLoggedOn() = 0;
    virtual CSteamID GetSteamID() = 0;
    virtual int InitiateGameConnection_DEPRECATED(void*, int, CSteamID, uint32_t, uint16_t, bool) = 0;
    virtual void TerminateGameConnection_DEPRECATED(uint32_t, uint16_t) = 0;
    virtual void TrackAppUsageEvent(CGameID, int, const char*) = 0;
    virtual bool GetUserDataFolder(char*, int) = 0;
    virtual void StartVoiceRecording() = 0;
    virtual void StopVoiceRecording() = 0;
    virtual EVoiceResult GetAvailableVoice(uint32_t*, uint32_t*, uint32_t) = 0;
    virtual EVoiceResult GetVoice(bool, void*, uint32_t, uint32_t*, bool, void*, uint32_t, uint32_t*, uint32_t) = 0;
    virtual EVoiceResult DecompressVoice(const void*, uint32_t, void*, uint32_t, uint32_t*, uint32_t) = 0;
    virtual uint32_t GetVoiceOptimalSampleRate() = 0;
    virtual HAuthTicket GetAuthSessionTicket(void*, int, uint32_t*) = 0;
    virtual EBeginAuthSessionResult BeginAuthSession(const void*, int, CSteamID) = 0;
    virtual void EndAuthSession(CSteamID) = 0;
    virtual void CancelAuthTicket(HAuthTicket) = 0;
    virtual EUserHasLicenseForAppResult UserHasLicenseForApp(CSteamID, AppId_t) = 0;
    virtual bool BIsBehindNAT() = 0;
    virtual void AdvertiseGame(CSteamID, uint32_t, uint16_t) = 0;
    virtual SteamAPICall_t RequestEncryptedAppTicket(void*, int) = 0;
    virtual bool GetEncryptedAppTicket(void*, int, uint32_t*) = 0;
    virtual int GetGameBadgeLevel(int, bool) = 0;
    virtual int GetPlayerSteamLevel() = 0;
    virtual SteamAPICall_t RequestStoreAuthURL(const char*) = 0;
    virtual bool BIsPhoneVerified() = 0;
    virtual bool BIsTwoFactorEnabled() = 0;
    virtual bool BIsPhoneIdentifying() = 0;
    virtual bool BIsPhoneRequiringVerification() = 0;
    virtual int GetMarketEligibility() = 0;
    virtual void GetDurationControl(void*) = 0;
    virtual bool BSetDurationControlOnlineState(int) = 0;
    virtual HAuthTicket GetAuthTicketForWebApi(const char*) = 0;
};

class ISteamFriends {
public:
    virtual const char* GetPersonaName() = 0;
    virtual SteamAPICall_t SetPersonaName(const char*) = 0;
    virtual EPersonaState GetPersonaState() = 0;
    virtual int GetFriendCount(int) = 0;
    virtual CSteamID GetFriendByIndex(int, int) = 0;
    virtual EFriendRelationship GetFriendRelationship(CSteamID) = 0;
    virtual EPersonaState GetFriendPersonaState(CSteamID) = 0;
    virtual const char* GetFriendPersonaName(CSteamID) = 0;
    virtual bool GetFriendGamePlayed(CSteamID, FriendGameInfo_t*) = 0;
    virtual const char* GetFriendPersonaNameHistory(CSteamID, int) = 0;
    virtual int GetFriendSteamLevel(CSteamID) = 0;
    virtual const char* GetPlayerNickname(CSteamID) = 0;
    virtual int GetFriendsGroupCount() = 0;
    virtual FriendsGroupID_t GetFriendsGroupIDByIndex(int) = 0;
    virtual const char* GetFriendsGroupName(FriendsGroupID_t) = 0;
    virtual int GetFriendsGroupMembersCount(FriendsGroupID_t) = 0;
    virtual void GetFriendsGroupMembersList(FriendsGroupID_t, CSteamID*, int) = 0;
    virtual bool HasFriend(CSteamID, int) = 0;
    virtual int GetClanCount() = 0;
    virtual CSteamID GetClanByIndex(int) = 0;
    virtual const char* GetClanName(CSteamID) = 0;
    virtual const char* GetClanTag(CSteamID) = 0;
    virtual bool GetClanActivityCounts(CSteamID, int*, int*, int*) = 0;
    virtual SteamAPICall_t DownloadClanActivityCounts(CSteamID*, int) = 0;
    virtual int GetFriendCountFromSource(CSteamID) = 0;
    virtual CSteamID GetFriendFromSourceByIndex(CSteamID, int) = 0;
    virtual bool IsUserInSource(CSteamID, CSteamID) = 0;
    virtual void SetInGameVoiceSpeaking(CSteamID, bool) = 0;
    virtual void ActivateGameOverlay(const char*) = 0;
    virtual void ActivateGameOverlayToUser(const char*, CSteamID) = 0;
    virtual void ActivateGameOverlayToWebPage(const char*, int) = 0;
    virtual void ActivateGameOverlayToStore(AppId_t, int) = 0;
    virtual void SetPlayedWith(CSteamID) = 0;
    virtual void ActivateGameOverlayInviteDialog(CSteamID) = 0;
    virtual int GetSmallFriendAvatar(CSteamID) = 0;
    virtual int GetMediumFriendAvatar(CSteamID) = 0;
    virtual int GetLargeFriendAvatar(CSteamID) = 0;
    virtual bool RequestUserInformation(CSteamID, bool) = 0;
    virtual SteamAPICall_t RequestClanOfficerList(CSteamID) = 0;
    virtual CSteamID GetClanOwner(CSteamID) = 0;
    virtual int GetClanOfficerCount(CSteamID) = 0;
    virtual CSteamID GetClanOfficerByIndex(CSteamID, int) = 0;
    virtual uint32_t GetUserRestrictions() = 0;
    virtual bool SetRichPresence(const char*, const char*) = 0;
    virtual void ClearRichPresence() = 0;
    virtual const char* GetFriendRichPresence(CSteamID, const char*) = 0;
    virtual int GetFriendRichPresenceKeyCount(CSteamID) = 0;
    virtual const char* GetFriendRichPresenceKeyByIndex(CSteamID, int) = 0;
    virtual void RequestFriendRichPresence(CSteamID) = 0;
    virtual bool InviteUserToGame(CSteamID, const char*) = 0;
    virtual int GetCoplayFriendCount() = 0;
    virtual CSteamID GetCoplayFriend(int) = 0;
    virtual int GetFriendCoplayTime(CSteamID) = 0;
    virtual AppId_t GetFriendCoplayGame(CSteamID) = 0;
    virtual SteamAPICall_t JoinClanChatRoom(CSteamID) = 0;
    virtual bool LeaveClanChatRoom(CSteamID) = 0;
    virtual int GetClanChatMemberCount(CSteamID) = 0;
    virtual CSteamID GetChatMemberByIndex(CSteamID, int) = 0;
    virtual bool SendClanChatMessage(CSteamID, const char*) = 0;
    virtual int GetClanChatMessage(CSteamID, int, void*, int, int*, CSteamID*) = 0;
    virtual bool IsClanChatAdmin(CSteamID, CSteamID) = 0;
    virtual bool IsClanChatWindowOpenInSteam(CSteamID) = 0;
    virtual bool OpenClanChatWindowInSteam(CSteamID) = 0;
    virtual bool CloseClanChatWindowInSteam(CSteamID) = 0;
    virtual bool SetListenForFriendsMessages(bool) = 0;
    virtual bool ReplyToFriendMessage(CSteamID, const char*) = 0;
    virtual int GetFriendMessage(CSteamID, int, void*, int, int*) = 0;
    virtual SteamAPICall_t GetFollowerCount(CSteamID) = 0;
    virtual SteamAPICall_t IsFollowing(CSteamID) = 0;
    virtual SteamAPICall_t EnumerateFollowingList(uint32_t) = 0;
    virtual bool IsClanPublic(CSteamID) = 0;
    virtual bool IsClanOfficialGameGroup(CSteamID) = 0;
    virtual int GetNumChatsWithUnreadPriorityMessages() = 0;
    virtual void ActivateGameOverlayRemotePlayTogetherInviteDialog(CSteamID) = 0;
};

class ISteamUtils {
public:
    virtual uint32_t GetSecondsSinceAppActive() = 0;
    virtual uint32_t GetSecondsSinceComputerActive() = 0;
    virtual EUniverse GetConnectedUniverse() = 0;
    virtual uint32_t GetServerRealTime() = 0;
    virtual const char* GetIPCountry() = 0;
    virtual bool GetImageSize(int, uint32_t*, uint32_t*) = 0;
    virtual bool GetImageRGBA(int, uint8_t*, int) = 0;
    virtual bool GetCSERIPPort(uint32_t*, uint16_t*) = 0;
    virtual uint8_t GetCurrentBatteryPower() = 0;
    virtual AppId_t GetAppID() = 0;
    virtual void SetOverlayNotificationPosition(int) = 0;
    virtual bool IsAPICallCompleted(SteamAPICall_t, bool*) = 0;
    virtual int GetAPICallFailureReason(SteamAPICall_t) = 0;
    virtual bool GetAPICallResult(SteamAPICall_t, void*, int, int, bool*) = 0;
    virtual uint32_t GetIPCCallCount() = 0;
    virtual void SetWarningMessageHook(void*) = 0;
    virtual bool IsOverlayEnabled() = 0;
    virtual bool BOverlayNeedsPresent() = 0;
    virtual SteamAPICall_t CheckFileSignature(const char*) = 0;
    virtual bool ShowGamepadTextInput(int, int, const char*, uint32_t, const char*) = 0;
    virtual uint32_t GetEnteredGamepadTextLength() = 0;
    virtual bool GetEnteredGamepadTextInput(char*, uint32_t) = 0;
    virtual const char* GetSteamUILanguage() = 0;
    virtual bool IsSteamRunningInVR() = 0;
    virtual void SetOverlayNotificationInset(int, int) = 0;
    virtual bool IsSteamInBigPictureMode() = 0;
    virtual void StartVrDashboard() = 0;
    virtual bool IsVRHeadsetStreamingEnabled() = 0;
    virtual void SetVRHeadsetStreamingEnabled(bool) = 0;
    virtual bool IsSteamChinaLauncher() = 0;
    virtual bool InitFilterText(uint32_t) = 0;
    virtual int FilterText(int, CSteamID, const char*, char*, uint32_t) = 0;
    virtual int GetIPv6ConnectivityState(int) = 0;
    virtual bool IsSteamRunningOnSteamDeck() = 0;
};

class ISteamUserStats {
public:
    virtual bool RequestCurrentStats() = 0;
    virtual bool GetStat(const char*, int32_t*) = 0;
    virtual bool GetStat(const char*, float*) = 0;
    virtual bool SetStat(const char*, int32_t) = 0;
    virtual bool SetStat(const char*, float) = 0;
    virtual bool UpdateAvgRateStat(const char*, float, double) = 0;
    virtual bool GetAchievement(const char*, bool*) = 0;
    virtual bool SetAchievement(const char*) = 0;
    virtual bool ClearAchievement(const char*) = 0;
    virtual bool GetAchievementAndUnlockTime(const char*, bool*, uint32_t*) = 0;
    virtual bool StoreStats() = 0;
    virtual int GetAchievementIcon(const char*) = 0;
    virtual const char* GetAchievementDisplayAttribute(const char*, const char*) = 0;
    virtual bool IndicateAchievementProgress(const char*, uint32_t, uint32_t) = 0;
    virtual uint32_t GetNumAchievements() = 0;
    virtual const char* GetAchievementName(uint32_t) = 0;
    virtual SteamAPICall_t RequestUserStats(CSteamID) = 0;
    virtual bool GetUserStat(CSteamID, const char*, int32_t*) = 0;
    virtual bool GetUserStat(CSteamID, const char*, float*) = 0;
    virtual bool GetUserAchievement(CSteamID, const char*, bool*) = 0;
    virtual bool GetUserAchievementAndUnlockTime(CSteamID, const char*, bool*, uint32_t*) = 0;
    virtual bool ResetAllStats(bool) = 0;
    virtual SteamAPICall_t FindOrCreateLeaderboard(const char*, int, int) = 0;
    virtual SteamAPICall_t FindLeaderboard(const char*) = 0;
    virtual const char* GetLeaderboardName(SteamLeaderboard_t) = 0;
    virtual int GetLeaderboardEntryCount(SteamLeaderboard_t) = 0;
    virtual int GetLeaderboardSortMethod(SteamLeaderboard_t) = 0;
    virtual int GetLeaderboardDisplayType(SteamLeaderboard_t) = 0;
    virtual SteamAPICall_t DownloadLeaderboardEntries(SteamLeaderboard_t, int, int, int) = 0;
    virtual SteamAPICall_t DownloadLeaderboardEntriesForUsers(SteamLeaderboard_t, CSteamID*, int) = 0;
    virtual bool GetDownloadedLeaderboardEntry(SteamLeaderboardEntries_t, int, LeaderboardEntry_t*, int32_t*, int) = 0;
    virtual SteamAPICall_t UploadLeaderboardScore(SteamLeaderboard_t, int, int32_t, const int32_t*, int) = 0;
    virtual SteamAPICall_t AttachLeaderboardUGC(SteamLeaderboard_t, UGCHandle_t) = 0;
    virtual SteamAPICall_t GetNumberOfCurrentPlayers() = 0;
    virtual SteamAPICall_t RequestGlobalAchievementPercentages() = 0;
    virtual int GetMostAchievedAchievementInfo(char*, uint32_t, float*, bool*) = 0;
    virtual int GetNextMostAchievedAchievementInfo(int, char*, uint32_t, float*, bool*) = 0;
    virtual bool GetAchievementAchievedPercent(const char*, float*) = 0;
    virtual SteamAPICall_t RequestGlobalStats(int) = 0;
    virtual bool GetGlobalStat(const char*, int64_t*) = 0;
    virtual bool GetGlobalStat(const char*, double*) = 0;
    virtual int32_t GetGlobalStatHistory(const char*, int64_t*, uint32_t) = 0;
    virtual int32_t GetGlobalStatHistory(const char*, double*, uint32_t) = 0;
};

class ISteamMatchmaking {
public:
    virtual int GetFavoriteGameCount() = 0;
    virtual bool GetFavoriteGame(int, AppId_t*, uint32_t*, uint16_t*, uint16_t*, uint32_t*, uint32_t*) = 0;
    virtual int AddFavoriteGame(AppId_t, uint32_t, uint16_t, uint16_t, uint32_t, uint32_t) = 0;
    virtual bool RemoveFavoriteGame(AppId_t, uint32_t, uint16_t, uint16_t, uint32_t) = 0;
    virtual SteamAPICall_t RequestLobbyList() = 0;
    virtual void AddRequestLobbyListStringFilter(const char*, const char*, int) = 0;
    virtual void AddRequestLobbyListNumericalFilter(const char*, int, int) = 0;
    virtual void AddRequestLobbyListNearValueFilter(const char*, int) = 0;
    virtual void AddRequestLobbyListFilterSlotsAvailable(int) = 0;
    virtual void AddRequestLobbyListDistanceFilter(int) = 0;
    virtual void AddRequestLobbyListResultCountFilter(int) = 0;
    virtual void AddRequestLobbyListCompatibleMembersFilter(CSteamID) = 0;
    virtual CSteamID GetLobbyByIndex(int) = 0;
    virtual SteamAPICall_t CreateLobby(int, int) = 0;
    virtual SteamAPICall_t JoinLobby(CSteamID) = 0;
    virtual void LeaveLobby(CSteamID) = 0;
    virtual bool InviteUserToLobby(CSteamID, CSteamID) = 0;
    virtual int GetNumLobbyMembers(CSteamID) = 0;
    virtual CSteamID GetLobbyMemberByIndex(CSteamID, int) = 0;
    virtual const char* GetLobbyData(CSteamID, const char*) = 0;
    virtual bool SetLobbyData(CSteamID, const char*, const char*) = 0;
    virtual int GetLobbyDataCount(CSteamID) = 0;
    virtual bool GetLobbyDataByIndex(CSteamID, int, char*, int, char*, int) = 0;
    virtual bool DeleteLobbyData(CSteamID, const char*) = 0;
    virtual const char* GetLobbyMemberData(CSteamID, CSteamID, const char*) = 0;
    virtual void SetLobbyMemberData(CSteamID, const char*, const char*) = 0;
    virtual bool SendLobbyChatMsg(CSteamID, const void*, int) = 0;
    virtual int GetLobbyChatEntry(CSteamID, int, CSteamID*, void*, int, int*) = 0;
    virtual bool RequestLobbyData(CSteamID) = 0;
    virtual void SetLobbyGameServer(CSteamID, uint32_t, uint16_t, CSteamID) = 0;
    virtual bool GetLobbyGameServer(CSteamID, uint32_t*, uint16_t*, CSteamID*) = 0;
    virtual bool SetLobbyMemberLimit(CSteamID, int) = 0;
    virtual int GetLobbyMemberLimit(CSteamID) = 0;
    virtual bool SetLobbyType(CSteamID, int) = 0;
    virtual bool SetLobbyJoinable(CSteamID, bool) = 0;
    virtual CSteamID GetLobbyOwner(CSteamID) = 0;
    virtual bool SetLobbyOwner(CSteamID, CSteamID) = 0;
    virtual bool SetLinkedLobby(CSteamID, CSteamID) = 0;
};

class ISteamMatchmakingServerListResponse {
public:
    virtual void ServerResponded(HServerListRequest, int) = 0;
    virtual void ServerFailedToRespond(HServerListRequest, int) = 0;
    virtual void RefreshComplete(HServerListRequest, int) = 0;
};

class ISteamMatchmakingServers {
public:
    virtual HServerListRequest RequestInternetServerList(AppId_t, MatchMakingKeyValuePair_t**, uint32_t, ISteamMatchmakingServerListResponse*) = 0;
    virtual HServerListRequest RequestLANServerList(AppId_t, ISteamMatchmakingServerListResponse*) = 0;
    virtual HServerListRequest RequestFriendsServerList(AppId_t, MatchMakingKeyValuePair_t**, uint32_t, ISteamMatchmakingServerListResponse*) = 0;
    virtual HServerListRequest RequestFavoritesServerList(AppId_t, MatchMakingKeyValuePair_t**, uint32_t, ISteamMatchmakingServerListResponse*) = 0;
    virtual HServerListRequest RequestHistoryServerList(AppId_t, MatchMakingKeyValuePair_t**, uint32_t, ISteamMatchmakingServerListResponse*) = 0;
    virtual HServerListRequest RequestSpectatorServerList(AppId_t, MatchMakingKeyValuePair_t**, uint32_t, ISteamMatchmakingServerListResponse*) = 0;
    virtual void ReleaseRequest(HServerListRequest) = 0;
    virtual gameserveritem_t* GetServerDetails(HServerListRequest, int) = 0;
    virtual void CancelQuery(HServerListRequest) = 0;
    virtual void RefreshQuery(HServerListRequest) = 0;
    virtual bool IsRefreshing(HServerListRequest) = 0;
    virtual int GetServerCount(HServerListRequest) = 0;
    virtual void RefreshServer(HServerListRequest, int) = 0;
    virtual HServerQuery PingServer(uint32_t, uint16_t, ISteamMatchmakingServerListResponse*) = 0;
    virtual HServerQuery PlayerDetails(uint32_t, uint16_t, ISteamMatchmakingServerListResponse*) = 0;
    virtual HServerQuery ServerRules(uint32_t, uint16_t, ISteamMatchmakingServerListResponse*) = 0;
    virtual void CancelServerQuery(HServerQuery) = 0;
};

class ISteamNetworking {
public:
    virtual bool SendP2PPacket(CSteamID, const void*, uint32_t, int, int) = 0;
    virtual bool IsP2PPacketAvailable(uint32_t*, int) = 0;
    virtual bool ReadP2PPacket(void*, uint32_t, uint32_t*, CSteamID*, int) = 0;
    virtual bool AcceptP2PSessionWithUser(CSteamID) = 0;
    virtual bool CloseP2PSessionWithUser(CSteamID) = 0;
    virtual bool CloseP2PChannelWithUser(CSteamID, int) = 0;
    virtual bool GetP2PSessionState(CSteamID, P2PSessionState_t*) = 0;
    virtual bool AllowP2PPacketRelay(bool) = 0;
    virtual SNetListenSocket_t CreateListenSocket(int, SteamIPAddress_t, uint16_t, bool) = 0;
    virtual SNetSocket_t CreateP2PConnectionSocket(CSteamID, int, int, bool) = 0;
    virtual SNetSocket_t CreateConnectionSocket(SteamIPAddress_t, uint16_t, int) = 0;
    virtual bool DestroySocket(SNetSocket_t, bool) = 0;
    virtual bool DestroyListenSocket(SNetListenSocket_t, bool) = 0;
    virtual bool SendDataOnSocket(SNetSocket_t, void*, uint32_t, bool) = 0;
    virtual bool IsDataAvailableOnSocket(SNetSocket_t, uint32_t*, bool) = 0;
    virtual bool RetrieveDataFromSocket(SNetSocket_t, void*, uint32_t, uint32_t*) = 0;
    virtual bool IsDataAvailable(SNetListenSocket_t, uint32_t*, bool) = 0;
    virtual bool RetrieveData(SNetListenSocket_t, void*, uint32_t, uint32_t*) = 0;
    virtual bool GetSocketInfo(SNetSocket_t, CSteamID*, int*, SteamIPAddress_t*, uint16_t*) = 0;
    virtual bool GetListenSocketInfo(SNetListenSocket_t, SteamIPAddress_t*, uint16_t*) = 0;
    virtual int GetSocketConnectionType(SNetSocket_t) = 0;
    virtual int GetMaxPacketSize(SNetSocket_t) = 0;
};

class ISteamRemoteStorage {
public:
    virtual bool FileWrite(const char*, const void*, int32_t) = 0;
    virtual int32_t FileRead(const char*, void*, int32_t) = 0;
    virtual SteamAPICall_t FileWriteAsync(const char*, const void*, uint32_t) = 0;
    virtual SteamAPICall_t FileReadAsync(const char*, uint32_t, uint32_t) = 0;
    virtual bool FileReadAsyncComplete(SteamAPICall_t, void*, uint32_t) = 0;
    virtual bool FileForget(const char*) = 0;
    virtual bool FileDelete(const char*) = 0;
    virtual SteamAPICall_t FileShare(const char*) = 0;
    virtual bool SetSyncPlatforms(const char*, uint32_t) = 0;
    virtual UGCFileWriteStreamHandle_t FileWriteStreamOpen(const char*) = 0;
    virtual bool FileWriteStreamWriteChunk(UGCFileWriteStreamHandle_t, const void*, int32_t) = 0;
    virtual bool FileWriteStreamClose(UGCFileWriteStreamHandle_t) = 0;
    virtual bool FileWriteStreamCancel(UGCFileWriteStreamHandle_t) = 0;
    virtual bool FileExists(const char*) = 0;
    virtual bool FilePersisted(const char*) = 0;
    virtual int32_t GetFileSize(const char*) = 0;
    virtual int64_t GetFileTimestamp(const char*) = 0;
    virtual uint32_t GetSyncPlatforms(const char*) = 0;
    virtual int32_t GetFileCount() = 0;
    virtual bool GetFileNameAndSize(int, char*, int32_t*, uint32_t*) = 0;
    virtual bool GetQuota(uint64_t*, uint64_t*) = 0;
    virtual bool IsCloudEnabledForAccount() = 0;
    virtual bool IsCloudEnabledForApp() = 0;
    virtual void SetCloudEnabledForApp(bool) = 0;
    virtual SteamAPICall_t UGCDownload(UGCHandle_t, uint32_t) = 0;
    virtual SteamAPICall_t UGCDownloadToLocation(UGCHandle_t, const char*, uint32_t) = 0;
    virtual bool GetUGCDownloadProgress(UGCHandle_t, int32_t*, int32_t*) = 0;
    virtual bool GetUGCDetails(UGCHandle_t, AppId_t*, char**, int32_t*, CSteamID*) = 0;
    virtual int32_t UGCRead(UGCHandle_t, void*, int32_t, uint32_t, int) = 0;
    virtual int32_t GetCachedUGCCount() = 0;
    virtual UGCHandle_t GetCachedUGCHandle(int32_t) = 0;
    virtual SteamAPICall_t PublishWorkshopFile(const char*, const char*, AppId_t, const char*, const char*, int, SteamParamStringArray_t*, int) = 0;
    virtual PublishedFileUpdateHandle_t CreatePublishedFileUpdateRequest(PublishedFileId_t) = 0;
    virtual bool UpdatePublishedFileFile(PublishedFileUpdateHandle_t, const char*) = 0;
    virtual bool UpdatePublishedFilePreviewFile(PublishedFileUpdateHandle_t, const char*) = 0;
    virtual bool UpdatePublishedFileTitle(PublishedFileUpdateHandle_t, const char*) = 0;
    virtual bool UpdatePublishedFileDescription(PublishedFileUpdateHandle_t, const char*) = 0;
    virtual bool UpdatePublishedFileVisibility(PublishedFileUpdateHandle_t, int) = 0;
    virtual bool UpdatePublishedFileTags(PublishedFileUpdateHandle_t, SteamParamStringArray_t*) = 0;
    virtual SteamAPICall_t CommitPublishedFileUpdate(PublishedFileUpdateHandle_t) = 0;
    virtual SteamAPICall_t GetPublishedFileDetails(PublishedFileId_t, uint32_t) = 0;
    virtual SteamAPICall_t DeletePublishedFile(PublishedFileId_t) = 0;
    virtual SteamAPICall_t EnumerateUserPublishedFiles(uint32_t) = 0;
    virtual SteamAPICall_t SubscribePublishedFile(PublishedFileId_t) = 0;
    virtual SteamAPICall_t EnumerateUserSubscribedFiles(uint32_t) = 0;
    virtual SteamAPICall_t UnsubscribePublishedFile(PublishedFileId_t) = 0;
    virtual bool UpdatePublishedFileSetChangeDescription(PublishedFileUpdateHandle_t, const char*) = 0;
    virtual SteamAPICall_t GetPublishedItemVoteDetails(PublishedFileId_t) = 0;
    virtual SteamAPICall_t UpdateUserPublishedItemVote(PublishedFileId_t, bool) = 0;
    virtual SteamAPICall_t GetUserPublishedItemVoteDetails(PublishedFileId_t) = 0;
    virtual SteamAPICall_t EnumerateUserSharedWorkshopFiles(CSteamID, uint32_t, SteamParamStringArray_t*, SteamParamStringArray_t*) = 0;
    virtual SteamAPICall_t PublishVideo(int, const char*, const char*, const char*, const char*, AppId_t, const char*, const char*, int, SteamParamStringArray_t*) = 0;
    virtual SteamAPICall_t SetUserPublishedFileAction(PublishedFileId_t, int) = 0;
    virtual SteamAPICall_t EnumeratePublishedFilesByUserAction(int, uint32_t) = 0;
    virtual SteamAPICall_t EnumeratePublishedWorkshopFiles(int, uint32_t, uint32_t, uint32_t, SteamParamStringArray_t*, SteamParamStringArray_t*) = 0;
    virtual SteamAPICall_t UGCDownloadToLocationEx(UGCHandle_t, const char*, uint32_t, const char*) = 0;
    virtual int32_t GetLocalFileChangeCount() = 0;
    virtual int32_t BeginFileWriteBatch() = 0;
    virtual bool EndFileWriteBatch() = 0;
};

class ISteamApps {
public:
    virtual bool BIsSubscribed() = 0;
    virtual bool BIsLowViolence() = 0;
    virtual bool BIsCybercafe() = 0;
    virtual bool BIsVACBanned() = 0;
    virtual const char* GetCurrentGameLanguage() = 0;
    virtual const char* GetAvailableGameLanguages() = 0;
    virtual bool BIsSubscribedApp(AppId_t) = 0;
    virtual bool BIsDlcInstalled(AppId_t) = 0;
    virtual uint32_t GetEarliestPurchaseUnixTime(AppId_t) = 0;
    virtual bool BIsSubscribedFromFreeWeekend() = 0;
    virtual int GetDLCCount() = 0;
    virtual bool BGetDLCDataByIndex(int, AppId_t*, bool*, char*, int) = 0;
    virtual void InstallDLC(AppId_t) = 0;
    virtual void UninstallDLC(AppId_t) = 0;
    virtual void RequestAppProofOfPurchaseKey(AppId_t) = 0;
    virtual bool GetCurrentBetaName(char*, int) = 0;
    virtual bool MarkContentCorrupt(bool) = 0;
    virtual uint32_t GetInstalledDepots(AppId_t, DepotId_t*, uint32_t) = 0;
    virtual uint32_t GetAppInstallDir(AppId_t, char*, uint32_t) = 0;
    virtual bool BIsAppInstalled(AppId_t) = 0;
    virtual CSteamID GetAppOwner() = 0;
    virtual const char* GetLaunchQueryParam(const char*) = 0;
    virtual bool GetDlcDownloadProgress(AppId_t, uint64_t*, uint64_t*) = 0;
    virtual int GetAppBuildId() = 0;
    virtual void RequestAllProofOfPurchaseKeys() = 0;
    virtual SteamAPICall_t GetFileDetails(const char*) = 0;
    virtual int GetLaunchCommandLine(char*, int) = 0;
    virtual bool BIsSubscribedFromFamilySharing() = 0;
    virtual bool BIsTimedTrial(uint32_t*, uint32_t*) = 0;
    virtual bool SetDlcContext(AppId_t) = 0;
    virtual int GetNumBetas(int*, int*) = 0;
    virtual bool GetBetaInfo(int, char*, int, char*, int, bool*, char*, int) = 0;
    virtual bool SetActiveBeta(const char*, bool) = 0;
};

class ISteamUGC {
public:
    virtual UGCQueryHandle_t CreateQueryUserUGCRequest(AccountID_t, int, int, int, AppId_t, AppId_t, uint32_t) = 0;
    virtual UGCQueryHandle_t CreateQueryAllUGCRequest(int, int, AppId_t, AppId_t, uint32_t) = 0;
    virtual UGCQueryHandle_t CreateQueryAllUGCRequest(int, int, AppId_t, AppId_t, AppId_t, uint32_t) = 0;
    virtual UGCQueryHandle_t CreateQueryUGCDetailsRequest(PublishedFileId_t*, uint32_t) = 0;
    virtual SteamAPICall_t SendQueryUGCRequest(UGCQueryHandle_t) = 0;
    virtual bool GetQueryUGCResult(UGCQueryHandle_t, uint32_t, SteamUGCDetails_t*) = 0;
    virtual bool GetQueryUGCPreviewURL(UGCQueryHandle_t, uint32_t, char*, uint32_t) = 0;
    virtual bool GetQueryUGCMetadataURL(UGCQueryHandle_t, uint32_t, char*, uint32_t) = 0;
    virtual bool GetQueryUGCChildren(UGCQueryHandle_t, uint32_t, PublishedFileId_t*, uint32_t) = 0;
    virtual bool GetQueryUGCStatistic(UGCQueryHandle_t, uint32_t, int, uint64_t*) = 0;
    virtual uint32_t GetQueryUGCNumAdditionalPreviews(UGCQueryHandle_t, uint32_t) = 0;
    virtual bool GetQueryUGCAdditionalPreview(UGCQueryHandle_t, uint32_t, uint32_t, char*, uint32_t, char*, uint32_t, int*) = 0;
    virtual uint32_t GetQueryUGCNumKeyValueTags(UGCQueryHandle_t, uint32_t) = 0;
    virtual bool GetQueryUGCKeyValueTag(UGCQueryHandle_t, uint32_t, uint32_t, char*, uint32_t, char*, uint32_t) = 0;
    virtual uint32_t GetQueryUGCContentDescriptors(UGCQueryHandle_t, uint32_t, uint32_t*, uint32_t) = 0;
    virtual bool ReleaseQueryUGCRequest(UGCQueryHandle_t) = 0;
    virtual bool AddRequiredTag(UGCQueryHandle_t, const char*) = 0;
    virtual bool AddRequiredTagGroup(UGCQueryHandle_t, const SteamParamStringArray_t*) = 0;
    virtual bool AddExcludedTag(UGCQueryHandle_t, const char*) = 0;
    virtual bool SetReturnOnlyIDs(UGCQueryHandle_t, bool) = 0;
    virtual bool SetReturnKeyValueTags(UGCQueryHandle_t, bool) = 0;
    virtual bool SetReturnLongDescription(UGCQueryHandle_t, bool) = 0;
    virtual bool SetReturnMetadata(UGCQueryHandle_t, bool) = 0;
    virtual bool SetReturnChildren(UGCQueryHandle_t, bool) = 0;
    virtual bool SetReturnAdditionalPreviews(UGCQueryHandle_t, bool) = 0;
    virtual bool SetReturnTotalOnly(UGCQueryHandle_t, bool) = 0;
    virtual bool SetReturnPlaytimeStats(UGCQueryHandle_t, uint32_t) = 0;
    virtual bool SetLanguage(UGCQueryHandle_t, const char*) = 0;
    virtual bool SetAllowCachedResponse(UGCQueryHandle_t, uint32_t) = 0;
    virtual bool SetCloudFileNameFilter(UGCQueryHandle_t, const char*) = 0;
    virtual bool SetMatchAnyTag(UGCQueryHandle_t, bool) = 0;
    virtual bool SetSearchText(UGCQueryHandle_t, const char*) = 0;
    virtual bool SetRankedByTrendDays(UGCQueryHandle_t, uint32_t) = 0;
    virtual bool SetTimeCreatedFileId(UGCQueryHandle_t, PublishedFileId_t) = 0;
    virtual bool SetTimeUpdatedFileId(UGCQueryHandle_t, PublishedFileId_t) = 0;
    virtual bool AddRequiredKeyValueTag(UGCQueryHandle_t, const char*, const char*) = 0;
    virtual SteamAPICall_t RequestUGCDetails(PublishedFileId_t*, uint32_t, uint32_t) = 0;
    virtual SteamAPICall_t CreateItem(AppId_t, int) = 0;
    virtual UGCUpdateHandle_t StartItemUpdate(AppId_t, PublishedFileId_t) = 0;
    virtual bool SetItemTitle(UGCUpdateHandle_t, const char*) = 0;
    virtual bool SetItemDescription(UGCUpdateHandle_t, const char*) = 0;
    virtual bool SetItemUpdateLanguage(UGCUpdateHandle_t, const char*) = 0;
    virtual bool SetItemMetadata(UGCUpdateHandle_t, const char*) = 0;
    virtual bool SetItemVisibility(UGCUpdateHandle_t, int) = 0;
    virtual bool SetItemTags(UGCUpdateHandle_t, const SteamParamStringArray_t*) = 0;
    virtual bool SetItemContent(UGCUpdateHandle_t, const char*) = 0;
    virtual bool SetItemPreview(UGCUpdateHandle_t, const char*) = 0;
    virtual bool SetAllowLegacyUpload(UGCUpdateHandle_t, bool) = 0;
    virtual bool RemoveAllItemKeyValueTags(UGCUpdateHandle_t) = 0;
    virtual bool RemoveItemKeyValueTags(UGCUpdateHandle_t, const char*) = 0;
    virtual bool AddItemKeyValueTag(UGCUpdateHandle_t, const char*, const char*) = 0;
    virtual bool AddItemPreviewFile(UGCUpdateHandle_t, const char*, int) = 0;
    virtual bool AddItemPreviewVideo(UGCUpdateHandle_t, const char*) = 0;
    virtual bool UpdateItemPreviewFile(UGCUpdateHandle_t, uint32_t, const char*) = 0;
    virtual bool UpdateItemPreviewVideo(UGCUpdateHandle_t, uint32_t, const char*) = 0;
    virtual bool RemoveItemPreview(UGCUpdateHandle_t, uint32_t) = 0;
    virtual SteamAPICall_t SubmitItemUpdate(UGCUpdateHandle_t, const char*) = 0;
    virtual int GetItemUpdateProgress(UGCUpdateHandle_t, uint64_t*, uint64_t*) = 0;
    virtual SteamAPICall_t SetUserItemVote(PublishedFileId_t, bool) = 0;
    virtual SteamAPICall_t GetUserItemVote(PublishedFileId_t) = 0;
    virtual SteamAPICall_t AddItemToFavorites(AppId_t, PublishedFileId_t) = 0;
    virtual SteamAPICall_t RemoveItemFromFavorites(AppId_t, PublishedFileId_t) = 0;
    virtual SteamAPICall_t SubscribeItem(PublishedFileId_t) = 0;
    virtual SteamAPICall_t UnsubscribeItem(PublishedFileId_t) = 0;
    virtual uint32_t GetNumSubscribedItems() = 0;
    virtual uint32_t GetSubscribedItems(PublishedFileId_t*, uint32_t) = 0;
    virtual uint32_t GetItemState(PublishedFileId_t) = 0;
    virtual bool GetItemInstallInfo(PublishedFileId_t, uint64_t*, char*, uint32_t, uint32_t*) = 0;
    virtual bool GetItemDownloadInfo(PublishedFileId_t, uint64_t*, uint64_t*) = 0;
    virtual bool DeleteItem(PublishedFileId_t) = 0;
    virtual bool SetItemTaxonomyTags(UGCUpdateHandle_t, const SteamParamStringArray_t*) = 0;
    virtual bool GetItemTaxonomyTags(UGCUpdateHandle_t, uint32_t, uint32_t, char*, uint32_t) = 0;
};

class ISteamInventory {
public:
    virtual EResult GetResultStatus(SteamInventoryResult_t) = 0;
    virtual bool GetResultItems(SteamInventoryResult_t, SteamItemDetails_t*, uint32_t) = 0;
    virtual bool GetResultItemProperty(SteamInventoryResult_t, uint32_t, const char*, char*, uint32_t*) = 0;
    virtual uint32_t GetResultTimestamp(SteamInventoryResult_t) = 0;
    virtual bool CheckResultSteamID(SteamInventoryResult_t, CSteamID) = 0;
    virtual void DestroyResult(SteamInventoryResult_t) = 0;
    virtual bool GetAllItems(SteamInventoryResult_t*) = 0;
    virtual bool GetItemsByID(SteamInventoryResult_t*, const SteamItemInstanceID_t*, uint32_t) = 0;
    virtual bool SerializeResult(SteamInventoryResult_t, void*, uint32_t*) = 0;
    virtual bool DeserializeResult(SteamInventoryResult_t*, const void*, uint32_t, bool) = 0;
    virtual bool GenerateItems(SteamInventoryResult_t*, const SteamItemDef_t*, const uint32_t*, uint32_t) = 0;
    virtual bool GrantPromoItems(SteamInventoryResult_t*) = 0;
    virtual bool AddPromoItem(SteamInventoryResult_t*, SteamItemDef_t) = 0;
    virtual bool AddPromoItems(SteamInventoryResult_t*, const SteamItemDef_t*, uint32_t) = 0;
    virtual bool ConsumeItem(SteamInventoryResult_t*, SteamItemInstanceID_t, uint32_t) = 0;
    virtual bool ExchangeItems(SteamInventoryResult_t*, const SteamItemDef_t*, const uint32_t*, uint32_t, const SteamItemInstanceID_t*, const uint32_t*, uint32_t) = 0;
    virtual bool TransferItemQuantity(SteamInventoryResult_t*, SteamItemInstanceID_t, uint32_t, SteamItemInstanceID_t) = 0;
    virtual void SendItemDropHeartbeat() = 0;
    virtual bool TriggerItemDrop(SteamInventoryResult_t*, SteamItemDef_t) = 0;
    virtual bool TradeItems(SteamInventoryResult_t*, CSteamID, const SteamItemInstanceID_t*, const uint32_t*, uint32_t, const SteamItemInstanceID_t*, const uint32_t*, uint32_t) = 0;
    virtual bool LoadItemDefinitions() = 0;
    virtual bool GetItemDefinitionIDs(SteamItemDef_t*, uint32_t*) = 0;
    virtual bool GetItemDefinitionProperty(SteamItemDef_t, const char*, char*, uint32_t*) = 0;
    virtual SteamAPICall_t RequestEligiblePromoItemDefinitionsIDs(CSteamID) = 0;
    virtual bool GetEligiblePromoItemDefinitionIDs(CSteamID, SteamItemDef_t*, uint32_t*) = 0;
    virtual SteamAPICall_t StartPurchase(const SteamItemDef_t*, const uint32_t*, uint32_t) = 0;
    virtual SteamAPICall_t RequestPrices() = 0;
    virtual uint32_t GetNumItemsWithPrices() = 0;
    virtual bool GetItemsWithPrices(SteamItemDef_t*, uint64_t*, uint64_t*, uint32_t) = 0;
    virtual bool GetItemPrice(SteamItemDef_t, uint64_t*, uint64_t*) = 0;
    virtual SteamInventoryUpdateHandle_t StartUpdateProperties() = 0;
    virtual bool RemoveProperty(SteamInventoryUpdateHandle_t, SteamItemInstanceID_t, const char*) = 0;
    virtual bool SetProperty(SteamInventoryUpdateHandle_t, SteamItemInstanceID_t, const char*, bool) = 0;
    virtual bool SetProperty(SteamInventoryUpdateHandle_t, SteamItemInstanceID_t, const char*, int64_t) = 0;
    virtual bool SetProperty(SteamInventoryUpdateHandle_t, SteamItemInstanceID_t, const char*, float) = 0;
    virtual bool SetProperty(SteamInventoryUpdateHandle_t, SteamItemInstanceID_t, const char*, const char*) = 0;
    virtual bool SubmitUpdateProperties(SteamInventoryUpdateHandle_t, SteamInventoryResult_t*) = 0;
    virtual bool InspectItem(SteamInventoryResult_t*, const char*) = 0;
};

class ISteamHTTP {
public:
    virtual HTTPRequestHandle CreateHTTPRequest(int, const char*) = 0;
    virtual bool SetHTTPRequestContextValue(HTTPRequestHandle, uint64_t) = 0;
    virtual bool SetHTTPRequestNetworkActivityTimeout(HTTPRequestHandle, uint32_t) = 0;
    virtual bool SetHTTPRequestHeaderValue(HTTPRequestHandle, const char*, const char*) = 0;
    virtual bool SetHTTPRequestGetOrPostParameter(HTTPRequestHandle, const char*, const char*) = 0;
    virtual bool SendHTTPRequest(HTTPRequestHandle, SteamAPICall_t*) = 0;
    virtual bool SendHTTPRequestAndStreamResponse(HTTPRequestHandle, SteamAPICall_t*) = 0;
    virtual bool DeferHTTPRequest(HTTPRequestHandle) = 0;
    virtual bool PrioritizeHTTPRequest(HTTPRequestHandle) = 0;
    virtual bool GetHTTPResponseHeaderSize(HTTPRequestHandle, const char*, uint32_t*) = 0;
    virtual bool GetHTTPResponseHeaderValue(HTTPRequestHandle, const char*, uint8_t*, uint32_t) = 0;
    virtual bool GetHTTPResponseBodySize(HTTPRequestHandle, uint32_t*) = 0;
    virtual bool GetHTTPResponseBodyData(HTTPRequestHandle, uint8_t*, uint32_t) = 0;
    virtual bool GetHTTPStreamingResponseBodyData(HTTPRequestHandle, uint32_t, uint8_t*, uint32_t) = 0;
    virtual bool ReleaseHTTPRequest(HTTPRequestHandle) = 0;
    virtual bool GetHTTPDownloadProgressPct(HTTPRequestHandle, float*) = 0;
    virtual bool SetHTTPRequestRawPostBody(HTTPRequestHandle, const char*, uint8_t*, uint32_t) = 0;
    virtual HTTPCookieContainerHandle CreateCookieContainer(bool) = 0;
    virtual bool ReleaseCookieContainer(HTTPCookieContainerHandle) = 0;
    virtual bool SetCookie(HTTPCookieContainerHandle, const char*, const char*, const char*) = 0;
    virtual bool SetHTTPRequestCookieContainer(HTTPRequestHandle, HTTPCookieContainerHandle) = 0;
    virtual bool SetHTTPRequestUserAgentInfo(HTTPRequestHandle, const char*) = 0;
    virtual bool SetHTTPRequestRequiresVerifiedCertificate(HTTPRequestHandle, bool) = 0;
    virtual bool SetHTTPRequestAbsoluteTimeoutMS(HTTPRequestHandle, uint32_t) = 0;
    virtual bool GetHTTPRequestWasTimedOut(HTTPRequestHandle, bool*) = 0;
};

class ISteamScreenshots {
public:
    virtual ScreenshotHandle WriteScreenshot(void*, uint32_t, int, int) = 0;
    virtual ScreenshotHandle AddScreenshotToLibrary(const char*, const char*, int, int) = 0;
    virtual void TriggerScreenshot() = 0;
    virtual void HookScreenshots(bool) = 0;
    virtual bool SetLocation(ScreenshotHandle, const char*) = 0;
    virtual bool TagUser(ScreenshotHandle, CSteamID) = 0;
    virtual bool TagPublishedFile(ScreenshotHandle, PublishedFileId_t) = 0;
    virtual bool IsScreenshotsHooked() = 0;
    virtual ScreenshotHandle AddVRScreenshotToLibrary(int, const char*, const char*) = 0;
};

class ISteamController {
public:
    virtual bool Init() = 0;
    virtual bool Shutdown() = 0;
    virtual void RunFrame() = 0;
    virtual int GetConnectedControllers(ControllerHandle_t*) = 0;
    virtual ControllerActionSetHandle_t GetActionSetHandle(const char*) = 0;
    virtual void ActivateActionSet(ControllerHandle_t, ControllerActionSetHandle_t) = 0;
    virtual ControllerActionSetHandle_t GetCurrentActionSet(ControllerHandle_t) = 0;
    virtual void ActivateActionSetLayer(ControllerHandle_t, ControllerActionSetHandle_t) = 0;
    virtual void DeactivateActionSetLayer(ControllerHandle_t, ControllerActionSetHandle_t) = 0;
    virtual void DeactivateAllActionSetLayers(ControllerHandle_t) = 0;
    virtual int GetActiveActionSetLayers(ControllerHandle_t, ControllerActionSetHandle_t*) = 0;
    virtual ControllerDigitalActionHandle_t GetDigitalActionHandle(const char*) = 0;
    virtual InputDigitalActionData_t GetDigitalActionData(ControllerHandle_t, ControllerDigitalActionHandle_t) = 0;
    virtual int GetDigitalActionOrigins(ControllerHandle_t, ControllerActionSetHandle_t, ControllerDigitalActionHandle_t, int*) = 0;
    virtual ControllerAnalogActionHandle_t GetAnalogActionHandle(const char*) = 0;
    virtual InputAnalogActionData_t GetAnalogActionData(ControllerHandle_t, ControllerAnalogActionHandle_t) = 0;
    virtual int GetAnalogActionOrigins(ControllerHandle_t, ControllerActionSetHandle_t, ControllerAnalogActionHandle_t, int*) = 0;
    virtual const char* GetGlyphForActionOrigin(int) = 0;
    virtual const char* GetStringForActionOrigin(int) = 0;
    virtual void StopAnalogActionMomentum(ControllerHandle_t, ControllerAnalogActionHandle_t) = 0;
    virtual InputMotionData_t GetMotionData(ControllerHandle_t) = 0;
    virtual void TriggerHapticPulse(ControllerHandle_t, int, unsigned short) = 0;
    virtual void TriggerRepeatedHapticPulse(ControllerHandle_t, int, unsigned short, unsigned short, unsigned short, unsigned int) = 0;
    virtual void TriggerVibration(ControllerHandle_t, unsigned short, unsigned short) = 0;
    virtual void SetLEDColor(ControllerHandle_t, uint8_t, uint8_t, uint8_t, unsigned int) = 0;
    virtual bool ShowBindingPanel(ControllerHandle_t) = 0;
    virtual int GetInputTypeForHandle(ControllerHandle_t) = 0;
    virtual ControllerHandle_t GetControllerForGamepadIndex(int) = 0;
    virtual int GetGamepadIndexForController(ControllerHandle_t) = 0;
    virtual const char* GetStringForXboxOrigin(int) = 0;
    virtual const char* GetGlyphForXboxOrigin(int) = 0;
    virtual int GetActionOriginFromXboxOrigin(ControllerHandle_t, int) = 0;
    virtual int TranslateActionOrigin(int, int) = 0;
    virtual bool GetControllerBindingRevision(ControllerHandle_t, int*) = 0;
};

class ISteamInput {
public:
    virtual bool Init(bool) = 0;
    virtual bool Shutdown() = 0;
    virtual void RunFrame(bool) = 0;
    virtual int GetConnectedControllers(ControllerHandle_t*) = 0;
    virtual ControllerActionSetHandle_t GetActionSetHandle(const char*) = 0;
    virtual void ActivateActionSet(ControllerHandle_t, ControllerActionSetHandle_t) = 0;
    virtual ControllerActionSetHandle_t GetCurrentActionSet(ControllerHandle_t) = 0;
    virtual void ActivateActionSetLayer(ControllerHandle_t, ControllerActionSetHandle_t) = 0;
    virtual void DeactivateActionSetLayer(ControllerHandle_t, ControllerActionSetHandle_t) = 0;
    virtual void DeactivateAllActionSetLayers(ControllerHandle_t) = 0;
    virtual int GetActiveActionSetLayers(ControllerHandle_t, ControllerActionSetHandle_t*) = 0;
    virtual ControllerDigitalActionHandle_t GetDigitalActionHandle(const char*) = 0;
    virtual InputDigitalActionData_t GetDigitalActionData(ControllerHandle_t, ControllerDigitalActionHandle_t) = 0;
    virtual int GetDigitalActionOrigins(ControllerHandle_t, ControllerActionSetHandle_t, ControllerDigitalActionHandle_t, int*) = 0;
    virtual const char* GetStringForDigitalActionName(ControllerDigitalActionHandle_t) = 0;
    virtual ControllerAnalogActionHandle_t GetAnalogActionHandle(const char*) = 0;
    virtual InputAnalogActionData_t GetAnalogActionData(ControllerHandle_t, ControllerAnalogActionHandle_t) = 0;
    virtual int GetAnalogActionOrigins(ControllerHandle_t, ControllerActionSetHandle_t, ControllerAnalogActionHandle_t, int*) = 0;
    virtual const char* GetGlyphForActionOrigin_Legacy(int) = 0;
    virtual const char* GetStringForActionOrigin_Legacy(int) = 0;
    virtual const char* GetGlyphForXboxOrigin_Legacy(int) = 0;
    virtual const char* GetStringForXboxOrigin_Legacy(int) = 0;
    virtual int GetActionOriginFromXboxOrigin(ControllerHandle_t, int) = 0;
    virtual int TranslateActionOrigin(int, int) = 0;
    virtual bool GetDeviceBindingRevision(ControllerHandle_t, int*, int*) = 0;
    virtual uint32_t GetRemotePlaySessionID(ControllerHandle_t) = 0;
    virtual uint16_t GetSessionInputConfigurationSettings() = 0;
    virtual void SetDualSenseTriggerEffect(ControllerHandle_t, const void*, int) = 0;
    virtual void StopAnalogActionMomentum(ControllerHandle_t, ControllerAnalogActionHandle_t) = 0;
    virtual InputMotionData_t GetMotionData(ControllerHandle_t) = 0;
    virtual void TriggerVibration(ControllerHandle_t, unsigned short, unsigned short) = 0;
    virtual void SetLEDColor(ControllerHandle_t, uint8_t, uint8_t, uint8_t, unsigned int) = 0;
    virtual void TriggerVibrationExtended(ControllerHandle_t, unsigned short, unsigned short, unsigned short, unsigned short) = 0;
    virtual void TriggerSimpleHapticEvent(ControllerHandle_t, int, uint8_t, uint8_t, uint8_t) = 0;
    virtual int GetActionOriginFromXboxOrigin_Legacy(int) = 0;
};

class ISteamMusic {
public:
    virtual bool BIsEnabled() = 0;
    virtual bool BIsPlaying() = 0;
    virtual int GetPlaybackStatus() = 0;
    virtual void Play() = 0;
    virtual void Pause() = 0;
    virtual void PlayPrevious() = 0;
    virtual void PlayNext() = 0;
    virtual void SetVolume(float) = 0;
    virtual float GetVolume() = 0;
};

class ISteamVideo {
public:
    virtual void GetVideoURL(AppId_t) = 0;
    virtual bool IsBroadcasting(int*) = 0;
    virtual void GetOPFSettings(AppId_t) = 0;
    virtual bool GetOPFStringForApp(AppId_t, char*, int32_t*) = 0;
    virtual bool GetOPFIntForApp(AppId_t, int32_t*) = 0;
};

class ISteamParentalSettings {
public:
    virtual bool BIsParentalLockEnabled() = 0;
    virtual bool BIsParentalLockLocked() = 0;
    virtual bool BIsAppBlocked(AppId_t) = 0;
    virtual bool BIsAppInBlockList(AppId_t) = 0;
    virtual bool BIsFeatureBlocked(int) = 0;
    virtual bool BIsFeatureInBlockList(int) = 0;
};

class ISteamAppList {
public:
    virtual uint32_t GetNumInstalledApps() = 0;
    virtual uint32_t GetInstalledApps(AppId_t*, uint32_t) = 0;
    virtual int GetAppName(AppId_t, char*, int) = 0;
    virtual int GetAppInstallDir(AppId_t, char*, int) = 0;
    virtual int GetAppBuildId(AppId_t) = 0;
};

class ISteamGameServer {
public:
    virtual bool InitGameServer(uint32_t, uint16_t, uint16_t, uint16_t, int, const char*) = 0;
    virtual void SetProduct(const char*) = 0;
    virtual void SetGameDescription(const char*) = 0;
    virtual void SetModDir(const char*) = 0;
    virtual void SetDedicatedServer(bool) = 0;
    virtual void LogOn(const char*) = 0;
    virtual void LogOnAnonymous() = 0;
    virtual void LogOff() = 0;
    virtual bool BLoggedOn() = 0;
    virtual bool BSecure() = 0;
    virtual CSteamID GetSteamID() = 0;
    virtual bool WasRestartRequested() = 0;
    virtual void SetMaxPlayerCount(int) = 0;
    virtual void SetBotPlayerCount(int) = 0;
    virtual void SetServerName(const char*) = 0;
    virtual void SetMapName(const char*) = 0;
    virtual void SetPasswordProtected(bool) = 0;
    virtual void SetSpectatorPort(uint16_t) = 0;
    virtual void SetSpectatorServerName(const char*) = 0;
    virtual void ClearAllKeyValues() = 0;
    virtual void SetKeyValue(const char*, const char*) = 0;
    virtual void SetGameTags(const char*) = 0;
    virtual void SetGameData(const char*) = 0;
    virtual void SetRegion(const char*) = 0;
    virtual void SetAdvertiseServerActive(bool) = 0;
    virtual HAuthTicket GetAuthSessionTicket(void*, int, uint32_t*) = 0;
    virtual EBeginAuthSessionResult BeginAuthSession(const void*, int, CSteamID) = 0;
    virtual void EndAuthSession(CSteamID) = 0;
    virtual void CancelAuthTicket(HAuthTicket) = 0;
    virtual EUserHasLicenseForAppResult UserHasLicenseForApp(CSteamID, AppId_t) = 0;
    virtual bool RequestUserGroupStatus(CSteamID, CSteamID) = 0;
    virtual void GetGameplayStats() = 0;
    virtual SteamAPICall_t GetServerReputation() = 0;
    virtual SteamIPAddress_t GetPublicIP() = 0;
    virtual void HandleIncomingPacket(const void*, int, uint32_t, uint16_t) = 0;
    virtual int GetNextOutgoingPacket(void*, int, uint32_t*, uint16_t*) = 0;
    virtual SteamAPICall_t AssociateWithClan(CSteamID) = 0;
    virtual SteamAPICall_t ComputeNewPlayerCompatibility(SteamAPICall_t) = 0;
    virtual bool SendUserConnectAndAuthenticate_DEPRECATED(uint32_t, const void*, uint32_t, CSteamID*) = 0;
    virtual CSteamID CreateUnauthenticatedUserConnection() = 0;
    virtual void SendUserDisconnect_DEPRECATED(CSteamID) = 0;
    virtual bool BUpdateUserData(CSteamID, const char*, uint32_t) = 0;
    virtual bool GetCertificateValidationBundle(void*, int) = 0;
};

class ISteamGameServerStats {
public:
    virtual SteamAPICall_t RequestUserStats(CSteamID) = 0;
    virtual bool GetUserStat(CSteamID, const char*, int32_t*) = 0;
    virtual bool GetUserStat(CSteamID, const char*, float*) = 0;
    virtual bool GetUserAchievement(CSteamID, const char*, bool*) = 0;
    virtual bool SetUserStat(CSteamID, const char*, int32_t) = 0;
    virtual bool SetUserStat(CSteamID, const char*, float) = 0;
    virtual bool UpdateUserAvgRateStat(CSteamID, const char*, float, double) = 0;
    virtual bool SetUserAchievement(CSteamID, const char*) = 0;
    virtual bool ClearUserAchievement(CSteamID, const char*) = 0;
    virtual SteamAPICall_t StoreUserStats(CSteamID) = 0;
};

} // namespace sfemu
