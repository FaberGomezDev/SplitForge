#include "impls.h"
#include "interfaces.h"
#include "state.h"

#include <cstring>

namespace sfemu {

// Tipos mínimos de las interfaces extra (solo se necesitan como punteros/valores opacos;
// los stubs devuelven vacío/cero y nunca rellenan estas estructuras).

struct SteamNetworkingIdentity {
    int m_eType;
    char m_cbSize;   // marcador de posición; el tamaño real es 32 bytes
    char m_reserved[27];
};

struct SteamNetworkingIPAddr {
    uint8_t m_ipv6[16];
    uint16_t m_port;
    uint16_t m_pad;
};

struct SteamNetworkPingLocation {
    uint8_t m_data[512];
};

struct SteamRelayNetworkStatus {
    int m_eAvail;
    int m_bPingMeasurementInProgress;
    int m_eAvailNetworkConfig;
    int m_eAvailAnyRelay;
    char m_debugMsg[256];
};

struct SteamNetworkingConfigValue {
    int m_eValue;
    int m_eDataType;
    int64_t m_val_int64;
    int m_val_int32;
    float m_val_float;
    const char* m_val_string;
    void* m_val_functionPtr;
};

struct SteamNetworkingMessage {
    void* m_pData;
    int m_cbSize;
    SteamNetworkingIdentity m_identityPeer;
    int64_t m_nConnUserData;
    long long m_usecTimeReceived;
    int64_t m_nMessageNumber;
    void* m_pfnFreeData;
    void* m_pfnRelease;
    int m_nChannel;
    int m_nFlags;
    long long m_nUserData;
    uint16_t m_idxLane;
    uint16_t m_pad1;
};

struct SteamNetConnectionInfo {
    SteamNetworkingIdentity m_identityRemote;
    long long m_nUserData;
    uint32_t m_hListenSocket;
    SteamNetworkingIPAddr m_addrRemote;
    uint16_t m_pad1;
    uint32_t m_idPOPRemote;
    uint32_t m_idPOPRelay;
    int m_eState;
    int m_eEndReason;
    char m_szEndDebug[128];
    char m_szConnectionDescription[128];
};

struct SteamNetConnectionRealTimeStatus {
    int m_eState;
    int m_nPing;
    float m_flConnectionQualityLocal;
    float m_flConnectionQualityRemote;
    float m_flOutPacketsPerSec;
    float m_flOutBytesPerSec;
    float m_flInPacketsPerSec;
    float m_flInBytesPerSec;
    int m_nSendRateBytesPerSecond;
    int m_cbPendingUnreliable;
    int m_cbPendingReliable;
    int m_cbSentUnackedReliable;
    long long m_usecQueueTime;
    uint32_t m_pad1;
};

struct SteamNetConnectionRealTimeLaneStatus {
    int m_cbPendingUnreliable;
    int m_cbPendingReliable;
    int m_cbSentUnackedReliable;
    int m_usecQueueTime;
    uint32_t m_pad1;
};

struct SteamNetAuthenticationStatus {
    int m_eAvail;
    char m_debugMsg[256];
};

struct SteamNetworkingFakeIPResult {
    EResult m_eResult;
    SteamNetworkingIdentity m_identity;
    uint32_t m_unIP;
    uint16_t m_pad[2];
};

struct SteamPartyBeaconLocation {
    int m_eType;
    uint64_t m_ulLocationID;
};

class SteamGameSearchImpl {
public:
    virtual void AddGameSearchParams(const char*, const char*) {}
    virtual void SearchForGameWithLobby(CSteamID, int, int) {}
    virtual void SearchForGameSolo(int, int) {}
    virtual void AcceptGame() {}
    virtual void DeclineGame() {}
    virtual void RetrieveConnectionDetails(CSteamID, char*, int) {}
    virtual void EndGameSearch() {}
    virtual void SetGameHostParams(const char*, const char*) {}
    virtual void SetConnectionDetails(const char*, int) {}
    virtual void RequestPlayersForGame(int, int, int) {}
    virtual void HostConfirmGameStart(uint64_t) {}
    virtual void CancelRequestPlayersForGame() {}
    virtual void SubmitPlayerResult(uint64_t, CSteamID, int) {}
    virtual void EndGame(uint64_t) {}
};

class SteamMusicRemoteImpl {
public:
    virtual void RegisterSteamMusicRemote(const char*) {}
    virtual void DeregisterSteamMusicRemote() {}
    virtual bool BIsCurrentMusicRemote() { return false; }
    virtual void BActivationSuccess(bool) {}
    virtual void SetDisplayName(const char*) {}
    virtual void SetPNGIcon_64x64(void*, uint32_t) {}
    virtual void EnablePlayPrevious(bool) {}
    virtual void EnablePlayNext(bool) {}
    virtual void EnableShuffled(bool) {}
    virtual void EnableLooped(bool) {}
    virtual void EnableQueue(bool) {}
    virtual void EnablePlaylists(bool) {}
    virtual void UpdatePlaybackStatus(int) {}
    virtual void UpdateShuffled(bool) {}
    virtual void UpdateLooped(bool) {}
    virtual void UpdateVolume(float) {}
    virtual void CurrentEntryWillChange() {}
    virtual void CurrentEntryIsAvailable(bool) {}
    virtual void UpdateCurrentEntryText(const char*) {}
    virtual void UpdateCurrentEntryElapsedSeconds(int) {}
    virtual void UpdateCurrentEntryCoverArt(void*, uint32_t) {}
    virtual void CurrentEntryDidChange() {}
    virtual void QueueWillChange() {}
    virtual void QueueSetDirty() {}
    virtual void QueueAddEntry(const char*, int) {}
    virtual void QueueRemoveEntry(int) {}
    virtual void QueueGetEntry(int, int*) {}
    virtual void QueueClear() {}
    virtual void QueueDidChange() {}
    virtual void PlaylistWillChange() {}
    virtual void PlaylistSetDirty() {}
    virtual void PlaylistAddEntry(int, int, int) {}
    virtual void PlaylistRemoveEntry(int) {}
    virtual void PlaylistGetEntry(int, int*, int*) {}
    virtual void PlaylistClear() {}
    virtual void PlaylistDidChange() {}
};

class SteamHTMLSurfaceImpl {
public:
    virtual bool Init() { return true; }
    virtual bool Shutdown() { return true; }
    virtual SteamAPICall_t CreateBrowser(const char*, const char*) { return 0; }
    virtual void RemoveBrowser(uint32_t) {}
    virtual void LoadURL(uint32_t, const char*, const char*) {}
    virtual void SetSize(uint32_t, uint32_t, uint32_t) {}
    virtual void StopLoad(uint32_t) {}
    virtual void Reload(uint32_t) {}
    virtual void GoBack(uint32_t) {}
    virtual void GoForward(uint32_t) {}
    virtual void AddHeader(uint32_t, const char*, const char*) {}
    virtual void ExecuteJavascript(uint32_t, const char*) {}
    virtual void MouseUp(uint32_t, int) {}
    virtual void MouseDown(uint32_t, int) {}
    virtual void MouseDoubleClick(uint32_t, int) {}
    virtual void MouseMove(uint32_t, int, int) {}
    virtual void MouseWheel(uint32_t, int32_t) {}
    virtual void KeyDown(uint32_t, uint32_t, int, uint32_t) {}
    virtual void KeyUp(uint32_t, uint32_t, int) {}
    virtual void KeyChar(uint32_t, uint32_t, int) {}
    virtual void SetHorizontalScroll(uint32_t, uint32_t) {}
    virtual void SetVerticalScroll(uint32_t, uint32_t) {}
    virtual void SetKeyFocus(uint32_t, bool) {}
    virtual void ViewSource(uint32_t) {}
    virtual void CopyToClipboard(uint32_t) {}
    virtual void PasteFromClipboard(uint32_t) {}
    virtual void Find(uint32_t, const char*, bool, bool) {}
    virtual void StopFind(uint32_t) {}
    virtual void GetLinkAtPosition(uint32_t, int, int) {}
    virtual void SetCookie(const char*, const char*, const char*, const char*, uint32_t, bool, bool) {}
    virtual void SetPageScaleFactor(uint32_t, float, int, int) {}
    virtual void SetBackgroundMode(uint32_t, bool) {}
    virtual void SetDPIScalingFactor(uint32_t, float) {}
    virtual void OpenDeveloperTools(uint32_t) {}
    virtual void AllowStartRequest(uint32_t, bool) {}
    virtual void JSDialogResponse(uint32_t, bool) {}
    virtual void FileLoadDialogResponse(uint32_t, const char**) {}
    virtual void SetFullscreen(uint32_t, bool) {}
    virtual void SetScrollbarsEnabled(uint32_t, bool) {}
};

class SteamPartiesImpl {
public:
    virtual uint32_t GetNumActiveBeacons() { return 0; }
    virtual uint64_t GetBeaconByIndex(uint32_t) { return 0; }
    virtual bool GetBeaconDetails(uint64_t, CSteamID*, SteamPartyBeaconLocation*, char*, int) {
        return false;
    }
    virtual SteamAPICall_t JoinParty(uint64_t) { return 0; }
    virtual uint32_t GetNumAvailableBeaconLocations(uint32_t*) { return 0; }
    virtual uint32_t GetAvailableBeaconLocations(SteamPartyBeaconLocation*, uint32_t) { return 0; }
    virtual SteamAPICall_t CreateBeacon(uint32_t, SteamPartyBeaconLocation*, int, int, const char*) {
        return 0;
    }
    virtual void OnReservationCompleted(uint64_t, CSteamID) {}
    virtual void CancelReservation(uint64_t, CSteamID) {}
    virtual void ChangeNumOpenSlots(uint64_t, uint32_t) {}
    virtual bool DestroyBeacon(uint64_t) { return false; }
    virtual bool GetBeaconLocationData(SteamPartyBeaconLocation, int, char*, int) { return false; }
};

class SteamRemotePlayImpl {
public:
    virtual uint32_t GetSessionCount() { return 0; }
    virtual uint32_t GetSessionID(int) { return 0; }
    virtual CSteamID GetSessionSteamID(uint32_t) { return k_steamIDNil; }
    virtual const char* GetSessionClientName(uint32_t) { return ""; }
    virtual int GetSessionClientFormFactor(uint32_t) { return 0; }
    virtual bool BGetSessionClientResolution(uint32_t, int*, int*) { return false; }
    virtual bool BSendRemotePlayTogetherInvite(CSteamID) { return false; }
};

class SteamNetworkingUtilsImpl {
public:
    virtual void* AllocateMessage(int) { return nullptr; }
    virtual void InitRelayNetworkAccess() {}
    virtual int GetRelayNetworkStatus(SteamRelayNetworkStatus*) { return 0; }
    virtual float GetLocalPingLocation(SteamNetworkPingLocation&) { return 0.0f; }
    virtual int EstimatePingTimeBetweenLocations(const SteamNetworkPingLocation&,
                                                 const SteamNetworkPingLocation&) {
        return -1;
    }
    virtual int EstimatePingTimeFromLocalHost(const SteamNetworkPingLocation&) { return -1; }
    virtual void ConvertPingLocationToString(const SteamNetworkPingLocation&, char*, int) {}
    virtual bool ParsePingLocationString(const char*, SteamNetworkPingLocation&) { return false; }
    virtual bool CheckPingDataUpToDate(float) { return true; }
    virtual int GetPingToDataCenter(uint32_t, uint32_t*) { return -1; }
    virtual int GetDirectPingToPOP(uint32_t) { return -1; }
    virtual int GetPOPCount() { return 0; }
    virtual int GetPOPList(void*, int) { return 0; }
    virtual long long GetLocalTimestamp() { return 0; }
    virtual void SetDebugOutputFunction(int, void*) {}
    virtual bool IsFakeIPv4(uint32_t) { return false; }
    virtual int GetIPv4FakeIPType(uint32_t) { return 0; }
    virtual EResult GetRealIdentityForFakeIP(const SteamNetworkingIPAddr&,
                                             SteamNetworkingIdentity*) {
        return k_EResultFail;
    }
    virtual bool SetConfigValue(int, int, intptr_t, int, const void*) { return false; }
    virtual int GetConfigValue(int, int, intptr_t, int*, void*, size_t*) { return 0; }
    virtual const char* GetConfigValueInfo(int, const char**, int*, int*, int*) { return nullptr; }
    virtual int GetFirstConfigValue() { return -1; }
    virtual void SteamNetworkingIPAddr_ToString(const SteamNetworkingIPAddr&, char*, size_t, bool) {}
    virtual bool SteamNetworkingIPAddr_ParseString(SteamNetworkingIPAddr*, const char*) {
        return false;
    }
    virtual int SteamNetworkingIPAddr_GetFakeIPType(const SteamNetworkingIPAddr&) { return 0; }
    virtual void SteamNetworkingIdentity_ToString(const SteamNetworkingIdentity&, char*, size_t) {}
    virtual bool SteamNetworkingIdentity_ParseString(SteamNetworkingIdentity*, size_t,
                                                     const char*) {
        return false;
    }
};

class SteamNetworkingSocketsImpl {
public:
    virtual uint32_t CreateListenSocketIP(const SteamNetworkingIPAddr&, int,
                                          const SteamNetworkingConfigValue*) {
        return 0;
    }
    virtual uint32_t CreateListenSocketP2P(int, int, const SteamNetworkingConfigValue*) { return 0; }
    virtual uint32_t ConnectByIPAddress(const SteamNetworkingIPAddr&, int,
                                        const SteamNetworkingConfigValue*) {
        return 0;
    }
    virtual uint32_t ConnectP2P(const SteamNetworkingIdentity&, int, int,
                                const SteamNetworkingConfigValue*) {
        return 0;
    }
    virtual EResult AcceptConnection(uint32_t) { return k_EResultInvalidParam; }
    virtual bool CloseConnection(uint32_t, int, const char*, bool) { return false; }
    virtual bool CloseListenSocket(uint32_t) { return false; }
    virtual bool SetConnectionUserData(uint32_t, int64_t) { return false; }
    virtual int64_t GetConnectionUserData(uint32_t) { return -1; }
    virtual void SetConnectionName(uint32_t, const char*) {}
    virtual bool GetConnectionName(uint32_t, char*, int) { return false; }
    virtual EResult SendMessageToConnection(uint32_t, const void*, uint32_t, int, int64_t*) {
        return k_EResultInvalidParam;
    }
    virtual EResult FlushMessagesOnConnection(uint32_t) { return k_EResultInvalidParam; }
    virtual int ReceiveMessagesOnConnection(uint32_t, SteamNetworkingMessage**, int) { return 0; }
    virtual int ReceiveMessagesOnListenSocket(uint32_t, SteamNetworkingMessage**, int) { return 0; }
    virtual bool GetConnectionInfo(uint32_t, SteamNetConnectionInfo*) { return false; }
    virtual EResult GetConnectionRealTimeStatus(uint32_t, SteamNetConnectionRealTimeStatus*, int,
                                                SteamNetConnectionRealTimeLaneStatus*) {
        return k_EResultInvalidParam;
    }
    virtual int GetDetailedConnectionStatus(uint32_t, char*, int) { return 0; }
    virtual bool GetListenSocketAddress(uint32_t, SteamNetworkingIPAddr*) { return false; }
    virtual bool CreateSocketPair(uint32_t*, uint32_t*, bool, const SteamNetworkingIdentity*,
                                  const SteamNetworkingIdentity*) {
        return false;
    }
    virtual EResult ConfigureConnectionLanes(uint32_t, int, const int*, const uint16_t*) {
        return k_EResultInvalidParam;
    }
    virtual bool GetIdentity(SteamNetworkingIdentity* identity) {
        if (identity) {
            std::memset(identity, 0, sizeof(SteamNetworkingIdentity));
            identity->m_eType = 1;
        }
        return true;
    }
    virtual int InitAuthentication() { return 2; }
    virtual int GetAuthenticationStatus(SteamNetAuthenticationStatus*) { return 2; }
    virtual uint32_t CreatePollGroup() { return 0; }
    virtual bool DestroyPollGroup(uint32_t) { return false; }
    virtual bool SetConnectionPollGroup(uint32_t, uint32_t) { return false; }
    virtual int ReceiveMessagesOnPollGroup(uint32_t, SteamNetworkingMessage**, int) { return 0; }
    virtual EResult ReceivedRelayAuthTicket(const void*, int, uint32_t*, SteamNetworkingIdentity*) {
        return k_EResultFail;
    }
    virtual int FindRelayAuthTicketForServer(const SteamNetworkingIdentity&, int, void*) {
        return 0;
    }
    virtual uint32_t ConnectToHostedDedicatedServer(const SteamNetworkingIdentity&, int, int,
                                                    const SteamNetworkingConfigValue*) {
        return 0;
    }
    virtual uint16_t GetHostedDedicatedServerPort() { return 0; }
    virtual uint32_t GetHostedDedicatedServerPOPID() { return 0; }
    virtual EResult GetHostedDedicatedServerAddress(void*) { return k_EResultFail; }
    virtual uint32_t CreateHostedDedicatedServerListenSocket(int, int,
                                                             const SteamNetworkingConfigValue*) {
        return 0;
    }
    virtual EResult GetGameCoordinatorServerLogin(void*, int*, void*) { return k_EResultFail; }
    virtual uint32_t ConnectP2PCustomSignaling(void*, const SteamNetworkingIdentity*, int, int,
                                               const SteamNetworkingConfigValue*) {
        return 0;
    }
    virtual bool ReceivedP2PCustomSignal(const void*, int, void*) { return false; }
    virtual bool GetCertificateRequest(int*, void*, char*) { return false; }
    virtual bool SetCertificate(const void*, int, char*) { return false; }
    virtual void ResetIdentity(const SteamNetworkingIdentity*) {}
    virtual void RunCallbacks(void*) {}
    virtual bool BeginAsyncRequestFakeIP(int) { return false; }
    virtual void GetFakeIP(int, SteamNetworkingFakeIPResult*) {}
    virtual void* CreateFakeUDPPort(int) { return nullptr; }
    virtual void DestroyFakeUDPPort(void*) {}
};

class SteamNetworkingMessagesImpl {
public:
    virtual EResult SendMessageToUser(const SteamNetworkingIdentity&, const void*, uint32_t, int,
                                      int) {
        return k_EResultFail;
    }
    virtual int ReceiveMessagesOnChannel(int, SteamNetworkingMessage**, int) { return 0; }
    virtual bool AcceptSessionWithUser(const SteamNetworkingIdentity&) { return false; }
    virtual bool CloseSessionWithUser(const SteamNetworkingIdentity&) { return false; }
    virtual bool CloseChannelWithUser(const SteamNetworkingIdentity&, int) { return false; }
    virtual int GetSessionConnectionInfo(const SteamNetworkingIdentity&, SteamNetConnectionInfo*,
                                         SteamNetConnectionRealTimeStatus*) {
        return 0;
    }
};

} // namespace sfemu

namespace sfemu {

void* GameSearchInstance() {
    static SteamGameSearchImpl impl;
    return &impl;
}

void* MusicRemoteInstance() {
    static SteamMusicRemoteImpl impl;
    return &impl;
}

void* HtmlSurfaceInstance() {
    static SteamHTMLSurfaceImpl impl;
    return &impl;
}

void* PartiesInstance() {
    static SteamPartiesImpl impl;
    return &impl;
}

void* RemotePlayInstance() {
    static SteamRemotePlayImpl impl;
    return &impl;
}

void* NetworkingUtilsInstance() {
    static SteamNetworkingUtilsImpl impl;
    return &impl;
}

void* NetworkingSocketsInstance() {
    static SteamNetworkingSocketsImpl impl;
    return &impl;
}

void* NetworkingMessagesInstance() {
    static SteamNetworkingMessagesImpl impl;
    return &impl;
}

} // namespace sfemu
