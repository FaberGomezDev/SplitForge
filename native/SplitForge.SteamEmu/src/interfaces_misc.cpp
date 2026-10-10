#include "impls.h"
#include "interfaces.h"
#include "state.h"

#include <cstring>

namespace sfemu {

namespace {

class SteamControllerImpl final : public ISteamController {
public:
    bool Init() override { return true; }
    bool Shutdown() override { return true; }
    void RunFrame() override {}
    int GetConnectedControllers(ControllerHandle_t*) override { return 0; }
    ControllerActionSetHandle_t GetActionSetHandle(const char*) override { return 0; }
    void ActivateActionSet(ControllerHandle_t, ControllerActionSetHandle_t) override {}
    ControllerActionSetHandle_t GetCurrentActionSet(ControllerHandle_t) override { return 0; }
    void ActivateActionSetLayer(ControllerHandle_t, ControllerActionSetHandle_t) override {}
    void DeactivateActionSetLayer(ControllerHandle_t, ControllerActionSetHandle_t) override {}
    void DeactivateAllActionSetLayers(ControllerHandle_t) override {}
    int GetActiveActionSetLayers(ControllerHandle_t, ControllerActionSetHandle_t*) override { return 0; }
    ControllerDigitalActionHandle_t GetDigitalActionHandle(const char*) override { return 0; }
    InputDigitalActionData_t GetDigitalActionData(ControllerHandle_t, ControllerDigitalActionHandle_t) override {
        return {};
    }
    int GetDigitalActionOrigins(ControllerHandle_t, ControllerActionSetHandle_t,
                                ControllerDigitalActionHandle_t, int*) override {
        return 0;
    }
    ControllerAnalogActionHandle_t GetAnalogActionHandle(const char*) override { return 0; }
    InputAnalogActionData_t GetAnalogActionData(ControllerHandle_t, ControllerAnalogActionHandle_t) override {
        return {};
    }
    int GetAnalogActionOrigins(ControllerHandle_t, ControllerActionSetHandle_t,
                               ControllerAnalogActionHandle_t, int*) override {
        return 0;
    }
    const char* GetGlyphForActionOrigin(int) override { return ""; }
    const char* GetStringForActionOrigin(int) override { return ""; }
    void StopAnalogActionMomentum(ControllerHandle_t, ControllerAnalogActionHandle_t) override {}
    InputMotionData_t GetMotionData(ControllerHandle_t) override { return {}; }
    void TriggerHapticPulse(ControllerHandle_t, int, unsigned short) override {}
    void TriggerRepeatedHapticPulse(ControllerHandle_t, int, unsigned short, unsigned short,
                                    unsigned short, unsigned int) override {}
    void TriggerVibration(ControllerHandle_t, unsigned short, unsigned short) override {}
    void SetLEDColor(ControllerHandle_t, uint8_t, uint8_t, uint8_t, unsigned int) override {}
    bool ShowBindingPanel(ControllerHandle_t) override { return false; }
    int GetInputTypeForHandle(ControllerHandle_t) override { return k_ESteamInputType_Unknown; }
    ControllerHandle_t GetControllerForGamepadIndex(int) override { return 0; }
    int GetGamepadIndexForController(ControllerHandle_t) override { return -1; }
    const char* GetStringForXboxOrigin(int) override { return ""; }
    const char* GetGlyphForXboxOrigin(int) override { return ""; }
    int GetActionOriginFromXboxOrigin(ControllerHandle_t, int) override { return 0; }
    int TranslateActionOrigin(int, int) override { return 0; }
    bool GetControllerBindingRevision(ControllerHandle_t, int*) override { return false; }
};

class SteamInputImpl final : public ISteamInput {
public:
    bool Init(bool) override { return true; }
    bool Shutdown() override { return true; }
    void RunFrame(bool) override {}
    int GetConnectedControllers(ControllerHandle_t*) override { return 0; }
    ControllerActionSetHandle_t GetActionSetHandle(const char*) override { return 0; }
    void ActivateActionSet(ControllerHandle_t, ControllerActionSetHandle_t) override {}
    ControllerActionSetHandle_t GetCurrentActionSet(ControllerHandle_t) override { return 0; }
    void ActivateActionSetLayer(ControllerHandle_t, ControllerActionSetHandle_t) override {}
    void DeactivateActionSetLayer(ControllerHandle_t, ControllerActionSetHandle_t) override {}
    void DeactivateAllActionSetLayers(ControllerHandle_t) override {}
    int GetActiveActionSetLayers(ControllerHandle_t, ControllerActionSetHandle_t*) override { return 0; }
    ControllerDigitalActionHandle_t GetDigitalActionHandle(const char*) override { return 0; }
    InputDigitalActionData_t GetDigitalActionData(ControllerHandle_t, ControllerDigitalActionHandle_t) override {
        return {};
    }
    int GetDigitalActionOrigins(ControllerHandle_t, ControllerActionSetHandle_t,
                                ControllerDigitalActionHandle_t, int*) override {
        return 0;
    }
    const char* GetStringForDigitalActionName(ControllerDigitalActionHandle_t) override { return ""; }
    ControllerAnalogActionHandle_t GetAnalogActionHandle(const char*) override { return 0; }
    InputAnalogActionData_t GetAnalogActionData(ControllerHandle_t, ControllerAnalogActionHandle_t) override {
        return {};
    }
    int GetAnalogActionOrigins(ControllerHandle_t, ControllerActionSetHandle_t,
                               ControllerAnalogActionHandle_t, int*) override {
        return 0;
    }
    const char* GetGlyphForActionOrigin_Legacy(int) override { return ""; }
    const char* GetStringForActionOrigin_Legacy(int) override { return ""; }
    const char* GetGlyphForXboxOrigin_Legacy(int) override { return ""; }
    const char* GetStringForXboxOrigin_Legacy(int) override { return ""; }
    int GetActionOriginFromXboxOrigin(ControllerHandle_t, int) override { return 0; }
    int TranslateActionOrigin(int, int) override { return 0; }
    bool GetDeviceBindingRevision(ControllerHandle_t, int*, int*) override { return false; }
    uint32_t GetRemotePlaySessionID(ControllerHandle_t) override { return 0; }
    uint16_t GetSessionInputConfigurationSettings() override { return 0; }
    void SetDualSenseTriggerEffect(ControllerHandle_t, const void*, int) override {}
    void StopAnalogActionMomentum(ControllerHandle_t, ControllerAnalogActionHandle_t) override {}
    InputMotionData_t GetMotionData(ControllerHandle_t) override { return {}; }
    void TriggerVibration(ControllerHandle_t, unsigned short, unsigned short) override {}
    void SetLEDColor(ControllerHandle_t, uint8_t, uint8_t, uint8_t, unsigned int) override {}
    void TriggerVibrationExtended(ControllerHandle_t, unsigned short, unsigned short, unsigned short,
                                  unsigned short) override {}
    void TriggerSimpleHapticEvent(ControllerHandle_t, int, uint8_t, uint8_t, uint8_t) override {}
    int GetActionOriginFromXboxOrigin_Legacy(int) override { return 0; }
};

class SteamMusicImpl final : public ISteamMusic {
public:
    bool BIsEnabled() override { return false; }
    bool BIsPlaying() override { return false; }
    int GetPlaybackStatus() override { return 0; }
    void Play() override {}
    void Pause() override {}
    void PlayPrevious() override {}
    void PlayNext() override {}
    void SetVolume(float) override {}
    float GetVolume() override { return 0.0f; }
};

class SteamVideoImpl final : public ISteamVideo {
public:
    void GetVideoURL(AppId_t) override {}
    bool IsBroadcasting(int* viewers) override {
        if (viewers)
            *viewers = 0;
        return false;
    }
    void GetOPFSettings(AppId_t) override {}
    bool GetOPFStringForApp(AppId_t, char*, int32_t*) override { return false; }
    bool GetOPFIntForApp(AppId_t, int32_t*) override { return false; }
};

class SteamParentalSettingsImpl final : public ISteamParentalSettings {
public:
    bool BIsParentalLockEnabled() override { return false; }
    bool BIsParentalLockLocked() override { return false; }
    bool BIsAppBlocked(AppId_t) override { return false; }
    bool BIsAppInBlockList(AppId_t) override { return false; }
    bool BIsFeatureBlocked(int) override { return false; }
    bool BIsFeatureInBlockList(int) override { return false; }
};

class SteamAppListImpl final : public ISteamAppList {
public:
    uint32_t GetNumInstalledApps() override { return 0; }
    uint32_t GetInstalledApps(AppId_t*, uint32_t) override { return 0; }
    int GetAppName(AppId_t, char*, int) override { return -1; }
    int GetAppInstallDir(AppId_t, char*, int) override { return -1; }
    int GetAppBuildId(AppId_t) override { return 0; }
};

class SteamGameServerImpl final : public ISteamGameServer {
public:
    bool InitGameServer(uint32_t, uint16_t, uint16_t, uint16_t, int, const char*) override {
        Emu().gameServerInitialized = true;
        return true;
    }
    void SetProduct(const char*) override {}
    void SetGameDescription(const char*) override {}
    void SetModDir(const char*) override {}
    void SetDedicatedServer(bool) override {}
    void LogOn(const char*) override {}
    void LogOnAnonymous() override {}
    void LogOff() override { Emu().gameServerInitialized = false; }
    bool BLoggedOn() override { return Emu().gameServerInitialized; }
    bool BSecure() override { return false; }
    CSteamID GetSteamID() override { return MakeSteamID(0, 1, k_EAccountTypeGameServer); }
    bool WasRestartRequested() override { return false; }
    void SetMaxPlayerCount(int) override {}
    void SetBotPlayerCount(int) override {}
    void SetServerName(const char*) override {}
    void SetMapName(const char*) override {}
    void SetPasswordProtected(bool) override {}
    void SetSpectatorPort(uint16_t) override {}
    void SetSpectatorServerName(const char*) override {}
    void ClearAllKeyValues() override {}
    void SetKeyValue(const char*, const char*) override {}
    void SetGameTags(const char*) override {}
    void SetGameData(const char*) override {}
    void SetRegion(const char*) override {}
    void SetAdvertiseServerActive(bool) override {}
    HAuthTicket GetAuthSessionTicket(void*, int, uint32_t*) override { return 1; }
    EBeginAuthSessionResult BeginAuthSession(const void*, int, CSteamID) override {
        return k_EBeginAuthSessionResultOK;
    }
    void EndAuthSession(CSteamID) override {}
    void CancelAuthTicket(HAuthTicket) override {}
    EUserHasLicenseForAppResult UserHasLicenseForApp(CSteamID, AppId_t appId) override {
        return appId == Emu().config.appId ? k_EUserHasLicenseResultHasLicense
                                           : k_EUserHasLicenseResultDoesNotHaveLicense;
    }
    bool RequestUserGroupStatus(CSteamID, CSteamID) override { return false; }
    void GetGameplayStats() override {}
    SteamAPICall_t GetServerReputation() override { return 0; }
    SteamIPAddress_t GetPublicIP() override {
        SteamIPAddress_t ip{};
        ip.m_ipv4 = 0x0100007F;
        return ip;
    }
    void HandleIncomingPacket(const void*, int, uint32_t, uint16_t) override {}
    int GetNextOutgoingPacket(void*, int, uint32_t*, uint16_t*) override { return 0; }
    SteamAPICall_t AssociateWithClan(CSteamID) override { return 0; }
    SteamAPICall_t ComputeNewPlayerCompatibility(SteamAPICall_t) override { return 0; }
    bool SendUserConnectAndAuthenticate_DEPRECATED(uint32_t, const void*, uint32_t, CSteamID*) override {
        return false;
    }
    CSteamID CreateUnauthenticatedUserConnection() override { return MakeSteamID(0, 2, k_EAccountTypeAnonUser); }
    void SendUserDisconnect_DEPRECATED(CSteamID) override {}
    bool BUpdateUserData(CSteamID, const char*, uint32_t) override { return false; }
    bool GetCertificateValidationBundle(void*, int) override { return false; }
};

} // namespace

ISteamController* ControllerInstance() {
    static SteamControllerImpl impl;
    return &impl;
}

ISteamInput* InputInstance() {
    static SteamInputImpl impl;
    return &impl;
}

ISteamMusic* MusicInstance() {
    static SteamMusicImpl impl;
    return &impl;
}

ISteamVideo* VideoInstance() {
    static SteamVideoImpl impl;
    return &impl;
}

ISteamParentalSettings* ParentalSettingsInstance() {
    static SteamParentalSettingsImpl impl;
    return &impl;
}

ISteamAppList* AppListInstance() {
    static SteamAppListImpl impl;
    return &impl;
}

ISteamGameServer* GameServerInstance() {
    static SteamGameServerImpl impl;
    return &impl;
}

} // namespace sfemu
