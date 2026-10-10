// GENERADO por tools/generar_api_plana.py a partir de src/interfaces.h y tools/steamworksnet_api.json.
// No editar a mano: cambia las entradas y vuelve a ejecutar el generador.
//
// API plana del SDK: cada método de interfaz se exporta como SteamAPI_<Interfaz>_<Método> con el
// puntero a la interfaz (self) como primer argumento. Steamworks.NET solo usa esta vía.

#include "impls.h"
#include "interfaces.h"
#include "state.h"

#include <windows.h>

#include <atomic>
#include <cstring>

namespace {

// Registra en steamemu.log la primera llamada a cada entrada (qué usa realmente cada juego).
void FirstCall(std::atomic<bool>& seen, const char* name, bool implemented) {
    if (!seen.exchange(true, std::memory_order_relaxed))
        sfemu::Emu().Log("API plana: %s%s (pid=%lu)", name, implemented ? "" : " [sin implementar]",
                         GetCurrentProcessId());
}

} // namespace

#define SF_FLAT(name, implemented)                       \
    do {                                                 \
        static std::atomic<bool> sf_seen{false};         \
        FirstCall(sf_seen, name, implemented);           \
    } while (0)

using namespace sfemu;

extern "C" {
// 683 reenvíos a las vtables del emulador.

HSteamPipe SteamAPI_ISteamClient_CreateSteamPipe(ISteamClient* self) {
    SF_FLAT("SteamAPI_ISteamClient_CreateSteamPipe", true);
    if (!self)
        return {};
    return self->CreateSteamPipe();
}

bool SteamAPI_ISteamClient_BReleaseSteamPipe(ISteamClient* self, HSteamPipe a0) {
    SF_FLAT("SteamAPI_ISteamClient_BReleaseSteamPipe", true);
    if (!self)
        return {};
    return self->BReleaseSteamPipe(a0);
}

HSteamUser SteamAPI_ISteamClient_ConnectToGlobalUser(ISteamClient* self, HSteamPipe a0) {
    SF_FLAT("SteamAPI_ISteamClient_ConnectToGlobalUser", true);
    if (!self)
        return {};
    return self->ConnectToGlobalUser(a0);
}

HSteamUser SteamAPI_ISteamClient_CreateLocalUser(ISteamClient* self, HSteamPipe* a0, EAccountType a1) {
    SF_FLAT("SteamAPI_ISteamClient_CreateLocalUser", true);
    if (!self)
        return {};
    return self->CreateLocalUser(a0, a1);
}

void SteamAPI_ISteamClient_ReleaseUser(ISteamClient* self, HSteamPipe a0, HSteamUser a1) {
    SF_FLAT("SteamAPI_ISteamClient_ReleaseUser", true);
    if (self)
        self->ReleaseUser(a0, a1);
}

ISteamUser* SteamAPI_ISteamClient_GetISteamUser(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamUser", true);
    if (!self)
        return {};
    return self->GetISteamUser(a0, a1, a2);
}

ISteamGameServer* SteamAPI_ISteamClient_GetISteamGameServer(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamGameServer", true);
    if (!self)
        return {};
    return self->GetISteamGameServer(a0, a1, a2);
}

void SteamAPI_ISteamClient_SetLocalIPBinding(ISteamClient* self, const SteamIPAddress_t& a0, uint16_t a1) {
    SF_FLAT("SteamAPI_ISteamClient_SetLocalIPBinding", true);
    if (self)
        self->SetLocalIPBinding(a0, a1);
}

ISteamFriends* SteamAPI_ISteamClient_GetISteamFriends(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamFriends", true);
    if (!self)
        return {};
    return self->GetISteamFriends(a0, a1, a2);
}

ISteamUtils* SteamAPI_ISteamClient_GetISteamUtils(ISteamClient* self, HSteamPipe a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamUtils", true);
    if (!self)
        return {};
    return self->GetISteamUtils(a0, a1);
}

ISteamMatchmaking* SteamAPI_ISteamClient_GetISteamMatchmaking(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamMatchmaking", true);
    if (!self)
        return {};
    return self->GetISteamMatchmaking(a0, a1, a2);
}

ISteamMatchmakingServers* SteamAPI_ISteamClient_GetISteamMatchmakingServers(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamMatchmakingServers", true);
    if (!self)
        return {};
    return self->GetISteamMatchmakingServers(a0, a1, a2);
}

void* SteamAPI_ISteamClient_GetISteamGenericInterface(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamGenericInterface", true);
    if (!self)
        return {};
    return self->GetISteamGenericInterface(a0, a1, a2);
}

ISteamUserStats* SteamAPI_ISteamClient_GetISteamUserStats(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamUserStats", true);
    if (!self)
        return {};
    return self->GetISteamUserStats(a0, a1, a2);
}

ISteamGameServerStats* SteamAPI_ISteamClient_GetISteamGameServerStats(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamGameServerStats", true);
    if (!self)
        return {};
    return self->GetISteamGameServerStats(a0, a1, a2);
}

ISteamApps* SteamAPI_ISteamClient_GetISteamApps(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamApps", true);
    if (!self)
        return {};
    return self->GetISteamApps(a0, a1, a2);
}

ISteamNetworking* SteamAPI_ISteamClient_GetISteamNetworking(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamNetworking", true);
    if (!self)
        return {};
    return self->GetISteamNetworking(a0, a1, a2);
}

ISteamRemoteStorage* SteamAPI_ISteamClient_GetISteamRemoteStorage(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamRemoteStorage", true);
    if (!self)
        return {};
    return self->GetISteamRemoteStorage(a0, a1, a2);
}

ISteamScreenshots* SteamAPI_ISteamClient_GetISteamScreenshots(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamScreenshots", true);
    if (!self)
        return {};
    return self->GetISteamScreenshots(a0, a1, a2);
}

void* SteamAPI_ISteamClient_GetISteamGameSearch(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamGameSearch", true);
    if (!self)
        return {};
    return self->GetISteamGameSearch(a0, a1, a2);
}

void SteamAPI_ISteamClient_RunFrame(ISteamClient* self) {
    SF_FLAT("SteamAPI_ISteamClient_RunFrame", true);
    if (self)
        self->RunFrame();
}

uint32_t SteamAPI_ISteamClient_GetIPCCallCount(ISteamClient* self) {
    SF_FLAT("SteamAPI_ISteamClient_GetIPCCallCount", true);
    if (!self)
        return {};
    return self->GetIPCCallCount();
}

void SteamAPI_ISteamClient_SetWarningMessageHook(ISteamClient* self, void* a0) {
    SF_FLAT("SteamAPI_ISteamClient_SetWarningMessageHook", true);
    if (self)
        self->SetWarningMessageHook(a0);
}

bool SteamAPI_ISteamClient_BShutdownIfAllPipesClosed(ISteamClient* self) {
    SF_FLAT("SteamAPI_ISteamClient_BShutdownIfAllPipesClosed", true);
    if (!self)
        return {};
    return self->BShutdownIfAllPipesClosed();
}

ISteamHTTP* SteamAPI_ISteamClient_GetISteamHTTP(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamHTTP", true);
    if (!self)
        return {};
    return self->GetISteamHTTP(a0, a1, a2);
}

ISteamController* SteamAPI_ISteamClient_GetISteamController(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamController", true);
    if (!self)
        return {};
    return self->GetISteamController(a0, a1, a2);
}

ISteamUGC* SteamAPI_ISteamClient_GetISteamUGC(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamUGC", true);
    if (!self)
        return {};
    return self->GetISteamUGC(a0, a1, a2);
}

ISteamAppList* SteamAPI_ISteamClient_GetISteamAppList(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamAppList", true);
    if (!self)
        return {};
    return self->GetISteamAppList(a0, a1, a2);
}

ISteamMusic* SteamAPI_ISteamClient_GetISteamMusic(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamMusic", true);
    if (!self)
        return {};
    return self->GetISteamMusic(a0, a1, a2);
}

void* SteamAPI_ISteamClient_GetISteamMusicRemote(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamMusicRemote", true);
    if (!self)
        return {};
    return self->GetISteamMusicRemote(a0, a1, a2);
}

void* SteamAPI_ISteamClient_GetISteamHTMLSurface(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamHTMLSurface", true);
    if (!self)
        return {};
    return self->GetISteamHTMLSurface(a0, a1, a2);
}

void SteamAPI_ISteamClient_Set_SteamAPI_CPostAPIResultInProcess(ISteamClient* self, void* a0) {
    SF_FLAT("SteamAPI_ISteamClient_Set_SteamAPI_CPostAPIResultInProcess", true);
    if (self)
        self->Set_SteamAPI_CPostAPIResultInProcess(a0);
}

void SteamAPI_ISteamClient_Remove_SteamAPI_CPostAPIResultInProcess(ISteamClient* self, void* a0) {
    SF_FLAT("SteamAPI_ISteamClient_Remove_SteamAPI_CPostAPIResultInProcess", true);
    if (self)
        self->Remove_SteamAPI_CPostAPIResultInProcess(a0);
}

void SteamAPI_ISteamClient_Set_SteamAPI_CCheckCallbackRegisteredInProcess(ISteamClient* self, void* a0) {
    SF_FLAT("SteamAPI_ISteamClient_Set_SteamAPI_CCheckCallbackRegisteredInProcess", true);
    if (self)
        self->Set_SteamAPI_CCheckCallbackRegisteredInProcess(a0);
}

ISteamInventory* SteamAPI_ISteamClient_GetISteamInventory(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamInventory", true);
    if (!self)
        return {};
    return self->GetISteamInventory(a0, a1, a2);
}

ISteamVideo* SteamAPI_ISteamClient_GetISteamVideo(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamVideo", true);
    if (!self)
        return {};
    return self->GetISteamVideo(a0, a1, a2);
}

ISteamParentalSettings* SteamAPI_ISteamClient_GetISteamParentalSettings(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamParentalSettings", true);
    if (!self)
        return {};
    return self->GetISteamParentalSettings(a0, a1, a2);
}

ISteamInput* SteamAPI_ISteamClient_GetISteamInput(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamInput", true);
    if (!self)
        return {};
    return self->GetISteamInput(a0, a1, a2);
}

void* SteamAPI_ISteamClient_GetISteamParties(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamParties", true);
    if (!self)
        return {};
    return self->GetISteamParties(a0, a1, a2);
}

void* SteamAPI_ISteamClient_GetISteamRemotePlay(ISteamClient* self, HSteamUser a0, HSteamPipe a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamClient_GetISteamRemotePlay", true);
    if (!self)
        return {};
    return self->GetISteamRemotePlay(a0, a1, a2);
}

HSteamUser SteamAPI_ISteamUser_GetHSteamUser(ISteamUser* self) {
    SF_FLAT("SteamAPI_ISteamUser_GetHSteamUser", true);
    if (!self)
        return {};
    return self->GetHSteamUser();
}

bool SteamAPI_ISteamUser_BLoggedOn(ISteamUser* self) {
    SF_FLAT("SteamAPI_ISteamUser_BLoggedOn", true);
    if (!self)
        return {};
    return self->BLoggedOn();
}

CSteamID SteamAPI_ISteamUser_GetSteamID(ISteamUser* self) {
    SF_FLAT("SteamAPI_ISteamUser_GetSteamID", true);
    if (!self)
        return {};
    return self->GetSteamID();
}

int SteamAPI_ISteamUser_InitiateGameConnection_DEPRECATED(ISteamUser* self, void* a0, int a1, CSteamID a2, uint32_t a3, uint16_t a4, bool a5) {
    SF_FLAT("SteamAPI_ISteamUser_InitiateGameConnection_DEPRECATED", true);
    if (!self)
        return {};
    return self->InitiateGameConnection_DEPRECATED(a0, a1, a2, a3, a4, a5);
}

void SteamAPI_ISteamUser_TerminateGameConnection_DEPRECATED(ISteamUser* self, uint32_t a0, uint16_t a1) {
    SF_FLAT("SteamAPI_ISteamUser_TerminateGameConnection_DEPRECATED", true);
    if (self)
        self->TerminateGameConnection_DEPRECATED(a0, a1);
}

void SteamAPI_ISteamUser_TrackAppUsageEvent(ISteamUser* self, CGameID a0, int a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamUser_TrackAppUsageEvent", true);
    if (self)
        self->TrackAppUsageEvent(a0, a1, a2);
}

bool SteamAPI_ISteamUser_GetUserDataFolder(ISteamUser* self, char* a0, int a1) {
    SF_FLAT("SteamAPI_ISteamUser_GetUserDataFolder", true);
    if (!self)
        return {};
    return self->GetUserDataFolder(a0, a1);
}

void SteamAPI_ISteamUser_StartVoiceRecording(ISteamUser* self) {
    SF_FLAT("SteamAPI_ISteamUser_StartVoiceRecording", true);
    if (self)
        self->StartVoiceRecording();
}

void SteamAPI_ISteamUser_StopVoiceRecording(ISteamUser* self) {
    SF_FLAT("SteamAPI_ISteamUser_StopVoiceRecording", true);
    if (self)
        self->StopVoiceRecording();
}

EVoiceResult SteamAPI_ISteamUser_GetAvailableVoice(ISteamUser* self, uint32_t* a0, uint32_t* a1, uint32_t a2) {
    SF_FLAT("SteamAPI_ISteamUser_GetAvailableVoice", true);
    if (!self)
        return {};
    return self->GetAvailableVoice(a0, a1, a2);
}

EVoiceResult SteamAPI_ISteamUser_GetVoice(ISteamUser* self, bool a0, void* a1, uint32_t a2, uint32_t* a3, bool a4, void* a5, uint32_t a6, uint32_t* a7, uint32_t a8) {
    SF_FLAT("SteamAPI_ISteamUser_GetVoice", true);
    if (!self)
        return {};
    return self->GetVoice(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

EVoiceResult SteamAPI_ISteamUser_DecompressVoice(ISteamUser* self, const void* a0, uint32_t a1, void* a2, uint32_t a3, uint32_t* a4, uint32_t a5) {
    SF_FLAT("SteamAPI_ISteamUser_DecompressVoice", true);
    if (!self)
        return {};
    return self->DecompressVoice(a0, a1, a2, a3, a4, a5);
}

uint32_t SteamAPI_ISteamUser_GetVoiceOptimalSampleRate(ISteamUser* self) {
    SF_FLAT("SteamAPI_ISteamUser_GetVoiceOptimalSampleRate", true);
    if (!self)
        return {};
    return self->GetVoiceOptimalSampleRate();
}

HAuthTicket SteamAPI_ISteamUser_GetAuthSessionTicket(ISteamUser* self, void* a0, int a1, uint32_t* a2) {
    SF_FLAT("SteamAPI_ISteamUser_GetAuthSessionTicket", true);
    if (!self)
        return {};
    return self->GetAuthSessionTicket(a0, a1, a2);
}

EBeginAuthSessionResult SteamAPI_ISteamUser_BeginAuthSession(ISteamUser* self, const void* a0, int a1, CSteamID a2) {
    SF_FLAT("SteamAPI_ISteamUser_BeginAuthSession", true);
    if (!self)
        return {};
    return self->BeginAuthSession(a0, a1, a2);
}

void SteamAPI_ISteamUser_EndAuthSession(ISteamUser* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamUser_EndAuthSession", true);
    if (self)
        self->EndAuthSession(a0);
}

void SteamAPI_ISteamUser_CancelAuthTicket(ISteamUser* self, HAuthTicket a0) {
    SF_FLAT("SteamAPI_ISteamUser_CancelAuthTicket", true);
    if (self)
        self->CancelAuthTicket(a0);
}

EUserHasLicenseForAppResult SteamAPI_ISteamUser_UserHasLicenseForApp(ISteamUser* self, CSteamID a0, AppId_t a1) {
    SF_FLAT("SteamAPI_ISteamUser_UserHasLicenseForApp", true);
    if (!self)
        return {};
    return self->UserHasLicenseForApp(a0, a1);
}

bool SteamAPI_ISteamUser_BIsBehindNAT(ISteamUser* self) {
    SF_FLAT("SteamAPI_ISteamUser_BIsBehindNAT", true);
    if (!self)
        return {};
    return self->BIsBehindNAT();
}

void SteamAPI_ISteamUser_AdvertiseGame(ISteamUser* self, CSteamID a0, uint32_t a1, uint16_t a2) {
    SF_FLAT("SteamAPI_ISteamUser_AdvertiseGame", true);
    if (self)
        self->AdvertiseGame(a0, a1, a2);
}

SteamAPICall_t SteamAPI_ISteamUser_RequestEncryptedAppTicket(ISteamUser* self, void* a0, int a1) {
    SF_FLAT("SteamAPI_ISteamUser_RequestEncryptedAppTicket", true);
    if (!self)
        return {};
    return self->RequestEncryptedAppTicket(a0, a1);
}

bool SteamAPI_ISteamUser_GetEncryptedAppTicket(ISteamUser* self, void* a0, int a1, uint32_t* a2) {
    SF_FLAT("SteamAPI_ISteamUser_GetEncryptedAppTicket", true);
    if (!self)
        return {};
    return self->GetEncryptedAppTicket(a0, a1, a2);
}

int SteamAPI_ISteamUser_GetGameBadgeLevel(ISteamUser* self, int a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamUser_GetGameBadgeLevel", true);
    if (!self)
        return {};
    return self->GetGameBadgeLevel(a0, a1);
}

int SteamAPI_ISteamUser_GetPlayerSteamLevel(ISteamUser* self) {
    SF_FLAT("SteamAPI_ISteamUser_GetPlayerSteamLevel", true);
    if (!self)
        return {};
    return self->GetPlayerSteamLevel();
}

SteamAPICall_t SteamAPI_ISteamUser_RequestStoreAuthURL(ISteamUser* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamUser_RequestStoreAuthURL", true);
    if (!self)
        return {};
    return self->RequestStoreAuthURL(a0);
}

bool SteamAPI_ISteamUser_BIsPhoneVerified(ISteamUser* self) {
    SF_FLAT("SteamAPI_ISteamUser_BIsPhoneVerified", true);
    if (!self)
        return {};
    return self->BIsPhoneVerified();
}

bool SteamAPI_ISteamUser_BIsTwoFactorEnabled(ISteamUser* self) {
    SF_FLAT("SteamAPI_ISteamUser_BIsTwoFactorEnabled", true);
    if (!self)
        return {};
    return self->BIsTwoFactorEnabled();
}

bool SteamAPI_ISteamUser_BIsPhoneIdentifying(ISteamUser* self) {
    SF_FLAT("SteamAPI_ISteamUser_BIsPhoneIdentifying", true);
    if (!self)
        return {};
    return self->BIsPhoneIdentifying();
}

bool SteamAPI_ISteamUser_BIsPhoneRequiringVerification(ISteamUser* self) {
    SF_FLAT("SteamAPI_ISteamUser_BIsPhoneRequiringVerification", true);
    if (!self)
        return {};
    return self->BIsPhoneRequiringVerification();
}

int SteamAPI_ISteamUser_GetMarketEligibility(ISteamUser* self) {
    SF_FLAT("SteamAPI_ISteamUser_GetMarketEligibility", true);
    if (!self)
        return {};
    return self->GetMarketEligibility();
}

SteamAPICall_t SteamAPI_ISteamUser_GetDurationControl(ISteamUser* self) {
    SF_FLAT("SteamAPI_ISteamUser_GetDurationControl", true);
    if (!self)
        return {};
    return self->GetDurationControl();
}

bool SteamAPI_ISteamUser_BSetDurationControlOnlineState(ISteamUser* self, int a0) {
    SF_FLAT("SteamAPI_ISteamUser_BSetDurationControlOnlineState", true);
    if (!self)
        return {};
    return self->BSetDurationControlOnlineState(a0);
}

HAuthTicket SteamAPI_ISteamUser_GetAuthTicketForWebApi(ISteamUser* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamUser_GetAuthTicketForWebApi", true);
    if (!self)
        return {};
    return self->GetAuthTicketForWebApi(a0);
}

const char* SteamAPI_ISteamFriends_GetPersonaName(ISteamFriends* self) {
    SF_FLAT("SteamAPI_ISteamFriends_GetPersonaName", true);
    if (!self)
        return {};
    return self->GetPersonaName();
}

SteamAPICall_t SteamAPI_ISteamFriends_SetPersonaName(ISteamFriends* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamFriends_SetPersonaName", true);
    if (!self)
        return {};
    return self->SetPersonaName(a0);
}

EPersonaState SteamAPI_ISteamFriends_GetPersonaState(ISteamFriends* self) {
    SF_FLAT("SteamAPI_ISteamFriends_GetPersonaState", true);
    if (!self)
        return {};
    return self->GetPersonaState();
}

int SteamAPI_ISteamFriends_GetFriendCount(ISteamFriends* self, int a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendCount", true);
    if (!self)
        return {};
    return self->GetFriendCount(a0);
}

CSteamID SteamAPI_ISteamFriends_GetFriendByIndex(ISteamFriends* self, int a0, int a1) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendByIndex", true);
    if (!self)
        return {};
    return self->GetFriendByIndex(a0, a1);
}

EFriendRelationship SteamAPI_ISteamFriends_GetFriendRelationship(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendRelationship", true);
    if (!self)
        return {};
    return self->GetFriendRelationship(a0);
}

EPersonaState SteamAPI_ISteamFriends_GetFriendPersonaState(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendPersonaState", true);
    if (!self)
        return {};
    return self->GetFriendPersonaState(a0);
}

const char* SteamAPI_ISteamFriends_GetFriendPersonaName(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendPersonaName", true);
    if (!self)
        return {};
    return self->GetFriendPersonaName(a0);
}

bool SteamAPI_ISteamFriends_GetFriendGamePlayed(ISteamFriends* self, CSteamID a0, FriendGameInfo_t* a1) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendGamePlayed", true);
    if (!self)
        return {};
    return self->GetFriendGamePlayed(a0, a1);
}

const char* SteamAPI_ISteamFriends_GetFriendPersonaNameHistory(ISteamFriends* self, CSteamID a0, int a1) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendPersonaNameHistory", true);
    if (!self)
        return {};
    return self->GetFriendPersonaNameHistory(a0, a1);
}

int SteamAPI_ISteamFriends_GetFriendSteamLevel(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendSteamLevel", true);
    if (!self)
        return {};
    return self->GetFriendSteamLevel(a0);
}

const char* SteamAPI_ISteamFriends_GetPlayerNickname(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetPlayerNickname", true);
    if (!self)
        return {};
    return self->GetPlayerNickname(a0);
}

int SteamAPI_ISteamFriends_GetFriendsGroupCount(ISteamFriends* self) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendsGroupCount", true);
    if (!self)
        return {};
    return self->GetFriendsGroupCount();
}

FriendsGroupID_t SteamAPI_ISteamFriends_GetFriendsGroupIDByIndex(ISteamFriends* self, int a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendsGroupIDByIndex", true);
    if (!self)
        return {};
    return self->GetFriendsGroupIDByIndex(a0);
}

const char* SteamAPI_ISteamFriends_GetFriendsGroupName(ISteamFriends* self, FriendsGroupID_t a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendsGroupName", true);
    if (!self)
        return {};
    return self->GetFriendsGroupName(a0);
}

int SteamAPI_ISteamFriends_GetFriendsGroupMembersCount(ISteamFriends* self, FriendsGroupID_t a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendsGroupMembersCount", true);
    if (!self)
        return {};
    return self->GetFriendsGroupMembersCount(a0);
}

void SteamAPI_ISteamFriends_GetFriendsGroupMembersList(ISteamFriends* self, FriendsGroupID_t a0, CSteamID* a1, int a2) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendsGroupMembersList", true);
    if (self)
        self->GetFriendsGroupMembersList(a0, a1, a2);
}

bool SteamAPI_ISteamFriends_HasFriend(ISteamFriends* self, CSteamID a0, int a1) {
    SF_FLAT("SteamAPI_ISteamFriends_HasFriend", true);
    if (!self)
        return {};
    return self->HasFriend(a0, a1);
}

int SteamAPI_ISteamFriends_GetClanCount(ISteamFriends* self) {
    SF_FLAT("SteamAPI_ISteamFriends_GetClanCount", true);
    if (!self)
        return {};
    return self->GetClanCount();
}

CSteamID SteamAPI_ISteamFriends_GetClanByIndex(ISteamFriends* self, int a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetClanByIndex", true);
    if (!self)
        return {};
    return self->GetClanByIndex(a0);
}

const char* SteamAPI_ISteamFriends_GetClanName(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetClanName", true);
    if (!self)
        return {};
    return self->GetClanName(a0);
}

const char* SteamAPI_ISteamFriends_GetClanTag(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetClanTag", true);
    if (!self)
        return {};
    return self->GetClanTag(a0);
}

bool SteamAPI_ISteamFriends_GetClanActivityCounts(ISteamFriends* self, CSteamID a0, int* a1, int* a2, int* a3) {
    SF_FLAT("SteamAPI_ISteamFriends_GetClanActivityCounts", true);
    if (!self)
        return {};
    return self->GetClanActivityCounts(a0, a1, a2, a3);
}

SteamAPICall_t SteamAPI_ISteamFriends_DownloadClanActivityCounts(ISteamFriends* self, CSteamID* a0, int a1) {
    SF_FLAT("SteamAPI_ISteamFriends_DownloadClanActivityCounts", true);
    if (!self)
        return {};
    return self->DownloadClanActivityCounts(a0, a1);
}

int SteamAPI_ISteamFriends_GetFriendCountFromSource(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendCountFromSource", true);
    if (!self)
        return {};
    return self->GetFriendCountFromSource(a0);
}

CSteamID SteamAPI_ISteamFriends_GetFriendFromSourceByIndex(ISteamFriends* self, CSteamID a0, int a1) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendFromSourceByIndex", true);
    if (!self)
        return {};
    return self->GetFriendFromSourceByIndex(a0, a1);
}

bool SteamAPI_ISteamFriends_IsUserInSource(ISteamFriends* self, CSteamID a0, CSteamID a1) {
    SF_FLAT("SteamAPI_ISteamFriends_IsUserInSource", true);
    if (!self)
        return {};
    return self->IsUserInSource(a0, a1);
}

void SteamAPI_ISteamFriends_SetInGameVoiceSpeaking(ISteamFriends* self, CSteamID a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamFriends_SetInGameVoiceSpeaking", true);
    if (self)
        self->SetInGameVoiceSpeaking(a0, a1);
}

void SteamAPI_ISteamFriends_ActivateGameOverlay(ISteamFriends* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamFriends_ActivateGameOverlay", true);
    if (self)
        self->ActivateGameOverlay(a0);
}

void SteamAPI_ISteamFriends_ActivateGameOverlayToUser(ISteamFriends* self, const char* a0, CSteamID a1) {
    SF_FLAT("SteamAPI_ISteamFriends_ActivateGameOverlayToUser", true);
    if (self)
        self->ActivateGameOverlayToUser(a0, a1);
}

void SteamAPI_ISteamFriends_ActivateGameOverlayToWebPage(ISteamFriends* self, const char* a0, int a1) {
    SF_FLAT("SteamAPI_ISteamFriends_ActivateGameOverlayToWebPage", true);
    if (self)
        self->ActivateGameOverlayToWebPage(a0, a1);
}

void SteamAPI_ISteamFriends_ActivateGameOverlayToStore(ISteamFriends* self, AppId_t a0, int a1) {
    SF_FLAT("SteamAPI_ISteamFriends_ActivateGameOverlayToStore", true);
    if (self)
        self->ActivateGameOverlayToStore(a0, a1);
}

void SteamAPI_ISteamFriends_SetPlayedWith(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_SetPlayedWith", true);
    if (self)
        self->SetPlayedWith(a0);
}

void SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialog(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialog", true);
    if (self)
        self->ActivateGameOverlayInviteDialog(a0);
}

int SteamAPI_ISteamFriends_GetSmallFriendAvatar(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetSmallFriendAvatar", true);
    if (!self)
        return {};
    return self->GetSmallFriendAvatar(a0);
}

int SteamAPI_ISteamFriends_GetMediumFriendAvatar(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetMediumFriendAvatar", true);
    if (!self)
        return {};
    return self->GetMediumFriendAvatar(a0);
}

int SteamAPI_ISteamFriends_GetLargeFriendAvatar(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetLargeFriendAvatar", true);
    if (!self)
        return {};
    return self->GetLargeFriendAvatar(a0);
}

bool SteamAPI_ISteamFriends_RequestUserInformation(ISteamFriends* self, CSteamID a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamFriends_RequestUserInformation", true);
    if (!self)
        return {};
    return self->RequestUserInformation(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamFriends_RequestClanOfficerList(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_RequestClanOfficerList", true);
    if (!self)
        return {};
    return self->RequestClanOfficerList(a0);
}

CSteamID SteamAPI_ISteamFriends_GetClanOwner(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetClanOwner", true);
    if (!self)
        return {};
    return self->GetClanOwner(a0);
}

int SteamAPI_ISteamFriends_GetClanOfficerCount(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetClanOfficerCount", true);
    if (!self)
        return {};
    return self->GetClanOfficerCount(a0);
}

CSteamID SteamAPI_ISteamFriends_GetClanOfficerByIndex(ISteamFriends* self, CSteamID a0, int a1) {
    SF_FLAT("SteamAPI_ISteamFriends_GetClanOfficerByIndex", true);
    if (!self)
        return {};
    return self->GetClanOfficerByIndex(a0, a1);
}

uint32_t SteamAPI_ISteamFriends_GetUserRestrictions(ISteamFriends* self) {
    SF_FLAT("SteamAPI_ISteamFriends_GetUserRestrictions", true);
    if (!self)
        return {};
    return self->GetUserRestrictions();
}

bool SteamAPI_ISteamFriends_SetRichPresence(ISteamFriends* self, const char* a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamFriends_SetRichPresence", true);
    if (!self)
        return {};
    return self->SetRichPresence(a0, a1);
}

void SteamAPI_ISteamFriends_ClearRichPresence(ISteamFriends* self) {
    SF_FLAT("SteamAPI_ISteamFriends_ClearRichPresence", true);
    if (self)
        self->ClearRichPresence();
}

const char* SteamAPI_ISteamFriends_GetFriendRichPresence(ISteamFriends* self, CSteamID a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendRichPresence", true);
    if (!self)
        return {};
    return self->GetFriendRichPresence(a0, a1);
}

int SteamAPI_ISteamFriends_GetFriendRichPresenceKeyCount(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendRichPresenceKeyCount", true);
    if (!self)
        return {};
    return self->GetFriendRichPresenceKeyCount(a0);
}

const char* SteamAPI_ISteamFriends_GetFriendRichPresenceKeyByIndex(ISteamFriends* self, CSteamID a0, int a1) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendRichPresenceKeyByIndex", true);
    if (!self)
        return {};
    return self->GetFriendRichPresenceKeyByIndex(a0, a1);
}

void SteamAPI_ISteamFriends_RequestFriendRichPresence(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_RequestFriendRichPresence", true);
    if (self)
        self->RequestFriendRichPresence(a0);
}

bool SteamAPI_ISteamFriends_InviteUserToGame(ISteamFriends* self, CSteamID a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamFriends_InviteUserToGame", true);
    if (!self)
        return {};
    return self->InviteUserToGame(a0, a1);
}

int SteamAPI_ISteamFriends_GetCoplayFriendCount(ISteamFriends* self) {
    SF_FLAT("SteamAPI_ISteamFriends_GetCoplayFriendCount", true);
    if (!self)
        return {};
    return self->GetCoplayFriendCount();
}

CSteamID SteamAPI_ISteamFriends_GetCoplayFriend(ISteamFriends* self, int a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetCoplayFriend", true);
    if (!self)
        return {};
    return self->GetCoplayFriend(a0);
}

int SteamAPI_ISteamFriends_GetFriendCoplayTime(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendCoplayTime", true);
    if (!self)
        return {};
    return self->GetFriendCoplayTime(a0);
}

AppId_t SteamAPI_ISteamFriends_GetFriendCoplayGame(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendCoplayGame", true);
    if (!self)
        return {};
    return self->GetFriendCoplayGame(a0);
}

SteamAPICall_t SteamAPI_ISteamFriends_JoinClanChatRoom(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_JoinClanChatRoom", true);
    if (!self)
        return {};
    return self->JoinClanChatRoom(a0);
}

bool SteamAPI_ISteamFriends_LeaveClanChatRoom(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_LeaveClanChatRoom", true);
    if (!self)
        return {};
    return self->LeaveClanChatRoom(a0);
}

int SteamAPI_ISteamFriends_GetClanChatMemberCount(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetClanChatMemberCount", true);
    if (!self)
        return {};
    return self->GetClanChatMemberCount(a0);
}

CSteamID SteamAPI_ISteamFriends_GetChatMemberByIndex(ISteamFriends* self, CSteamID a0, int a1) {
    SF_FLAT("SteamAPI_ISteamFriends_GetChatMemberByIndex", true);
    if (!self)
        return {};
    return self->GetChatMemberByIndex(a0, a1);
}

bool SteamAPI_ISteamFriends_SendClanChatMessage(ISteamFriends* self, CSteamID a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamFriends_SendClanChatMessage", true);
    if (!self)
        return {};
    return self->SendClanChatMessage(a0, a1);
}

int SteamAPI_ISteamFriends_GetClanChatMessage(ISteamFriends* self, CSteamID a0, int a1, void* a2, int a3, int* a4, CSteamID* a5) {
    SF_FLAT("SteamAPI_ISteamFriends_GetClanChatMessage", true);
    if (!self)
        return {};
    return self->GetClanChatMessage(a0, a1, a2, a3, a4, a5);
}

bool SteamAPI_ISteamFriends_IsClanChatAdmin(ISteamFriends* self, CSteamID a0, CSteamID a1) {
    SF_FLAT("SteamAPI_ISteamFriends_IsClanChatAdmin", true);
    if (!self)
        return {};
    return self->IsClanChatAdmin(a0, a1);
}

bool SteamAPI_ISteamFriends_IsClanChatWindowOpenInSteam(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_IsClanChatWindowOpenInSteam", true);
    if (!self)
        return {};
    return self->IsClanChatWindowOpenInSteam(a0);
}

bool SteamAPI_ISteamFriends_OpenClanChatWindowInSteam(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_OpenClanChatWindowInSteam", true);
    if (!self)
        return {};
    return self->OpenClanChatWindowInSteam(a0);
}

bool SteamAPI_ISteamFriends_CloseClanChatWindowInSteam(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_CloseClanChatWindowInSteam", true);
    if (!self)
        return {};
    return self->CloseClanChatWindowInSteam(a0);
}

bool SteamAPI_ISteamFriends_SetListenForFriendsMessages(ISteamFriends* self, bool a0) {
    SF_FLAT("SteamAPI_ISteamFriends_SetListenForFriendsMessages", true);
    if (!self)
        return {};
    return self->SetListenForFriendsMessages(a0);
}

bool SteamAPI_ISteamFriends_ReplyToFriendMessage(ISteamFriends* self, CSteamID a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamFriends_ReplyToFriendMessage", true);
    if (!self)
        return {};
    return self->ReplyToFriendMessage(a0, a1);
}

int SteamAPI_ISteamFriends_GetFriendMessage(ISteamFriends* self, CSteamID a0, int a1, void* a2, int a3, int* a4) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFriendMessage", true);
    if (!self)
        return {};
    return self->GetFriendMessage(a0, a1, a2, a3, a4);
}

SteamAPICall_t SteamAPI_ISteamFriends_GetFollowerCount(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_GetFollowerCount", true);
    if (!self)
        return {};
    return self->GetFollowerCount(a0);
}

SteamAPICall_t SteamAPI_ISteamFriends_IsFollowing(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_IsFollowing", true);
    if (!self)
        return {};
    return self->IsFollowing(a0);
}

SteamAPICall_t SteamAPI_ISteamFriends_EnumerateFollowingList(ISteamFriends* self, uint32_t a0) {
    SF_FLAT("SteamAPI_ISteamFriends_EnumerateFollowingList", true);
    if (!self)
        return {};
    return self->EnumerateFollowingList(a0);
}

bool SteamAPI_ISteamFriends_IsClanPublic(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_IsClanPublic", true);
    if (!self)
        return {};
    return self->IsClanPublic(a0);
}

bool SteamAPI_ISteamFriends_IsClanOfficialGameGroup(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_IsClanOfficialGameGroup", true);
    if (!self)
        return {};
    return self->IsClanOfficialGameGroup(a0);
}

int SteamAPI_ISteamFriends_GetNumChatsWithUnreadPriorityMessages(ISteamFriends* self) {
    SF_FLAT("SteamAPI_ISteamFriends_GetNumChatsWithUnreadPriorityMessages", true);
    if (!self)
        return {};
    return self->GetNumChatsWithUnreadPriorityMessages();
}

void SteamAPI_ISteamFriends_ActivateGameOverlayRemotePlayTogetherInviteDialog(ISteamFriends* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamFriends_ActivateGameOverlayRemotePlayTogetherInviteDialog", true);
    if (self)
        self->ActivateGameOverlayRemotePlayTogetherInviteDialog(a0);
}

uint32_t SteamAPI_ISteamUtils_GetSecondsSinceAppActive(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_GetSecondsSinceAppActive", true);
    if (!self)
        return {};
    return self->GetSecondsSinceAppActive();
}

uint32_t SteamAPI_ISteamUtils_GetSecondsSinceComputerActive(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_GetSecondsSinceComputerActive", true);
    if (!self)
        return {};
    return self->GetSecondsSinceComputerActive();
}

EUniverse SteamAPI_ISteamUtils_GetConnectedUniverse(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_GetConnectedUniverse", true);
    if (!self)
        return {};
    return self->GetConnectedUniverse();
}

uint32_t SteamAPI_ISteamUtils_GetServerRealTime(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_GetServerRealTime", true);
    if (!self)
        return {};
    return self->GetServerRealTime();
}

const char* SteamAPI_ISteamUtils_GetIPCountry(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_GetIPCountry", true);
    if (!self)
        return {};
    return self->GetIPCountry();
}

bool SteamAPI_ISteamUtils_GetImageSize(ISteamUtils* self, int a0, uint32_t* a1, uint32_t* a2) {
    SF_FLAT("SteamAPI_ISteamUtils_GetImageSize", true);
    if (!self)
        return {};
    return self->GetImageSize(a0, a1, a2);
}

bool SteamAPI_ISteamUtils_GetImageRGBA(ISteamUtils* self, int a0, uint8_t* a1, int a2) {
    SF_FLAT("SteamAPI_ISteamUtils_GetImageRGBA", true);
    if (!self)
        return {};
    return self->GetImageRGBA(a0, a1, a2);
}

bool SteamAPI_ISteamUtils_GetCSERIPPort(ISteamUtils* self, uint32_t* a0, uint16_t* a1) {
    SF_FLAT("SteamAPI_ISteamUtils_GetCSERIPPort", true);
    if (!self)
        return {};
    return self->GetCSERIPPort(a0, a1);
}

uint8_t SteamAPI_ISteamUtils_GetCurrentBatteryPower(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_GetCurrentBatteryPower", true);
    if (!self)
        return {};
    return self->GetCurrentBatteryPower();
}

AppId_t SteamAPI_ISteamUtils_GetAppID(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_GetAppID", true);
    if (!self)
        return {};
    return self->GetAppID();
}

void SteamAPI_ISteamUtils_SetOverlayNotificationPosition(ISteamUtils* self, int a0) {
    SF_FLAT("SteamAPI_ISteamUtils_SetOverlayNotificationPosition", true);
    if (self)
        self->SetOverlayNotificationPosition(a0);
}

bool SteamAPI_ISteamUtils_IsAPICallCompleted(ISteamUtils* self, SteamAPICall_t a0, bool* a1) {
    SF_FLAT("SteamAPI_ISteamUtils_IsAPICallCompleted", true);
    if (!self)
        return {};
    return self->IsAPICallCompleted(a0, a1);
}

int SteamAPI_ISteamUtils_GetAPICallFailureReason(ISteamUtils* self, SteamAPICall_t a0) {
    SF_FLAT("SteamAPI_ISteamUtils_GetAPICallFailureReason", true);
    if (!self)
        return {};
    return self->GetAPICallFailureReason(a0);
}

bool SteamAPI_ISteamUtils_GetAPICallResult(ISteamUtils* self, SteamAPICall_t a0, void* a1, int a2, int a3, bool* a4) {
    SF_FLAT("SteamAPI_ISteamUtils_GetAPICallResult", true);
    if (!self)
        return {};
    return self->GetAPICallResult(a0, a1, a2, a3, a4);
}

uint32_t SteamAPI_ISteamUtils_GetIPCCallCount(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_GetIPCCallCount", true);
    if (!self)
        return {};
    return self->GetIPCCallCount();
}

void SteamAPI_ISteamUtils_SetWarningMessageHook(ISteamUtils* self, void* a0) {
    SF_FLAT("SteamAPI_ISteamUtils_SetWarningMessageHook", true);
    if (self)
        self->SetWarningMessageHook(a0);
}

bool SteamAPI_ISteamUtils_IsOverlayEnabled(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_IsOverlayEnabled", true);
    if (!self)
        return {};
    return self->IsOverlayEnabled();
}

bool SteamAPI_ISteamUtils_BOverlayNeedsPresent(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_BOverlayNeedsPresent", true);
    if (!self)
        return {};
    return self->BOverlayNeedsPresent();
}

SteamAPICall_t SteamAPI_ISteamUtils_CheckFileSignature(ISteamUtils* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamUtils_CheckFileSignature", true);
    if (!self)
        return {};
    return self->CheckFileSignature(a0);
}

bool SteamAPI_ISteamUtils_ShowGamepadTextInput(ISteamUtils* self, int a0, int a1, const char* a2, uint32_t a3, const char* a4) {
    SF_FLAT("SteamAPI_ISteamUtils_ShowGamepadTextInput", true);
    if (!self)
        return {};
    return self->ShowGamepadTextInput(a0, a1, a2, a3, a4);
}

uint32_t SteamAPI_ISteamUtils_GetEnteredGamepadTextLength(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_GetEnteredGamepadTextLength", true);
    if (!self)
        return {};
    return self->GetEnteredGamepadTextLength();
}

bool SteamAPI_ISteamUtils_GetEnteredGamepadTextInput(ISteamUtils* self, char* a0, uint32_t a1) {
    SF_FLAT("SteamAPI_ISteamUtils_GetEnteredGamepadTextInput", true);
    if (!self)
        return {};
    return self->GetEnteredGamepadTextInput(a0, a1);
}

const char* SteamAPI_ISteamUtils_GetSteamUILanguage(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_GetSteamUILanguage", true);
    if (!self)
        return {};
    return self->GetSteamUILanguage();
}

bool SteamAPI_ISteamUtils_IsSteamRunningInVR(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_IsSteamRunningInVR", true);
    if (!self)
        return {};
    return self->IsSteamRunningInVR();
}

void SteamAPI_ISteamUtils_SetOverlayNotificationInset(ISteamUtils* self, int a0, int a1) {
    SF_FLAT("SteamAPI_ISteamUtils_SetOverlayNotificationInset", true);
    if (self)
        self->SetOverlayNotificationInset(a0, a1);
}

bool SteamAPI_ISteamUtils_IsSteamInBigPictureMode(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_IsSteamInBigPictureMode", true);
    if (!self)
        return {};
    return self->IsSteamInBigPictureMode();
}

void SteamAPI_ISteamUtils_StartVrDashboard(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_StartVrDashboard", true);
    if (self)
        self->StartVrDashboard();
}

bool SteamAPI_ISteamUtils_IsVRHeadsetStreamingEnabled(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_IsVRHeadsetStreamingEnabled", true);
    if (!self)
        return {};
    return self->IsVRHeadsetStreamingEnabled();
}

void SteamAPI_ISteamUtils_SetVRHeadsetStreamingEnabled(ISteamUtils* self, bool a0) {
    SF_FLAT("SteamAPI_ISteamUtils_SetVRHeadsetStreamingEnabled", true);
    if (self)
        self->SetVRHeadsetStreamingEnabled(a0);
}

bool SteamAPI_ISteamUtils_IsSteamChinaLauncher(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_IsSteamChinaLauncher", true);
    if (!self)
        return {};
    return self->IsSteamChinaLauncher();
}

bool SteamAPI_ISteamUtils_InitFilterText(ISteamUtils* self, uint32_t a0) {
    SF_FLAT("SteamAPI_ISteamUtils_InitFilterText", true);
    if (!self)
        return {};
    return self->InitFilterText(a0);
}

int SteamAPI_ISteamUtils_FilterText(ISteamUtils* self, int a0, CSteamID a1, const char* a2, char* a3, uint32_t a4) {
    SF_FLAT("SteamAPI_ISteamUtils_FilterText", true);
    if (!self)
        return {};
    return self->FilterText(a0, a1, a2, a3, a4);
}

int SteamAPI_ISteamUtils_GetIPv6ConnectivityState(ISteamUtils* self, int a0) {
    SF_FLAT("SteamAPI_ISteamUtils_GetIPv6ConnectivityState", true);
    if (!self)
        return {};
    return self->GetIPv6ConnectivityState(a0);
}

bool SteamAPI_ISteamUtils_IsSteamRunningOnSteamDeck(ISteamUtils* self) {
    SF_FLAT("SteamAPI_ISteamUtils_IsSteamRunningOnSteamDeck", true);
    if (!self)
        return {};
    return self->IsSteamRunningOnSteamDeck();
}

bool SteamAPI_ISteamUserStats_RequestCurrentStats(ISteamUserStats* self) {
    SF_FLAT("SteamAPI_ISteamUserStats_RequestCurrentStats", true);
    if (!self)
        return {};
    return self->RequestCurrentStats();
}

bool SteamAPI_ISteamUserStats_GetStatInt32(ISteamUserStats* self, const char* a0, int32_t* a1) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetStatInt32", true);
    if (!self)
        return {};
    return self->GetStat(a0, a1);
}

bool SteamAPI_ISteamUserStats_GetStatFloat(ISteamUserStats* self, const char* a0, float* a1) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetStatFloat", true);
    if (!self)
        return {};
    return self->GetStat(a0, a1);
}

bool SteamAPI_ISteamUserStats_SetStatInt32(ISteamUserStats* self, const char* a0, int32_t a1) {
    SF_FLAT("SteamAPI_ISteamUserStats_SetStatInt32", true);
    if (!self)
        return {};
    return self->SetStat(a0, a1);
}

bool SteamAPI_ISteamUserStats_SetStatFloat(ISteamUserStats* self, const char* a0, float a1) {
    SF_FLAT("SteamAPI_ISteamUserStats_SetStatFloat", true);
    if (!self)
        return {};
    return self->SetStat(a0, a1);
}

bool SteamAPI_ISteamUserStats_UpdateAvgRateStat(ISteamUserStats* self, const char* a0, float a1, double a2) {
    SF_FLAT("SteamAPI_ISteamUserStats_UpdateAvgRateStat", true);
    if (!self)
        return {};
    return self->UpdateAvgRateStat(a0, a1, a2);
}

bool SteamAPI_ISteamUserStats_GetAchievement(ISteamUserStats* self, const char* a0, bool* a1) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetAchievement", true);
    if (!self)
        return {};
    return self->GetAchievement(a0, a1);
}

bool SteamAPI_ISteamUserStats_SetAchievement(ISteamUserStats* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamUserStats_SetAchievement", true);
    if (!self)
        return {};
    return self->SetAchievement(a0);
}

bool SteamAPI_ISteamUserStats_ClearAchievement(ISteamUserStats* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamUserStats_ClearAchievement", true);
    if (!self)
        return {};
    return self->ClearAchievement(a0);
}

bool SteamAPI_ISteamUserStats_GetAchievementAndUnlockTime(ISteamUserStats* self, const char* a0, bool* a1, uint32_t* a2) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetAchievementAndUnlockTime", true);
    if (!self)
        return {};
    return self->GetAchievementAndUnlockTime(a0, a1, a2);
}

bool SteamAPI_ISteamUserStats_StoreStats(ISteamUserStats* self) {
    SF_FLAT("SteamAPI_ISteamUserStats_StoreStats", true);
    if (!self)
        return {};
    return self->StoreStats();
}

int SteamAPI_ISteamUserStats_GetAchievementIcon(ISteamUserStats* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetAchievementIcon", true);
    if (!self)
        return {};
    return self->GetAchievementIcon(a0);
}

const char* SteamAPI_ISteamUserStats_GetAchievementDisplayAttribute(ISteamUserStats* self, const char* a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetAchievementDisplayAttribute", true);
    if (!self)
        return {};
    return self->GetAchievementDisplayAttribute(a0, a1);
}

bool SteamAPI_ISteamUserStats_IndicateAchievementProgress(ISteamUserStats* self, const char* a0, uint32_t a1, uint32_t a2) {
    SF_FLAT("SteamAPI_ISteamUserStats_IndicateAchievementProgress", true);
    if (!self)
        return {};
    return self->IndicateAchievementProgress(a0, a1, a2);
}

uint32_t SteamAPI_ISteamUserStats_GetNumAchievements(ISteamUserStats* self) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetNumAchievements", true);
    if (!self)
        return {};
    return self->GetNumAchievements();
}

const char* SteamAPI_ISteamUserStats_GetAchievementName(ISteamUserStats* self, uint32_t a0) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetAchievementName", true);
    if (!self)
        return {};
    return self->GetAchievementName(a0);
}

SteamAPICall_t SteamAPI_ISteamUserStats_RequestUserStats(ISteamUserStats* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamUserStats_RequestUserStats", true);
    if (!self)
        return {};
    return self->RequestUserStats(a0);
}

bool SteamAPI_ISteamUserStats_GetUserStatInt32(ISteamUserStats* self, CSteamID a0, const char* a1, int32_t* a2) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetUserStatInt32", true);
    if (!self)
        return {};
    return self->GetUserStat(a0, a1, a2);
}

bool SteamAPI_ISteamUserStats_GetUserStatFloat(ISteamUserStats* self, CSteamID a0, const char* a1, float* a2) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetUserStatFloat", true);
    if (!self)
        return {};
    return self->GetUserStat(a0, a1, a2);
}

bool SteamAPI_ISteamUserStats_GetUserAchievement(ISteamUserStats* self, CSteamID a0, const char* a1, bool* a2) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetUserAchievement", true);
    if (!self)
        return {};
    return self->GetUserAchievement(a0, a1, a2);
}

bool SteamAPI_ISteamUserStats_GetUserAchievementAndUnlockTime(ISteamUserStats* self, CSteamID a0, const char* a1, bool* a2, uint32_t* a3) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetUserAchievementAndUnlockTime", true);
    if (!self)
        return {};
    return self->GetUserAchievementAndUnlockTime(a0, a1, a2, a3);
}

bool SteamAPI_ISteamUserStats_ResetAllStats(ISteamUserStats* self, bool a0) {
    SF_FLAT("SteamAPI_ISteamUserStats_ResetAllStats", true);
    if (!self)
        return {};
    return self->ResetAllStats(a0);
}

SteamAPICall_t SteamAPI_ISteamUserStats_FindOrCreateLeaderboard(ISteamUserStats* self, const char* a0, int a1, int a2) {
    SF_FLAT("SteamAPI_ISteamUserStats_FindOrCreateLeaderboard", true);
    if (!self)
        return {};
    return self->FindOrCreateLeaderboard(a0, a1, a2);
}

SteamAPICall_t SteamAPI_ISteamUserStats_FindLeaderboard(ISteamUserStats* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamUserStats_FindLeaderboard", true);
    if (!self)
        return {};
    return self->FindLeaderboard(a0);
}

const char* SteamAPI_ISteamUserStats_GetLeaderboardName(ISteamUserStats* self, SteamLeaderboard_t a0) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetLeaderboardName", true);
    if (!self)
        return {};
    return self->GetLeaderboardName(a0);
}

int SteamAPI_ISteamUserStats_GetLeaderboardEntryCount(ISteamUserStats* self, SteamLeaderboard_t a0) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetLeaderboardEntryCount", true);
    if (!self)
        return {};
    return self->GetLeaderboardEntryCount(a0);
}

int SteamAPI_ISteamUserStats_GetLeaderboardSortMethod(ISteamUserStats* self, SteamLeaderboard_t a0) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetLeaderboardSortMethod", true);
    if (!self)
        return {};
    return self->GetLeaderboardSortMethod(a0);
}

int SteamAPI_ISteamUserStats_GetLeaderboardDisplayType(ISteamUserStats* self, SteamLeaderboard_t a0) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetLeaderboardDisplayType", true);
    if (!self)
        return {};
    return self->GetLeaderboardDisplayType(a0);
}

SteamAPICall_t SteamAPI_ISteamUserStats_DownloadLeaderboardEntries(ISteamUserStats* self, SteamLeaderboard_t a0, int a1, int a2, int a3) {
    SF_FLAT("SteamAPI_ISteamUserStats_DownloadLeaderboardEntries", true);
    if (!self)
        return {};
    return self->DownloadLeaderboardEntries(a0, a1, a2, a3);
}

SteamAPICall_t SteamAPI_ISteamUserStats_DownloadLeaderboardEntriesForUsers(ISteamUserStats* self, SteamLeaderboard_t a0, CSteamID* a1, int a2) {
    SF_FLAT("SteamAPI_ISteamUserStats_DownloadLeaderboardEntriesForUsers", true);
    if (!self)
        return {};
    return self->DownloadLeaderboardEntriesForUsers(a0, a1, a2);
}

bool SteamAPI_ISteamUserStats_GetDownloadedLeaderboardEntry(ISteamUserStats* self, SteamLeaderboardEntries_t a0, int a1, LeaderboardEntry_t* a2, int32_t* a3, int a4) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetDownloadedLeaderboardEntry", true);
    if (!self)
        return {};
    return self->GetDownloadedLeaderboardEntry(a0, a1, a2, a3, a4);
}

SteamAPICall_t SteamAPI_ISteamUserStats_UploadLeaderboardScore(ISteamUserStats* self, SteamLeaderboard_t a0, int a1, int32_t a2, const int32_t* a3, int a4) {
    SF_FLAT("SteamAPI_ISteamUserStats_UploadLeaderboardScore", true);
    if (!self)
        return {};
    return self->UploadLeaderboardScore(a0, a1, a2, a3, a4);
}

SteamAPICall_t SteamAPI_ISteamUserStats_AttachLeaderboardUGC(ISteamUserStats* self, SteamLeaderboard_t a0, UGCHandle_t a1) {
    SF_FLAT("SteamAPI_ISteamUserStats_AttachLeaderboardUGC", true);
    if (!self)
        return {};
    return self->AttachLeaderboardUGC(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamUserStats_GetNumberOfCurrentPlayers(ISteamUserStats* self) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetNumberOfCurrentPlayers", true);
    if (!self)
        return {};
    return self->GetNumberOfCurrentPlayers();
}

SteamAPICall_t SteamAPI_ISteamUserStats_RequestGlobalAchievementPercentages(ISteamUserStats* self) {
    SF_FLAT("SteamAPI_ISteamUserStats_RequestGlobalAchievementPercentages", true);
    if (!self)
        return {};
    return self->RequestGlobalAchievementPercentages();
}

int SteamAPI_ISteamUserStats_GetMostAchievedAchievementInfo(ISteamUserStats* self, char* a0, uint32_t a1, float* a2, bool* a3) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetMostAchievedAchievementInfo", true);
    if (!self)
        return {};
    return self->GetMostAchievedAchievementInfo(a0, a1, a2, a3);
}

int SteamAPI_ISteamUserStats_GetNextMostAchievedAchievementInfo(ISteamUserStats* self, int a0, char* a1, uint32_t a2, float* a3, bool* a4) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetNextMostAchievedAchievementInfo", true);
    if (!self)
        return {};
    return self->GetNextMostAchievedAchievementInfo(a0, a1, a2, a3, a4);
}

bool SteamAPI_ISteamUserStats_GetAchievementAchievedPercent(ISteamUserStats* self, const char* a0, float* a1) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetAchievementAchievedPercent", true);
    if (!self)
        return {};
    return self->GetAchievementAchievedPercent(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamUserStats_RequestGlobalStats(ISteamUserStats* self, int a0) {
    SF_FLAT("SteamAPI_ISteamUserStats_RequestGlobalStats", true);
    if (!self)
        return {};
    return self->RequestGlobalStats(a0);
}

bool SteamAPI_ISteamUserStats_GetGlobalStatInt64(ISteamUserStats* self, const char* a0, int64_t* a1) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetGlobalStatInt64", true);
    if (!self)
        return {};
    return self->GetGlobalStat(a0, a1);
}

bool SteamAPI_ISteamUserStats_GetGlobalStatDouble(ISteamUserStats* self, const char* a0, double* a1) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetGlobalStatDouble", true);
    if (!self)
        return {};
    return self->GetGlobalStat(a0, a1);
}

int32_t SteamAPI_ISteamUserStats_GetGlobalStatHistoryInt64(ISteamUserStats* self, const char* a0, int64_t* a1, uint32_t a2) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetGlobalStatHistoryInt64", true);
    if (!self)
        return {};
    return self->GetGlobalStatHistory(a0, a1, a2);
}

int32_t SteamAPI_ISteamUserStats_GetGlobalStatHistoryDouble(ISteamUserStats* self, const char* a0, double* a1, uint32_t a2) {
    SF_FLAT("SteamAPI_ISteamUserStats_GetGlobalStatHistoryDouble", true);
    if (!self)
        return {};
    return self->GetGlobalStatHistory(a0, a1, a2);
}

int SteamAPI_ISteamMatchmaking_GetFavoriteGameCount(ISteamMatchmaking* self) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_GetFavoriteGameCount", true);
    if (!self)
        return {};
    return self->GetFavoriteGameCount();
}

bool SteamAPI_ISteamMatchmaking_GetFavoriteGame(ISteamMatchmaking* self, int a0, AppId_t* a1, uint32_t* a2, uint16_t* a3, uint16_t* a4, uint32_t* a5, uint32_t* a6) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_GetFavoriteGame", true);
    if (!self)
        return {};
    return self->GetFavoriteGame(a0, a1, a2, a3, a4, a5, a6);
}

int SteamAPI_ISteamMatchmaking_AddFavoriteGame(ISteamMatchmaking* self, AppId_t a0, uint32_t a1, uint16_t a2, uint16_t a3, uint32_t a4, uint32_t a5) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_AddFavoriteGame", true);
    if (!self)
        return {};
    return self->AddFavoriteGame(a0, a1, a2, a3, a4, a5);
}

bool SteamAPI_ISteamMatchmaking_RemoveFavoriteGame(ISteamMatchmaking* self, AppId_t a0, uint32_t a1, uint16_t a2, uint16_t a3, uint32_t a4) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_RemoveFavoriteGame", true);
    if (!self)
        return {};
    return self->RemoveFavoriteGame(a0, a1, a2, a3, a4);
}

SteamAPICall_t SteamAPI_ISteamMatchmaking_RequestLobbyList(ISteamMatchmaking* self) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_RequestLobbyList", true);
    if (!self)
        return {};
    return self->RequestLobbyList();
}

void SteamAPI_ISteamMatchmaking_AddRequestLobbyListStringFilter(ISteamMatchmaking* self, const char* a0, const char* a1, int a2) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_AddRequestLobbyListStringFilter", true);
    if (self)
        self->AddRequestLobbyListStringFilter(a0, a1, a2);
}

void SteamAPI_ISteamMatchmaking_AddRequestLobbyListNumericalFilter(ISteamMatchmaking* self, const char* a0, int a1, int a2) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_AddRequestLobbyListNumericalFilter", true);
    if (self)
        self->AddRequestLobbyListNumericalFilter(a0, a1, a2);
}

void SteamAPI_ISteamMatchmaking_AddRequestLobbyListNearValueFilter(ISteamMatchmaking* self, const char* a0, int a1) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_AddRequestLobbyListNearValueFilter", true);
    if (self)
        self->AddRequestLobbyListNearValueFilter(a0, a1);
}

void SteamAPI_ISteamMatchmaking_AddRequestLobbyListFilterSlotsAvailable(ISteamMatchmaking* self, int a0) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_AddRequestLobbyListFilterSlotsAvailable", true);
    if (self)
        self->AddRequestLobbyListFilterSlotsAvailable(a0);
}

void SteamAPI_ISteamMatchmaking_AddRequestLobbyListDistanceFilter(ISteamMatchmaking* self, int a0) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_AddRequestLobbyListDistanceFilter", true);
    if (self)
        self->AddRequestLobbyListDistanceFilter(a0);
}

void SteamAPI_ISteamMatchmaking_AddRequestLobbyListResultCountFilter(ISteamMatchmaking* self, int a0) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_AddRequestLobbyListResultCountFilter", true);
    if (self)
        self->AddRequestLobbyListResultCountFilter(a0);
}

void SteamAPI_ISteamMatchmaking_AddRequestLobbyListCompatibleMembersFilter(ISteamMatchmaking* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_AddRequestLobbyListCompatibleMembersFilter", true);
    if (self)
        self->AddRequestLobbyListCompatibleMembersFilter(a0);
}

CSteamID SteamAPI_ISteamMatchmaking_GetLobbyByIndex(ISteamMatchmaking* self, int a0) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_GetLobbyByIndex", true);
    if (!self)
        return {};
    return self->GetLobbyByIndex(a0);
}

SteamAPICall_t SteamAPI_ISteamMatchmaking_CreateLobby(ISteamMatchmaking* self, int a0, int a1) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_CreateLobby", true);
    if (!self)
        return {};
    return self->CreateLobby(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamMatchmaking_JoinLobby(ISteamMatchmaking* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_JoinLobby", true);
    if (!self)
        return {};
    return self->JoinLobby(a0);
}

void SteamAPI_ISteamMatchmaking_LeaveLobby(ISteamMatchmaking* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_LeaveLobby", true);
    if (self)
        self->LeaveLobby(a0);
}

bool SteamAPI_ISteamMatchmaking_InviteUserToLobby(ISteamMatchmaking* self, CSteamID a0, CSteamID a1) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_InviteUserToLobby", true);
    if (!self)
        return {};
    return self->InviteUserToLobby(a0, a1);
}

int SteamAPI_ISteamMatchmaking_GetNumLobbyMembers(ISteamMatchmaking* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_GetNumLobbyMembers", true);
    if (!self)
        return {};
    return self->GetNumLobbyMembers(a0);
}

CSteamID SteamAPI_ISteamMatchmaking_GetLobbyMemberByIndex(ISteamMatchmaking* self, CSteamID a0, int a1) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_GetLobbyMemberByIndex", true);
    if (!self)
        return {};
    return self->GetLobbyMemberByIndex(a0, a1);
}

const char* SteamAPI_ISteamMatchmaking_GetLobbyData(ISteamMatchmaking* self, CSteamID a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_GetLobbyData", true);
    if (!self)
        return {};
    return self->GetLobbyData(a0, a1);
}

bool SteamAPI_ISteamMatchmaking_SetLobbyData(ISteamMatchmaking* self, CSteamID a0, const char* a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_SetLobbyData", true);
    if (!self)
        return {};
    return self->SetLobbyData(a0, a1, a2);
}

int SteamAPI_ISteamMatchmaking_GetLobbyDataCount(ISteamMatchmaking* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_GetLobbyDataCount", true);
    if (!self)
        return {};
    return self->GetLobbyDataCount(a0);
}

bool SteamAPI_ISteamMatchmaking_GetLobbyDataByIndex(ISteamMatchmaking* self, CSteamID a0, int a1, char* a2, int a3, char* a4, int a5) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_GetLobbyDataByIndex", true);
    if (!self)
        return {};
    return self->GetLobbyDataByIndex(a0, a1, a2, a3, a4, a5);
}

bool SteamAPI_ISteamMatchmaking_DeleteLobbyData(ISteamMatchmaking* self, CSteamID a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_DeleteLobbyData", true);
    if (!self)
        return {};
    return self->DeleteLobbyData(a0, a1);
}

const char* SteamAPI_ISteamMatchmaking_GetLobbyMemberData(ISteamMatchmaking* self, CSteamID a0, CSteamID a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_GetLobbyMemberData", true);
    if (!self)
        return {};
    return self->GetLobbyMemberData(a0, a1, a2);
}

void SteamAPI_ISteamMatchmaking_SetLobbyMemberData(ISteamMatchmaking* self, CSteamID a0, const char* a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_SetLobbyMemberData", true);
    if (self)
        self->SetLobbyMemberData(a0, a1, a2);
}

bool SteamAPI_ISteamMatchmaking_SendLobbyChatMsg(ISteamMatchmaking* self, CSteamID a0, const void* a1, int a2) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_SendLobbyChatMsg", true);
    if (!self)
        return {};
    return self->SendLobbyChatMsg(a0, a1, a2);
}

int SteamAPI_ISteamMatchmaking_GetLobbyChatEntry(ISteamMatchmaking* self, CSteamID a0, int a1, CSteamID* a2, void* a3, int a4, int* a5) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_GetLobbyChatEntry", true);
    if (!self)
        return {};
    return self->GetLobbyChatEntry(a0, a1, a2, a3, a4, a5);
}

bool SteamAPI_ISteamMatchmaking_RequestLobbyData(ISteamMatchmaking* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_RequestLobbyData", true);
    if (!self)
        return {};
    return self->RequestLobbyData(a0);
}

void SteamAPI_ISteamMatchmaking_SetLobbyGameServer(ISteamMatchmaking* self, CSteamID a0, uint32_t a1, uint16_t a2, CSteamID a3) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_SetLobbyGameServer", true);
    if (self)
        self->SetLobbyGameServer(a0, a1, a2, a3);
}

bool SteamAPI_ISteamMatchmaking_GetLobbyGameServer(ISteamMatchmaking* self, CSteamID a0, uint32_t* a1, uint16_t* a2, CSteamID* a3) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_GetLobbyGameServer", true);
    if (!self)
        return {};
    return self->GetLobbyGameServer(a0, a1, a2, a3);
}

bool SteamAPI_ISteamMatchmaking_SetLobbyMemberLimit(ISteamMatchmaking* self, CSteamID a0, int a1) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_SetLobbyMemberLimit", true);
    if (!self)
        return {};
    return self->SetLobbyMemberLimit(a0, a1);
}

int SteamAPI_ISteamMatchmaking_GetLobbyMemberLimit(ISteamMatchmaking* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_GetLobbyMemberLimit", true);
    if (!self)
        return {};
    return self->GetLobbyMemberLimit(a0);
}

bool SteamAPI_ISteamMatchmaking_SetLobbyType(ISteamMatchmaking* self, CSteamID a0, int a1) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_SetLobbyType", true);
    if (!self)
        return {};
    return self->SetLobbyType(a0, a1);
}

bool SteamAPI_ISteamMatchmaking_SetLobbyJoinable(ISteamMatchmaking* self, CSteamID a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_SetLobbyJoinable", true);
    if (!self)
        return {};
    return self->SetLobbyJoinable(a0, a1);
}

CSteamID SteamAPI_ISteamMatchmaking_GetLobbyOwner(ISteamMatchmaking* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_GetLobbyOwner", true);
    if (!self)
        return {};
    return self->GetLobbyOwner(a0);
}

bool SteamAPI_ISteamMatchmaking_SetLobbyOwner(ISteamMatchmaking* self, CSteamID a0, CSteamID a1) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_SetLobbyOwner", true);
    if (!self)
        return {};
    return self->SetLobbyOwner(a0, a1);
}

bool SteamAPI_ISteamMatchmaking_SetLinkedLobby(ISteamMatchmaking* self, CSteamID a0, CSteamID a1) {
    SF_FLAT("SteamAPI_ISteamMatchmaking_SetLinkedLobby", true);
    if (!self)
        return {};
    return self->SetLinkedLobby(a0, a1);
}

HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestInternetServerList(ISteamMatchmakingServers* self, AppId_t a0, MatchMakingKeyValuePair_t** a1, uint32_t a2, ISteamMatchmakingServerListResponse* a3) {
    SF_FLAT("SteamAPI_ISteamMatchmakingServers_RequestInternetServerList", true);
    if (!self)
        return {};
    return self->RequestInternetServerList(a0, a1, a2, a3);
}

HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestLANServerList(ISteamMatchmakingServers* self, AppId_t a0, ISteamMatchmakingServerListResponse* a1) {
    SF_FLAT("SteamAPI_ISteamMatchmakingServers_RequestLANServerList", true);
    if (!self)
        return {};
    return self->RequestLANServerList(a0, a1);
}

HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestFriendsServerList(ISteamMatchmakingServers* self, AppId_t a0, MatchMakingKeyValuePair_t** a1, uint32_t a2, ISteamMatchmakingServerListResponse* a3) {
    SF_FLAT("SteamAPI_ISteamMatchmakingServers_RequestFriendsServerList", true);
    if (!self)
        return {};
    return self->RequestFriendsServerList(a0, a1, a2, a3);
}

HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestFavoritesServerList(ISteamMatchmakingServers* self, AppId_t a0, MatchMakingKeyValuePair_t** a1, uint32_t a2, ISteamMatchmakingServerListResponse* a3) {
    SF_FLAT("SteamAPI_ISteamMatchmakingServers_RequestFavoritesServerList", true);
    if (!self)
        return {};
    return self->RequestFavoritesServerList(a0, a1, a2, a3);
}

HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestHistoryServerList(ISteamMatchmakingServers* self, AppId_t a0, MatchMakingKeyValuePair_t** a1, uint32_t a2, ISteamMatchmakingServerListResponse* a3) {
    SF_FLAT("SteamAPI_ISteamMatchmakingServers_RequestHistoryServerList", true);
    if (!self)
        return {};
    return self->RequestHistoryServerList(a0, a1, a2, a3);
}

HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestSpectatorServerList(ISteamMatchmakingServers* self, AppId_t a0, MatchMakingKeyValuePair_t** a1, uint32_t a2, ISteamMatchmakingServerListResponse* a3) {
    SF_FLAT("SteamAPI_ISteamMatchmakingServers_RequestSpectatorServerList", true);
    if (!self)
        return {};
    return self->RequestSpectatorServerList(a0, a1, a2, a3);
}

void SteamAPI_ISteamMatchmakingServers_ReleaseRequest(ISteamMatchmakingServers* self, HServerListRequest a0) {
    SF_FLAT("SteamAPI_ISteamMatchmakingServers_ReleaseRequest", true);
    if (self)
        self->ReleaseRequest(a0);
}

gameserveritem_t* SteamAPI_ISteamMatchmakingServers_GetServerDetails(ISteamMatchmakingServers* self, HServerListRequest a0, int a1) {
    SF_FLAT("SteamAPI_ISteamMatchmakingServers_GetServerDetails", true);
    if (!self)
        return {};
    return self->GetServerDetails(a0, a1);
}

void SteamAPI_ISteamMatchmakingServers_CancelQuery(ISteamMatchmakingServers* self, HServerListRequest a0) {
    SF_FLAT("SteamAPI_ISteamMatchmakingServers_CancelQuery", true);
    if (self)
        self->CancelQuery(a0);
}

void SteamAPI_ISteamMatchmakingServers_RefreshQuery(ISteamMatchmakingServers* self, HServerListRequest a0) {
    SF_FLAT("SteamAPI_ISteamMatchmakingServers_RefreshQuery", true);
    if (self)
        self->RefreshQuery(a0);
}

bool SteamAPI_ISteamMatchmakingServers_IsRefreshing(ISteamMatchmakingServers* self, HServerListRequest a0) {
    SF_FLAT("SteamAPI_ISteamMatchmakingServers_IsRefreshing", true);
    if (!self)
        return {};
    return self->IsRefreshing(a0);
}

int SteamAPI_ISteamMatchmakingServers_GetServerCount(ISteamMatchmakingServers* self, HServerListRequest a0) {
    SF_FLAT("SteamAPI_ISteamMatchmakingServers_GetServerCount", true);
    if (!self)
        return {};
    return self->GetServerCount(a0);
}

void SteamAPI_ISteamMatchmakingServers_RefreshServer(ISteamMatchmakingServers* self, HServerListRequest a0, int a1) {
    SF_FLAT("SteamAPI_ISteamMatchmakingServers_RefreshServer", true);
    if (self)
        self->RefreshServer(a0, a1);
}

HServerQuery SteamAPI_ISteamMatchmakingServers_PingServer(ISteamMatchmakingServers* self, uint32_t a0, uint16_t a1, ISteamMatchmakingServerListResponse* a2) {
    SF_FLAT("SteamAPI_ISteamMatchmakingServers_PingServer", true);
    if (!self)
        return {};
    return self->PingServer(a0, a1, a2);
}

HServerQuery SteamAPI_ISteamMatchmakingServers_PlayerDetails(ISteamMatchmakingServers* self, uint32_t a0, uint16_t a1, ISteamMatchmakingServerListResponse* a2) {
    SF_FLAT("SteamAPI_ISteamMatchmakingServers_PlayerDetails", true);
    if (!self)
        return {};
    return self->PlayerDetails(a0, a1, a2);
}

HServerQuery SteamAPI_ISteamMatchmakingServers_ServerRules(ISteamMatchmakingServers* self, uint32_t a0, uint16_t a1, ISteamMatchmakingServerListResponse* a2) {
    SF_FLAT("SteamAPI_ISteamMatchmakingServers_ServerRules", true);
    if (!self)
        return {};
    return self->ServerRules(a0, a1, a2);
}

void SteamAPI_ISteamMatchmakingServers_CancelServerQuery(ISteamMatchmakingServers* self, HServerQuery a0) {
    SF_FLAT("SteamAPI_ISteamMatchmakingServers_CancelServerQuery", true);
    if (self)
        self->CancelServerQuery(a0);
}

bool SteamAPI_ISteamNetworking_SendP2PPacket(ISteamNetworking* self, CSteamID a0, const void* a1, uint32_t a2, int a3, int a4) {
    SF_FLAT("SteamAPI_ISteamNetworking_SendP2PPacket", true);
    if (!self)
        return {};
    return self->SendP2PPacket(a0, a1, a2, a3, a4);
}

bool SteamAPI_ISteamNetworking_IsP2PPacketAvailable(ISteamNetworking* self, uint32_t* a0, int a1) {
    SF_FLAT("SteamAPI_ISteamNetworking_IsP2PPacketAvailable", true);
    if (!self)
        return {};
    return self->IsP2PPacketAvailable(a0, a1);
}

bool SteamAPI_ISteamNetworking_ReadP2PPacket(ISteamNetworking* self, void* a0, uint32_t a1, uint32_t* a2, CSteamID* a3, int a4) {
    SF_FLAT("SteamAPI_ISteamNetworking_ReadP2PPacket", true);
    if (!self)
        return {};
    return self->ReadP2PPacket(a0, a1, a2, a3, a4);
}

bool SteamAPI_ISteamNetworking_AcceptP2PSessionWithUser(ISteamNetworking* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamNetworking_AcceptP2PSessionWithUser", true);
    if (!self)
        return {};
    return self->AcceptP2PSessionWithUser(a0);
}

bool SteamAPI_ISteamNetworking_CloseP2PSessionWithUser(ISteamNetworking* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamNetworking_CloseP2PSessionWithUser", true);
    if (!self)
        return {};
    return self->CloseP2PSessionWithUser(a0);
}

bool SteamAPI_ISteamNetworking_CloseP2PChannelWithUser(ISteamNetworking* self, CSteamID a0, int a1) {
    SF_FLAT("SteamAPI_ISteamNetworking_CloseP2PChannelWithUser", true);
    if (!self)
        return {};
    return self->CloseP2PChannelWithUser(a0, a1);
}

bool SteamAPI_ISteamNetworking_GetP2PSessionState(ISteamNetworking* self, CSteamID a0, P2PSessionState_t* a1) {
    SF_FLAT("SteamAPI_ISteamNetworking_GetP2PSessionState", true);
    if (!self)
        return {};
    return self->GetP2PSessionState(a0, a1);
}

bool SteamAPI_ISteamNetworking_AllowP2PPacketRelay(ISteamNetworking* self, bool a0) {
    SF_FLAT("SteamAPI_ISteamNetworking_AllowP2PPacketRelay", true);
    if (!self)
        return {};
    return self->AllowP2PPacketRelay(a0);
}

SNetListenSocket_t SteamAPI_ISteamNetworking_CreateListenSocket(ISteamNetworking* self, int a0, SteamIPAddress_t a1, uint16_t a2, bool a3) {
    SF_FLAT("SteamAPI_ISteamNetworking_CreateListenSocket", true);
    if (!self)
        return {};
    return self->CreateListenSocket(a0, a1, a2, a3);
}

SNetSocket_t SteamAPI_ISteamNetworking_CreateP2PConnectionSocket(ISteamNetworking* self, CSteamID a0, int a1, int a2, bool a3) {
    SF_FLAT("SteamAPI_ISteamNetworking_CreateP2PConnectionSocket", true);
    if (!self)
        return {};
    return self->CreateP2PConnectionSocket(a0, a1, a2, a3);
}

SNetSocket_t SteamAPI_ISteamNetworking_CreateConnectionSocket(ISteamNetworking* self, SteamIPAddress_t a0, uint16_t a1, int a2) {
    SF_FLAT("SteamAPI_ISteamNetworking_CreateConnectionSocket", true);
    if (!self)
        return {};
    return self->CreateConnectionSocket(a0, a1, a2);
}

bool SteamAPI_ISteamNetworking_DestroySocket(ISteamNetworking* self, SNetSocket_t a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamNetworking_DestroySocket", true);
    if (!self)
        return {};
    return self->DestroySocket(a0, a1);
}

bool SteamAPI_ISteamNetworking_DestroyListenSocket(ISteamNetworking* self, SNetListenSocket_t a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamNetworking_DestroyListenSocket", true);
    if (!self)
        return {};
    return self->DestroyListenSocket(a0, a1);
}

bool SteamAPI_ISteamNetworking_SendDataOnSocket(ISteamNetworking* self, SNetSocket_t a0, void* a1, uint32_t a2, bool a3) {
    SF_FLAT("SteamAPI_ISteamNetworking_SendDataOnSocket", true);
    if (!self)
        return {};
    return self->SendDataOnSocket(a0, a1, a2, a3);
}

bool SteamAPI_ISteamNetworking_IsDataAvailableOnSocket(ISteamNetworking* self, SNetSocket_t a0, uint32_t* a1) {
    SF_FLAT("SteamAPI_ISteamNetworking_IsDataAvailableOnSocket", true);
    if (!self)
        return {};
    return self->IsDataAvailableOnSocket(a0, a1);
}

bool SteamAPI_ISteamNetworking_RetrieveDataFromSocket(ISteamNetworking* self, SNetSocket_t a0, void* a1, uint32_t a2, uint32_t* a3) {
    SF_FLAT("SteamAPI_ISteamNetworking_RetrieveDataFromSocket", true);
    if (!self)
        return {};
    return self->RetrieveDataFromSocket(a0, a1, a2, a3);
}

bool SteamAPI_ISteamNetworking_IsDataAvailable(ISteamNetworking* self, SNetListenSocket_t a0, uint32_t* a1, bool a2) {
    SF_FLAT("SteamAPI_ISteamNetworking_IsDataAvailable", true);
    if (!self)
        return {};
    return self->IsDataAvailable(a0, a1, a2);
}

bool SteamAPI_ISteamNetworking_RetrieveData(ISteamNetworking* self, SNetListenSocket_t a0, void* a1, uint32_t a2, uint32_t* a3) {
    SF_FLAT("SteamAPI_ISteamNetworking_RetrieveData", true);
    if (!self)
        return {};
    return self->RetrieveData(a0, a1, a2, a3);
}

bool SteamAPI_ISteamNetworking_GetSocketInfo(ISteamNetworking* self, SNetSocket_t a0, CSteamID* a1, int* a2, SteamIPAddress_t* a3, uint16_t* a4) {
    SF_FLAT("SteamAPI_ISteamNetworking_GetSocketInfo", true);
    if (!self)
        return {};
    return self->GetSocketInfo(a0, a1, a2, a3, a4);
}

bool SteamAPI_ISteamNetworking_GetListenSocketInfo(ISteamNetworking* self, SNetListenSocket_t a0, SteamIPAddress_t* a1, uint16_t* a2) {
    SF_FLAT("SteamAPI_ISteamNetworking_GetListenSocketInfo", true);
    if (!self)
        return {};
    return self->GetListenSocketInfo(a0, a1, a2);
}

int SteamAPI_ISteamNetworking_GetSocketConnectionType(ISteamNetworking* self, SNetSocket_t a0) {
    SF_FLAT("SteamAPI_ISteamNetworking_GetSocketConnectionType", true);
    if (!self)
        return {};
    return self->GetSocketConnectionType(a0);
}

int SteamAPI_ISteamNetworking_GetMaxPacketSize(ISteamNetworking* self, SNetSocket_t a0) {
    SF_FLAT("SteamAPI_ISteamNetworking_GetMaxPacketSize", true);
    if (!self)
        return {};
    return self->GetMaxPacketSize(a0);
}

bool SteamAPI_ISteamRemoteStorage_FileWrite(ISteamRemoteStorage* self, const char* a0, const void* a1, int32_t a2) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_FileWrite", true);
    if (!self)
        return {};
    return self->FileWrite(a0, a1, a2);
}

int32_t SteamAPI_ISteamRemoteStorage_FileRead(ISteamRemoteStorage* self, const char* a0, void* a1, int32_t a2) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_FileRead", true);
    if (!self)
        return {};
    return self->FileRead(a0, a1, a2);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_FileWriteAsync(ISteamRemoteStorage* self, const char* a0, const void* a1, uint32_t a2) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_FileWriteAsync", true);
    if (!self)
        return {};
    return self->FileWriteAsync(a0, a1, a2);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_FileReadAsync(ISteamRemoteStorage* self, const char* a0, uint32_t a1, uint32_t a2) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_FileReadAsync", true);
    if (!self)
        return {};
    return self->FileReadAsync(a0, a1, a2);
}

bool SteamAPI_ISteamRemoteStorage_FileReadAsyncComplete(ISteamRemoteStorage* self, SteamAPICall_t a0, void* a1, uint32_t a2) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_FileReadAsyncComplete", true);
    if (!self)
        return {};
    return self->FileReadAsyncComplete(a0, a1, a2);
}

bool SteamAPI_ISteamRemoteStorage_FileForget(ISteamRemoteStorage* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_FileForget", true);
    if (!self)
        return {};
    return self->FileForget(a0);
}

bool SteamAPI_ISteamRemoteStorage_FileDelete(ISteamRemoteStorage* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_FileDelete", true);
    if (!self)
        return {};
    return self->FileDelete(a0);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_FileShare(ISteamRemoteStorage* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_FileShare", true);
    if (!self)
        return {};
    return self->FileShare(a0);
}

bool SteamAPI_ISteamRemoteStorage_SetSyncPlatforms(ISteamRemoteStorage* self, const char* a0, uint32_t a1) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_SetSyncPlatforms", true);
    if (!self)
        return {};
    return self->SetSyncPlatforms(a0, a1);
}

UGCFileWriteStreamHandle_t SteamAPI_ISteamRemoteStorage_FileWriteStreamOpen(ISteamRemoteStorage* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_FileWriteStreamOpen", true);
    if (!self)
        return {};
    return self->FileWriteStreamOpen(a0);
}

bool SteamAPI_ISteamRemoteStorage_FileWriteStreamWriteChunk(ISteamRemoteStorage* self, UGCFileWriteStreamHandle_t a0, const void* a1, int32_t a2) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_FileWriteStreamWriteChunk", true);
    if (!self)
        return {};
    return self->FileWriteStreamWriteChunk(a0, a1, a2);
}

bool SteamAPI_ISteamRemoteStorage_FileWriteStreamClose(ISteamRemoteStorage* self, UGCFileWriteStreamHandle_t a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_FileWriteStreamClose", true);
    if (!self)
        return {};
    return self->FileWriteStreamClose(a0);
}

bool SteamAPI_ISteamRemoteStorage_FileWriteStreamCancel(ISteamRemoteStorage* self, UGCFileWriteStreamHandle_t a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_FileWriteStreamCancel", true);
    if (!self)
        return {};
    return self->FileWriteStreamCancel(a0);
}

bool SteamAPI_ISteamRemoteStorage_FileExists(ISteamRemoteStorage* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_FileExists", true);
    if (!self)
        return {};
    return self->FileExists(a0);
}

bool SteamAPI_ISteamRemoteStorage_FilePersisted(ISteamRemoteStorage* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_FilePersisted", true);
    if (!self)
        return {};
    return self->FilePersisted(a0);
}

int32_t SteamAPI_ISteamRemoteStorage_GetFileSize(ISteamRemoteStorage* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_GetFileSize", true);
    if (!self)
        return {};
    return self->GetFileSize(a0);
}

int64_t SteamAPI_ISteamRemoteStorage_GetFileTimestamp(ISteamRemoteStorage* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_GetFileTimestamp", true);
    if (!self)
        return {};
    return self->GetFileTimestamp(a0);
}

uint32_t SteamAPI_ISteamRemoteStorage_GetSyncPlatforms(ISteamRemoteStorage* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_GetSyncPlatforms", true);
    if (!self)
        return {};
    return self->GetSyncPlatforms(a0);
}

int32_t SteamAPI_ISteamRemoteStorage_GetFileCount(ISteamRemoteStorage* self) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_GetFileCount", true);
    if (!self)
        return {};
    return self->GetFileCount();
}

const char* SteamAPI_ISteamRemoteStorage_GetFileNameAndSize(ISteamRemoteStorage* self, int a0, int32_t* a1) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_GetFileNameAndSize", true);
    if (!self)
        return {};
    return self->GetFileNameAndSize(a0, a1);
}

bool SteamAPI_ISteamRemoteStorage_GetQuota(ISteamRemoteStorage* self, uint64_t* a0, uint64_t* a1) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_GetQuota", true);
    if (!self)
        return {};
    return self->GetQuota(a0, a1);
}

bool SteamAPI_ISteamRemoteStorage_IsCloudEnabledForAccount(ISteamRemoteStorage* self) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_IsCloudEnabledForAccount", true);
    if (!self)
        return {};
    return self->IsCloudEnabledForAccount();
}

bool SteamAPI_ISteamRemoteStorage_IsCloudEnabledForApp(ISteamRemoteStorage* self) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_IsCloudEnabledForApp", true);
    if (!self)
        return {};
    return self->IsCloudEnabledForApp();
}

void SteamAPI_ISteamRemoteStorage_SetCloudEnabledForApp(ISteamRemoteStorage* self, bool a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_SetCloudEnabledForApp", true);
    if (self)
        self->SetCloudEnabledForApp(a0);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_UGCDownload(ISteamRemoteStorage* self, UGCHandle_t a0, uint32_t a1) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_UGCDownload", true);
    if (!self)
        return {};
    return self->UGCDownload(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_UGCDownloadToLocation(ISteamRemoteStorage* self, UGCHandle_t a0, const char* a1, uint32_t a2) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_UGCDownloadToLocation", true);
    if (!self)
        return {};
    return self->UGCDownloadToLocation(a0, a1, a2);
}

bool SteamAPI_ISteamRemoteStorage_GetUGCDownloadProgress(ISteamRemoteStorage* self, UGCHandle_t a0, int32_t* a1, int32_t* a2) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_GetUGCDownloadProgress", true);
    if (!self)
        return {};
    return self->GetUGCDownloadProgress(a0, a1, a2);
}

bool SteamAPI_ISteamRemoteStorage_GetUGCDetails(ISteamRemoteStorage* self, UGCHandle_t a0, AppId_t* a1, char** a2, int32_t* a3, CSteamID* a4) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_GetUGCDetails", true);
    if (!self)
        return {};
    return self->GetUGCDetails(a0, a1, a2, a3, a4);
}

int32_t SteamAPI_ISteamRemoteStorage_UGCRead(ISteamRemoteStorage* self, UGCHandle_t a0, void* a1, int32_t a2, uint32_t a3, int a4) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_UGCRead", true);
    if (!self)
        return {};
    return self->UGCRead(a0, a1, a2, a3, a4);
}

int32_t SteamAPI_ISteamRemoteStorage_GetCachedUGCCount(ISteamRemoteStorage* self) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_GetCachedUGCCount", true);
    if (!self)
        return {};
    return self->GetCachedUGCCount();
}

UGCHandle_t SteamAPI_ISteamRemoteStorage_GetCachedUGCHandle(ISteamRemoteStorage* self, int32_t a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_GetCachedUGCHandle", true);
    if (!self)
        return {};
    return self->GetCachedUGCHandle(a0);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_PublishWorkshopFile(ISteamRemoteStorage* self, const char* a0, const char* a1, AppId_t a2, const char* a3, const char* a4, int a5, SteamParamStringArray_t* a6, int a7) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_PublishWorkshopFile", true);
    if (!self)
        return {};
    return self->PublishWorkshopFile(a0, a1, a2, a3, a4, a5, a6, a7);
}

PublishedFileUpdateHandle_t SteamAPI_ISteamRemoteStorage_CreatePublishedFileUpdateRequest(ISteamRemoteStorage* self, PublishedFileId_t a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_CreatePublishedFileUpdateRequest", true);
    if (!self)
        return {};
    return self->CreatePublishedFileUpdateRequest(a0);
}

bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileFile(ISteamRemoteStorage* self, PublishedFileUpdateHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_UpdatePublishedFileFile", true);
    if (!self)
        return {};
    return self->UpdatePublishedFileFile(a0, a1);
}

bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFilePreviewFile(ISteamRemoteStorage* self, PublishedFileUpdateHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_UpdatePublishedFilePreviewFile", true);
    if (!self)
        return {};
    return self->UpdatePublishedFilePreviewFile(a0, a1);
}

bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileTitle(ISteamRemoteStorage* self, PublishedFileUpdateHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_UpdatePublishedFileTitle", true);
    if (!self)
        return {};
    return self->UpdatePublishedFileTitle(a0, a1);
}

bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileDescription(ISteamRemoteStorage* self, PublishedFileUpdateHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_UpdatePublishedFileDescription", true);
    if (!self)
        return {};
    return self->UpdatePublishedFileDescription(a0, a1);
}

bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileVisibility(ISteamRemoteStorage* self, PublishedFileUpdateHandle_t a0, int a1) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_UpdatePublishedFileVisibility", true);
    if (!self)
        return {};
    return self->UpdatePublishedFileVisibility(a0, a1);
}

bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileTags(ISteamRemoteStorage* self, PublishedFileUpdateHandle_t a0, SteamParamStringArray_t* a1) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_UpdatePublishedFileTags", true);
    if (!self)
        return {};
    return self->UpdatePublishedFileTags(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_CommitPublishedFileUpdate(ISteamRemoteStorage* self, PublishedFileUpdateHandle_t a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_CommitPublishedFileUpdate", true);
    if (!self)
        return {};
    return self->CommitPublishedFileUpdate(a0);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_GetPublishedFileDetails(ISteamRemoteStorage* self, PublishedFileId_t a0, uint32_t a1) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_GetPublishedFileDetails", true);
    if (!self)
        return {};
    return self->GetPublishedFileDetails(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_DeletePublishedFile(ISteamRemoteStorage* self, PublishedFileId_t a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_DeletePublishedFile", true);
    if (!self)
        return {};
    return self->DeletePublishedFile(a0);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_EnumerateUserPublishedFiles(ISteamRemoteStorage* self, uint32_t a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_EnumerateUserPublishedFiles", true);
    if (!self)
        return {};
    return self->EnumerateUserPublishedFiles(a0);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_SubscribePublishedFile(ISteamRemoteStorage* self, PublishedFileId_t a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_SubscribePublishedFile", true);
    if (!self)
        return {};
    return self->SubscribePublishedFile(a0);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_EnumerateUserSubscribedFiles(ISteamRemoteStorage* self, uint32_t a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_EnumerateUserSubscribedFiles", true);
    if (!self)
        return {};
    return self->EnumerateUserSubscribedFiles(a0);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_UnsubscribePublishedFile(ISteamRemoteStorage* self, PublishedFileId_t a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_UnsubscribePublishedFile", true);
    if (!self)
        return {};
    return self->UnsubscribePublishedFile(a0);
}

bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileSetChangeDescription(ISteamRemoteStorage* self, PublishedFileUpdateHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_UpdatePublishedFileSetChangeDescription", true);
    if (!self)
        return {};
    return self->UpdatePublishedFileSetChangeDescription(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_GetPublishedItemVoteDetails(ISteamRemoteStorage* self, PublishedFileId_t a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_GetPublishedItemVoteDetails", true);
    if (!self)
        return {};
    return self->GetPublishedItemVoteDetails(a0);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_UpdateUserPublishedItemVote(ISteamRemoteStorage* self, PublishedFileId_t a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_UpdateUserPublishedItemVote", true);
    if (!self)
        return {};
    return self->UpdateUserPublishedItemVote(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_GetUserPublishedItemVoteDetails(ISteamRemoteStorage* self, PublishedFileId_t a0) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_GetUserPublishedItemVoteDetails", true);
    if (!self)
        return {};
    return self->GetUserPublishedItemVoteDetails(a0);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_EnumerateUserSharedWorkshopFiles(ISteamRemoteStorage* self, CSteamID a0, uint32_t a1, SteamParamStringArray_t* a2, SteamParamStringArray_t* a3) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_EnumerateUserSharedWorkshopFiles", true);
    if (!self)
        return {};
    return self->EnumerateUserSharedWorkshopFiles(a0, a1, a2, a3);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_PublishVideo(ISteamRemoteStorage* self, int a0, const char* a1, const char* a2, const char* a3, AppId_t a4, const char* a5, const char* a6, int a7, SteamParamStringArray_t* a8) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_PublishVideo", true);
    if (!self)
        return {};
    return self->PublishVideo(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_SetUserPublishedFileAction(ISteamRemoteStorage* self, PublishedFileId_t a0, int a1) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_SetUserPublishedFileAction", true);
    if (!self)
        return {};
    return self->SetUserPublishedFileAction(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_EnumeratePublishedFilesByUserAction(ISteamRemoteStorage* self, int a0, uint32_t a1) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_EnumeratePublishedFilesByUserAction", true);
    if (!self)
        return {};
    return self->EnumeratePublishedFilesByUserAction(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_EnumeratePublishedWorkshopFiles(ISteamRemoteStorage* self, int a0, uint32_t a1, uint32_t a2, uint32_t a3, SteamParamStringArray_t* a4, SteamParamStringArray_t* a5) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_EnumeratePublishedWorkshopFiles", true);
    if (!self)
        return {};
    return self->EnumeratePublishedWorkshopFiles(a0, a1, a2, a3, a4, a5);
}

SteamAPICall_t SteamAPI_ISteamRemoteStorage_UGCDownloadToLocationEx(ISteamRemoteStorage* self, UGCHandle_t a0, const char* a1, uint32_t a2, const char* a3) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_UGCDownloadToLocationEx", true);
    if (!self)
        return {};
    return self->UGCDownloadToLocationEx(a0, a1, a2, a3);
}

int32_t SteamAPI_ISteamRemoteStorage_GetLocalFileChangeCount(ISteamRemoteStorage* self) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_GetLocalFileChangeCount", true);
    if (!self)
        return {};
    return self->GetLocalFileChangeCount();
}

int32_t SteamAPI_ISteamRemoteStorage_BeginFileWriteBatch(ISteamRemoteStorage* self) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_BeginFileWriteBatch", true);
    if (!self)
        return {};
    return self->BeginFileWriteBatch();
}

bool SteamAPI_ISteamRemoteStorage_EndFileWriteBatch(ISteamRemoteStorage* self) {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_EndFileWriteBatch", true);
    if (!self)
        return {};
    return self->EndFileWriteBatch();
}

bool SteamAPI_ISteamApps_BIsSubscribed(ISteamApps* self) {
    SF_FLAT("SteamAPI_ISteamApps_BIsSubscribed", true);
    if (!self)
        return {};
    return self->BIsSubscribed();
}

bool SteamAPI_ISteamApps_BIsLowViolence(ISteamApps* self) {
    SF_FLAT("SteamAPI_ISteamApps_BIsLowViolence", true);
    if (!self)
        return {};
    return self->BIsLowViolence();
}

bool SteamAPI_ISteamApps_BIsCybercafe(ISteamApps* self) {
    SF_FLAT("SteamAPI_ISteamApps_BIsCybercafe", true);
    if (!self)
        return {};
    return self->BIsCybercafe();
}

bool SteamAPI_ISteamApps_BIsVACBanned(ISteamApps* self) {
    SF_FLAT("SteamAPI_ISteamApps_BIsVACBanned", true);
    if (!self)
        return {};
    return self->BIsVACBanned();
}

const char* SteamAPI_ISteamApps_GetCurrentGameLanguage(ISteamApps* self) {
    SF_FLAT("SteamAPI_ISteamApps_GetCurrentGameLanguage", true);
    if (!self)
        return {};
    return self->GetCurrentGameLanguage();
}

const char* SteamAPI_ISteamApps_GetAvailableGameLanguages(ISteamApps* self) {
    SF_FLAT("SteamAPI_ISteamApps_GetAvailableGameLanguages", true);
    if (!self)
        return {};
    return self->GetAvailableGameLanguages();
}

bool SteamAPI_ISteamApps_BIsSubscribedApp(ISteamApps* self, AppId_t a0) {
    SF_FLAT("SteamAPI_ISteamApps_BIsSubscribedApp", true);
    if (!self)
        return {};
    return self->BIsSubscribedApp(a0);
}

bool SteamAPI_ISteamApps_BIsDlcInstalled(ISteamApps* self, AppId_t a0) {
    SF_FLAT("SteamAPI_ISteamApps_BIsDlcInstalled", true);
    if (!self)
        return {};
    return self->BIsDlcInstalled(a0);
}

uint32_t SteamAPI_ISteamApps_GetEarliestPurchaseUnixTime(ISteamApps* self, AppId_t a0) {
    SF_FLAT("SteamAPI_ISteamApps_GetEarliestPurchaseUnixTime", true);
    if (!self)
        return {};
    return self->GetEarliestPurchaseUnixTime(a0);
}

bool SteamAPI_ISteamApps_BIsSubscribedFromFreeWeekend(ISteamApps* self) {
    SF_FLAT("SteamAPI_ISteamApps_BIsSubscribedFromFreeWeekend", true);
    if (!self)
        return {};
    return self->BIsSubscribedFromFreeWeekend();
}

int SteamAPI_ISteamApps_GetDLCCount(ISteamApps* self) {
    SF_FLAT("SteamAPI_ISteamApps_GetDLCCount", true);
    if (!self)
        return {};
    return self->GetDLCCount();
}

bool SteamAPI_ISteamApps_BGetDLCDataByIndex(ISteamApps* self, int a0, AppId_t* a1, bool* a2, char* a3, int a4) {
    SF_FLAT("SteamAPI_ISteamApps_BGetDLCDataByIndex", true);
    if (!self)
        return {};
    return self->BGetDLCDataByIndex(a0, a1, a2, a3, a4);
}

void SteamAPI_ISteamApps_InstallDLC(ISteamApps* self, AppId_t a0) {
    SF_FLAT("SteamAPI_ISteamApps_InstallDLC", true);
    if (self)
        self->InstallDLC(a0);
}

void SteamAPI_ISteamApps_UninstallDLC(ISteamApps* self, AppId_t a0) {
    SF_FLAT("SteamAPI_ISteamApps_UninstallDLC", true);
    if (self)
        self->UninstallDLC(a0);
}

void SteamAPI_ISteamApps_RequestAppProofOfPurchaseKey(ISteamApps* self, AppId_t a0) {
    SF_FLAT("SteamAPI_ISteamApps_RequestAppProofOfPurchaseKey", true);
    if (self)
        self->RequestAppProofOfPurchaseKey(a0);
}

bool SteamAPI_ISteamApps_GetCurrentBetaName(ISteamApps* self, char* a0, int a1) {
    SF_FLAT("SteamAPI_ISteamApps_GetCurrentBetaName", true);
    if (!self)
        return {};
    return self->GetCurrentBetaName(a0, a1);
}

bool SteamAPI_ISteamApps_MarkContentCorrupt(ISteamApps* self, bool a0) {
    SF_FLAT("SteamAPI_ISteamApps_MarkContentCorrupt", true);
    if (!self)
        return {};
    return self->MarkContentCorrupt(a0);
}

uint32_t SteamAPI_ISteamApps_GetInstalledDepots(ISteamApps* self, AppId_t a0, DepotId_t* a1, uint32_t a2) {
    SF_FLAT("SteamAPI_ISteamApps_GetInstalledDepots", true);
    if (!self)
        return {};
    return self->GetInstalledDepots(a0, a1, a2);
}

uint32_t SteamAPI_ISteamApps_GetAppInstallDir(ISteamApps* self, AppId_t a0, char* a1, uint32_t a2) {
    SF_FLAT("SteamAPI_ISteamApps_GetAppInstallDir", true);
    if (!self)
        return {};
    return self->GetAppInstallDir(a0, a1, a2);
}

bool SteamAPI_ISteamApps_BIsAppInstalled(ISteamApps* self, AppId_t a0) {
    SF_FLAT("SteamAPI_ISteamApps_BIsAppInstalled", true);
    if (!self)
        return {};
    return self->BIsAppInstalled(a0);
}

CSteamID SteamAPI_ISteamApps_GetAppOwner(ISteamApps* self) {
    SF_FLAT("SteamAPI_ISteamApps_GetAppOwner", true);
    if (!self)
        return {};
    return self->GetAppOwner();
}

const char* SteamAPI_ISteamApps_GetLaunchQueryParam(ISteamApps* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamApps_GetLaunchQueryParam", true);
    if (!self)
        return {};
    return self->GetLaunchQueryParam(a0);
}

bool SteamAPI_ISteamApps_GetDlcDownloadProgress(ISteamApps* self, AppId_t a0, uint64_t* a1, uint64_t* a2) {
    SF_FLAT("SteamAPI_ISteamApps_GetDlcDownloadProgress", true);
    if (!self)
        return {};
    return self->GetDlcDownloadProgress(a0, a1, a2);
}

int SteamAPI_ISteamApps_GetAppBuildId(ISteamApps* self) {
    SF_FLAT("SteamAPI_ISteamApps_GetAppBuildId", true);
    if (!self)
        return {};
    return self->GetAppBuildId();
}

void SteamAPI_ISteamApps_RequestAllProofOfPurchaseKeys(ISteamApps* self) {
    SF_FLAT("SteamAPI_ISteamApps_RequestAllProofOfPurchaseKeys", true);
    if (self)
        self->RequestAllProofOfPurchaseKeys();
}

SteamAPICall_t SteamAPI_ISteamApps_GetFileDetails(ISteamApps* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamApps_GetFileDetails", true);
    if (!self)
        return {};
    return self->GetFileDetails(a0);
}

int SteamAPI_ISteamApps_GetLaunchCommandLine(ISteamApps* self, char* a0, int a1) {
    SF_FLAT("SteamAPI_ISteamApps_GetLaunchCommandLine", true);
    if (!self)
        return {};
    return self->GetLaunchCommandLine(a0, a1);
}

bool SteamAPI_ISteamApps_BIsSubscribedFromFamilySharing(ISteamApps* self) {
    SF_FLAT("SteamAPI_ISteamApps_BIsSubscribedFromFamilySharing", true);
    if (!self)
        return {};
    return self->BIsSubscribedFromFamilySharing();
}

bool SteamAPI_ISteamApps_BIsTimedTrial(ISteamApps* self, uint32_t* a0, uint32_t* a1) {
    SF_FLAT("SteamAPI_ISteamApps_BIsTimedTrial", true);
    if (!self)
        return {};
    return self->BIsTimedTrial(a0, a1);
}

bool SteamAPI_ISteamApps_SetDlcContext(ISteamApps* self, AppId_t a0) {
    SF_FLAT("SteamAPI_ISteamApps_SetDlcContext", true);
    if (!self)
        return {};
    return self->SetDlcContext(a0);
}

int SteamAPI_ISteamApps_GetNumBetas(ISteamApps* self, int* a0, int* a1) {
    SF_FLAT("SteamAPI_ISteamApps_GetNumBetas", true);
    if (!self)
        return {};
    return self->GetNumBetas(a0, a1);
}

bool SteamAPI_ISteamApps_GetBetaInfo(ISteamApps* self, int a0, char* a1, int a2, char* a3, int a4, bool* a5, char* a6, int a7) {
    SF_FLAT("SteamAPI_ISteamApps_GetBetaInfo", true);
    if (!self)
        return {};
    return self->GetBetaInfo(a0, a1, a2, a3, a4, a5, a6, a7);
}

bool SteamAPI_ISteamApps_SetActiveBeta(ISteamApps* self, const char* a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamApps_SetActiveBeta", true);
    if (!self)
        return {};
    return self->SetActiveBeta(a0, a1);
}

UGCQueryHandle_t SteamAPI_ISteamUGC_CreateQueryUserUGCRequest(ISteamUGC* self, AccountID_t a0, int a1, int a2, int a3, AppId_t a4, AppId_t a5, uint32_t a6) {
    SF_FLAT("SteamAPI_ISteamUGC_CreateQueryUserUGCRequest", true);
    if (!self)
        return {};
    return self->CreateQueryUserUGCRequest(a0, a1, a2, a3, a4, a5, a6);
}

UGCQueryHandle_t SteamAPI_ISteamUGC_CreateQueryAllUGCRequestPage(ISteamUGC* self, int a0, int a1, AppId_t a2, AppId_t a3, uint32_t a4) {
    SF_FLAT("SteamAPI_ISteamUGC_CreateQueryAllUGCRequestPage", true);
    if (!self)
        return {};
    return self->CreateQueryAllUGCRequest(a0, a1, a2, a3, a4);
}

UGCQueryHandle_t SteamAPI_ISteamUGC_CreateQueryUGCDetailsRequest(ISteamUGC* self, PublishedFileId_t* a0, uint32_t a1) {
    SF_FLAT("SteamAPI_ISteamUGC_CreateQueryUGCDetailsRequest", true);
    if (!self)
        return {};
    return self->CreateQueryUGCDetailsRequest(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamUGC_SendQueryUGCRequest(ISteamUGC* self, UGCQueryHandle_t a0) {
    SF_FLAT("SteamAPI_ISteamUGC_SendQueryUGCRequest", true);
    if (!self)
        return {};
    return self->SendQueryUGCRequest(a0);
}

bool SteamAPI_ISteamUGC_GetQueryUGCResult(ISteamUGC* self, UGCQueryHandle_t a0, uint32_t a1, SteamUGCDetails_t* a2) {
    SF_FLAT("SteamAPI_ISteamUGC_GetQueryUGCResult", true);
    if (!self)
        return {};
    return self->GetQueryUGCResult(a0, a1, a2);
}

bool SteamAPI_ISteamUGC_GetQueryUGCPreviewURL(ISteamUGC* self, UGCQueryHandle_t a0, uint32_t a1, char* a2, uint32_t a3) {
    SF_FLAT("SteamAPI_ISteamUGC_GetQueryUGCPreviewURL", true);
    if (!self)
        return {};
    return self->GetQueryUGCPreviewURL(a0, a1, a2, a3);
}

bool SteamAPI_ISteamUGC_GetQueryUGCMetadataURL(ISteamUGC* self, UGCQueryHandle_t a0, uint32_t a1, char* a2, uint32_t a3) {
    SF_FLAT("SteamAPI_ISteamUGC_GetQueryUGCMetadataURL", true);
    if (!self)
        return {};
    return self->GetQueryUGCMetadataURL(a0, a1, a2, a3);
}

bool SteamAPI_ISteamUGC_GetQueryUGCChildren(ISteamUGC* self, UGCQueryHandle_t a0, uint32_t a1, PublishedFileId_t* a2, uint32_t a3) {
    SF_FLAT("SteamAPI_ISteamUGC_GetQueryUGCChildren", true);
    if (!self)
        return {};
    return self->GetQueryUGCChildren(a0, a1, a2, a3);
}

bool SteamAPI_ISteamUGC_GetQueryUGCStatistic(ISteamUGC* self, UGCQueryHandle_t a0, uint32_t a1, int a2, uint64_t* a3) {
    SF_FLAT("SteamAPI_ISteamUGC_GetQueryUGCStatistic", true);
    if (!self)
        return {};
    return self->GetQueryUGCStatistic(a0, a1, a2, a3);
}

uint32_t SteamAPI_ISteamUGC_GetQueryUGCNumAdditionalPreviews(ISteamUGC* self, UGCQueryHandle_t a0, uint32_t a1) {
    SF_FLAT("SteamAPI_ISteamUGC_GetQueryUGCNumAdditionalPreviews", true);
    if (!self)
        return {};
    return self->GetQueryUGCNumAdditionalPreviews(a0, a1);
}

bool SteamAPI_ISteamUGC_GetQueryUGCAdditionalPreview(ISteamUGC* self, UGCQueryHandle_t a0, uint32_t a1, uint32_t a2, char* a3, uint32_t a4, char* a5, uint32_t a6, int* a7) {
    SF_FLAT("SteamAPI_ISteamUGC_GetQueryUGCAdditionalPreview", true);
    if (!self)
        return {};
    return self->GetQueryUGCAdditionalPreview(a0, a1, a2, a3, a4, a5, a6, a7);
}

uint32_t SteamAPI_ISteamUGC_GetQueryUGCNumKeyValueTags(ISteamUGC* self, UGCQueryHandle_t a0, uint32_t a1) {
    SF_FLAT("SteamAPI_ISteamUGC_GetQueryUGCNumKeyValueTags", true);
    if (!self)
        return {};
    return self->GetQueryUGCNumKeyValueTags(a0, a1);
}

bool SteamAPI_ISteamUGC_GetQueryUGCKeyValueTag(ISteamUGC* self, UGCQueryHandle_t a0, uint32_t a1, uint32_t a2, char* a3, uint32_t a4, char* a5, uint32_t a6) {
    SF_FLAT("SteamAPI_ISteamUGC_GetQueryUGCKeyValueTag", true);
    if (!self)
        return {};
    return self->GetQueryUGCKeyValueTag(a0, a1, a2, a3, a4, a5, a6);
}

uint32_t SteamAPI_ISteamUGC_GetQueryUGCContentDescriptors(ISteamUGC* self, UGCQueryHandle_t a0, uint32_t a1, uint32_t* a2, uint32_t a3) {
    SF_FLAT("SteamAPI_ISteamUGC_GetQueryUGCContentDescriptors", true);
    if (!self)
        return {};
    return self->GetQueryUGCContentDescriptors(a0, a1, a2, a3);
}

bool SteamAPI_ISteamUGC_ReleaseQueryUGCRequest(ISteamUGC* self, UGCQueryHandle_t a0) {
    SF_FLAT("SteamAPI_ISteamUGC_ReleaseQueryUGCRequest", true);
    if (!self)
        return {};
    return self->ReleaseQueryUGCRequest(a0);
}

bool SteamAPI_ISteamUGC_AddRequiredTag(ISteamUGC* self, UGCQueryHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamUGC_AddRequiredTag", true);
    if (!self)
        return {};
    return self->AddRequiredTag(a0, a1);
}

bool SteamAPI_ISteamUGC_AddRequiredTagGroup(ISteamUGC* self, UGCQueryHandle_t a0, const SteamParamStringArray_t* a1) {
    SF_FLAT("SteamAPI_ISteamUGC_AddRequiredTagGroup", true);
    if (!self)
        return {};
    return self->AddRequiredTagGroup(a0, a1);
}

bool SteamAPI_ISteamUGC_AddExcludedTag(ISteamUGC* self, UGCQueryHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamUGC_AddExcludedTag", true);
    if (!self)
        return {};
    return self->AddExcludedTag(a0, a1);
}

bool SteamAPI_ISteamUGC_SetReturnOnlyIDs(ISteamUGC* self, UGCQueryHandle_t a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetReturnOnlyIDs", true);
    if (!self)
        return {};
    return self->SetReturnOnlyIDs(a0, a1);
}

bool SteamAPI_ISteamUGC_SetReturnKeyValueTags(ISteamUGC* self, UGCQueryHandle_t a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetReturnKeyValueTags", true);
    if (!self)
        return {};
    return self->SetReturnKeyValueTags(a0, a1);
}

bool SteamAPI_ISteamUGC_SetReturnLongDescription(ISteamUGC* self, UGCQueryHandle_t a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetReturnLongDescription", true);
    if (!self)
        return {};
    return self->SetReturnLongDescription(a0, a1);
}

bool SteamAPI_ISteamUGC_SetReturnMetadata(ISteamUGC* self, UGCQueryHandle_t a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetReturnMetadata", true);
    if (!self)
        return {};
    return self->SetReturnMetadata(a0, a1);
}

bool SteamAPI_ISteamUGC_SetReturnChildren(ISteamUGC* self, UGCQueryHandle_t a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetReturnChildren", true);
    if (!self)
        return {};
    return self->SetReturnChildren(a0, a1);
}

bool SteamAPI_ISteamUGC_SetReturnAdditionalPreviews(ISteamUGC* self, UGCQueryHandle_t a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetReturnAdditionalPreviews", true);
    if (!self)
        return {};
    return self->SetReturnAdditionalPreviews(a0, a1);
}

bool SteamAPI_ISteamUGC_SetReturnTotalOnly(ISteamUGC* self, UGCQueryHandle_t a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetReturnTotalOnly", true);
    if (!self)
        return {};
    return self->SetReturnTotalOnly(a0, a1);
}

bool SteamAPI_ISteamUGC_SetReturnPlaytimeStats(ISteamUGC* self, UGCQueryHandle_t a0, uint32_t a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetReturnPlaytimeStats", true);
    if (!self)
        return {};
    return self->SetReturnPlaytimeStats(a0, a1);
}

bool SteamAPI_ISteamUGC_SetLanguage(ISteamUGC* self, UGCQueryHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetLanguage", true);
    if (!self)
        return {};
    return self->SetLanguage(a0, a1);
}

bool SteamAPI_ISteamUGC_SetAllowCachedResponse(ISteamUGC* self, UGCQueryHandle_t a0, uint32_t a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetAllowCachedResponse", true);
    if (!self)
        return {};
    return self->SetAllowCachedResponse(a0, a1);
}

bool SteamAPI_ISteamUGC_SetCloudFileNameFilter(ISteamUGC* self, UGCQueryHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetCloudFileNameFilter", true);
    if (!self)
        return {};
    return self->SetCloudFileNameFilter(a0, a1);
}

bool SteamAPI_ISteamUGC_SetMatchAnyTag(ISteamUGC* self, UGCQueryHandle_t a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetMatchAnyTag", true);
    if (!self)
        return {};
    return self->SetMatchAnyTag(a0, a1);
}

bool SteamAPI_ISteamUGC_SetSearchText(ISteamUGC* self, UGCQueryHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetSearchText", true);
    if (!self)
        return {};
    return self->SetSearchText(a0, a1);
}

bool SteamAPI_ISteamUGC_SetRankedByTrendDays(ISteamUGC* self, UGCQueryHandle_t a0, uint32_t a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetRankedByTrendDays", true);
    if (!self)
        return {};
    return self->SetRankedByTrendDays(a0, a1);
}

bool SteamAPI_ISteamUGC_SetTimeCreatedFileId(ISteamUGC* self, UGCQueryHandle_t a0, PublishedFileId_t a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetTimeCreatedFileId", true);
    if (!self)
        return {};
    return self->SetTimeCreatedFileId(a0, a1);
}

bool SteamAPI_ISteamUGC_SetTimeUpdatedFileId(ISteamUGC* self, UGCQueryHandle_t a0, PublishedFileId_t a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetTimeUpdatedFileId", true);
    if (!self)
        return {};
    return self->SetTimeUpdatedFileId(a0, a1);
}

bool SteamAPI_ISteamUGC_AddRequiredKeyValueTag(ISteamUGC* self, UGCQueryHandle_t a0, const char* a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamUGC_AddRequiredKeyValueTag", true);
    if (!self)
        return {};
    return self->AddRequiredKeyValueTag(a0, a1, a2);
}

SteamAPICall_t SteamAPI_ISteamUGC_RequestUGCDetails(ISteamUGC* self, PublishedFileId_t a0, uint32_t a1) {
    SF_FLAT("SteamAPI_ISteamUGC_RequestUGCDetails", true);
    if (!self)
        return {};
    return self->RequestUGCDetails(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamUGC_CreateItem(ISteamUGC* self, AppId_t a0, int a1) {
    SF_FLAT("SteamAPI_ISteamUGC_CreateItem", true);
    if (!self)
        return {};
    return self->CreateItem(a0, a1);
}

UGCUpdateHandle_t SteamAPI_ISteamUGC_StartItemUpdate(ISteamUGC* self, AppId_t a0, PublishedFileId_t a1) {
    SF_FLAT("SteamAPI_ISteamUGC_StartItemUpdate", true);
    if (!self)
        return {};
    return self->StartItemUpdate(a0, a1);
}

bool SteamAPI_ISteamUGC_SetItemTitle(ISteamUGC* self, UGCUpdateHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetItemTitle", true);
    if (!self)
        return {};
    return self->SetItemTitle(a0, a1);
}

bool SteamAPI_ISteamUGC_SetItemDescription(ISteamUGC* self, UGCUpdateHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetItemDescription", true);
    if (!self)
        return {};
    return self->SetItemDescription(a0, a1);
}

bool SteamAPI_ISteamUGC_SetItemUpdateLanguage(ISteamUGC* self, UGCUpdateHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetItemUpdateLanguage", true);
    if (!self)
        return {};
    return self->SetItemUpdateLanguage(a0, a1);
}

bool SteamAPI_ISteamUGC_SetItemMetadata(ISteamUGC* self, UGCUpdateHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetItemMetadata", true);
    if (!self)
        return {};
    return self->SetItemMetadata(a0, a1);
}

bool SteamAPI_ISteamUGC_SetItemVisibility(ISteamUGC* self, UGCUpdateHandle_t a0, int a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetItemVisibility", true);
    if (!self)
        return {};
    return self->SetItemVisibility(a0, a1);
}

bool SteamAPI_ISteamUGC_SetItemTags(ISteamUGC* self, UGCUpdateHandle_t a0, const SteamParamStringArray_t* a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetItemTags", true);
    if (!self)
        return {};
    return self->SetItemTags(a0, a1);
}

bool SteamAPI_ISteamUGC_SetItemContent(ISteamUGC* self, UGCUpdateHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetItemContent", true);
    if (!self)
        return {};
    return self->SetItemContent(a0, a1);
}

bool SteamAPI_ISteamUGC_SetItemPreview(ISteamUGC* self, UGCUpdateHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetItemPreview", true);
    if (!self)
        return {};
    return self->SetItemPreview(a0, a1);
}

bool SteamAPI_ISteamUGC_SetAllowLegacyUpload(ISteamUGC* self, UGCUpdateHandle_t a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetAllowLegacyUpload", true);
    if (!self)
        return {};
    return self->SetAllowLegacyUpload(a0, a1);
}

bool SteamAPI_ISteamUGC_RemoveAllItemKeyValueTags(ISteamUGC* self, UGCUpdateHandle_t a0) {
    SF_FLAT("SteamAPI_ISteamUGC_RemoveAllItemKeyValueTags", true);
    if (!self)
        return {};
    return self->RemoveAllItemKeyValueTags(a0);
}

bool SteamAPI_ISteamUGC_RemoveItemKeyValueTags(ISteamUGC* self, UGCUpdateHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamUGC_RemoveItemKeyValueTags", true);
    if (!self)
        return {};
    return self->RemoveItemKeyValueTags(a0, a1);
}

bool SteamAPI_ISteamUGC_AddItemKeyValueTag(ISteamUGC* self, UGCUpdateHandle_t a0, const char* a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamUGC_AddItemKeyValueTag", true);
    if (!self)
        return {};
    return self->AddItemKeyValueTag(a0, a1, a2);
}

bool SteamAPI_ISteamUGC_AddItemPreviewFile(ISteamUGC* self, UGCUpdateHandle_t a0, const char* a1, int a2) {
    SF_FLAT("SteamAPI_ISteamUGC_AddItemPreviewFile", true);
    if (!self)
        return {};
    return self->AddItemPreviewFile(a0, a1, a2);
}

bool SteamAPI_ISteamUGC_AddItemPreviewVideo(ISteamUGC* self, UGCUpdateHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamUGC_AddItemPreviewVideo", true);
    if (!self)
        return {};
    return self->AddItemPreviewVideo(a0, a1);
}

bool SteamAPI_ISteamUGC_UpdateItemPreviewFile(ISteamUGC* self, UGCUpdateHandle_t a0, uint32_t a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamUGC_UpdateItemPreviewFile", true);
    if (!self)
        return {};
    return self->UpdateItemPreviewFile(a0, a1, a2);
}

bool SteamAPI_ISteamUGC_UpdateItemPreviewVideo(ISteamUGC* self, UGCUpdateHandle_t a0, uint32_t a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamUGC_UpdateItemPreviewVideo", true);
    if (!self)
        return {};
    return self->UpdateItemPreviewVideo(a0, a1, a2);
}

bool SteamAPI_ISteamUGC_RemoveItemPreview(ISteamUGC* self, UGCUpdateHandle_t a0, uint32_t a1) {
    SF_FLAT("SteamAPI_ISteamUGC_RemoveItemPreview", true);
    if (!self)
        return {};
    return self->RemoveItemPreview(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamUGC_SubmitItemUpdate(ISteamUGC* self, UGCUpdateHandle_t a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SubmitItemUpdate", true);
    if (!self)
        return {};
    return self->SubmitItemUpdate(a0, a1);
}

int SteamAPI_ISteamUGC_GetItemUpdateProgress(ISteamUGC* self, UGCUpdateHandle_t a0, uint64_t* a1, uint64_t* a2) {
    SF_FLAT("SteamAPI_ISteamUGC_GetItemUpdateProgress", true);
    if (!self)
        return {};
    return self->GetItemUpdateProgress(a0, a1, a2);
}

SteamAPICall_t SteamAPI_ISteamUGC_SetUserItemVote(ISteamUGC* self, PublishedFileId_t a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetUserItemVote", true);
    if (!self)
        return {};
    return self->SetUserItemVote(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamUGC_GetUserItemVote(ISteamUGC* self, PublishedFileId_t a0) {
    SF_FLAT("SteamAPI_ISteamUGC_GetUserItemVote", true);
    if (!self)
        return {};
    return self->GetUserItemVote(a0);
}

SteamAPICall_t SteamAPI_ISteamUGC_AddItemToFavorites(ISteamUGC* self, AppId_t a0, PublishedFileId_t a1) {
    SF_FLAT("SteamAPI_ISteamUGC_AddItemToFavorites", true);
    if (!self)
        return {};
    return self->AddItemToFavorites(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamUGC_RemoveItemFromFavorites(ISteamUGC* self, AppId_t a0, PublishedFileId_t a1) {
    SF_FLAT("SteamAPI_ISteamUGC_RemoveItemFromFavorites", true);
    if (!self)
        return {};
    return self->RemoveItemFromFavorites(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamUGC_SubscribeItem(ISteamUGC* self, PublishedFileId_t a0) {
    SF_FLAT("SteamAPI_ISteamUGC_SubscribeItem", true);
    if (!self)
        return {};
    return self->SubscribeItem(a0);
}

SteamAPICall_t SteamAPI_ISteamUGC_UnsubscribeItem(ISteamUGC* self, PublishedFileId_t a0) {
    SF_FLAT("SteamAPI_ISteamUGC_UnsubscribeItem", true);
    if (!self)
        return {};
    return self->UnsubscribeItem(a0);
}

uint32_t SteamAPI_ISteamUGC_GetNumSubscribedItems(ISteamUGC* self) {
    SF_FLAT("SteamAPI_ISteamUGC_GetNumSubscribedItems", true);
    if (!self)
        return {};
    return self->GetNumSubscribedItems();
}

uint32_t SteamAPI_ISteamUGC_GetSubscribedItems(ISteamUGC* self, PublishedFileId_t* a0, uint32_t a1) {
    SF_FLAT("SteamAPI_ISteamUGC_GetSubscribedItems", true);
    if (!self)
        return {};
    return self->GetSubscribedItems(a0, a1);
}

uint32_t SteamAPI_ISteamUGC_GetItemState(ISteamUGC* self, PublishedFileId_t a0) {
    SF_FLAT("SteamAPI_ISteamUGC_GetItemState", true);
    if (!self)
        return {};
    return self->GetItemState(a0);
}

bool SteamAPI_ISteamUGC_GetItemInstallInfo(ISteamUGC* self, PublishedFileId_t a0, uint64_t* a1, char* a2, uint32_t a3, uint32_t* a4) {
    SF_FLAT("SteamAPI_ISteamUGC_GetItemInstallInfo", true);
    if (!self)
        return {};
    return self->GetItemInstallInfo(a0, a1, a2, a3, a4);
}

bool SteamAPI_ISteamUGC_GetItemDownloadInfo(ISteamUGC* self, PublishedFileId_t a0, uint64_t* a1, uint64_t* a2) {
    SF_FLAT("SteamAPI_ISteamUGC_GetItemDownloadInfo", true);
    if (!self)
        return {};
    return self->GetItemDownloadInfo(a0, a1, a2);
}

bool SteamAPI_ISteamUGC_DeleteItem(ISteamUGC* self, PublishedFileId_t a0) {
    SF_FLAT("SteamAPI_ISteamUGC_DeleteItem", true);
    if (!self)
        return {};
    return self->DeleteItem(a0);
}

bool SteamAPI_ISteamUGC_SetItemTaxonomyTags(ISteamUGC* self, UGCUpdateHandle_t a0, const SteamParamStringArray_t* a1) {
    SF_FLAT("SteamAPI_ISteamUGC_SetItemTaxonomyTags", true);
    if (!self)
        return {};
    return self->SetItemTaxonomyTags(a0, a1);
}

bool SteamAPI_ISteamUGC_GetItemTaxonomyTags(ISteamUGC* self, UGCUpdateHandle_t a0, uint32_t a1, uint32_t a2, char* a3, uint32_t a4) {
    SF_FLAT("SteamAPI_ISteamUGC_GetItemTaxonomyTags", true);
    if (!self)
        return {};
    return self->GetItemTaxonomyTags(a0, a1, a2, a3, a4);
}

EResult SteamAPI_ISteamInventory_GetResultStatus(ISteamInventory* self, SteamInventoryResult_t a0) {
    SF_FLAT("SteamAPI_ISteamInventory_GetResultStatus", true);
    if (!self)
        return {};
    return self->GetResultStatus(a0);
}

bool SteamAPI_ISteamInventory_GetResultItems(ISteamInventory* self, SteamInventoryResult_t a0, SteamItemDetails_t* a1, uint32_t a2) {
    SF_FLAT("SteamAPI_ISteamInventory_GetResultItems", true);
    if (!self)
        return {};
    return self->GetResultItems(a0, a1, a2);
}

bool SteamAPI_ISteamInventory_GetResultItemProperty(ISteamInventory* self, SteamInventoryResult_t a0, uint32_t a1, const char* a2, char* a3, uint32_t* a4) {
    SF_FLAT("SteamAPI_ISteamInventory_GetResultItemProperty", true);
    if (!self)
        return {};
    return self->GetResultItemProperty(a0, a1, a2, a3, a4);
}

uint32_t SteamAPI_ISteamInventory_GetResultTimestamp(ISteamInventory* self, SteamInventoryResult_t a0) {
    SF_FLAT("SteamAPI_ISteamInventory_GetResultTimestamp", true);
    if (!self)
        return {};
    return self->GetResultTimestamp(a0);
}

bool SteamAPI_ISteamInventory_CheckResultSteamID(ISteamInventory* self, SteamInventoryResult_t a0, CSteamID a1) {
    SF_FLAT("SteamAPI_ISteamInventory_CheckResultSteamID", true);
    if (!self)
        return {};
    return self->CheckResultSteamID(a0, a1);
}

void SteamAPI_ISteamInventory_DestroyResult(ISteamInventory* self, SteamInventoryResult_t a0) {
    SF_FLAT("SteamAPI_ISteamInventory_DestroyResult", true);
    if (self)
        self->DestroyResult(a0);
}

bool SteamAPI_ISteamInventory_GetAllItems(ISteamInventory* self, SteamInventoryResult_t* a0) {
    SF_FLAT("SteamAPI_ISteamInventory_GetAllItems", true);
    if (!self)
        return {};
    return self->GetAllItems(a0);
}

bool SteamAPI_ISteamInventory_GetItemsByID(ISteamInventory* self, SteamInventoryResult_t* a0, const SteamItemInstanceID_t* a1, uint32_t a2) {
    SF_FLAT("SteamAPI_ISteamInventory_GetItemsByID", true);
    if (!self)
        return {};
    return self->GetItemsByID(a0, a1, a2);
}

bool SteamAPI_ISteamInventory_SerializeResult(ISteamInventory* self, SteamInventoryResult_t a0, void* a1, uint32_t* a2) {
    SF_FLAT("SteamAPI_ISteamInventory_SerializeResult", true);
    if (!self)
        return {};
    return self->SerializeResult(a0, a1, a2);
}

bool SteamAPI_ISteamInventory_DeserializeResult(ISteamInventory* self, SteamInventoryResult_t* a0, const void* a1, uint32_t a2, bool a3) {
    SF_FLAT("SteamAPI_ISteamInventory_DeserializeResult", true);
    if (!self)
        return {};
    return self->DeserializeResult(a0, a1, a2, a3);
}

bool SteamAPI_ISteamInventory_GenerateItems(ISteamInventory* self, SteamInventoryResult_t* a0, const SteamItemDef_t* a1, const uint32_t* a2, uint32_t a3) {
    SF_FLAT("SteamAPI_ISteamInventory_GenerateItems", true);
    if (!self)
        return {};
    return self->GenerateItems(a0, a1, a2, a3);
}

bool SteamAPI_ISteamInventory_GrantPromoItems(ISteamInventory* self, SteamInventoryResult_t* a0) {
    SF_FLAT("SteamAPI_ISteamInventory_GrantPromoItems", true);
    if (!self)
        return {};
    return self->GrantPromoItems(a0);
}

bool SteamAPI_ISteamInventory_AddPromoItem(ISteamInventory* self, SteamInventoryResult_t* a0, SteamItemDef_t a1) {
    SF_FLAT("SteamAPI_ISteamInventory_AddPromoItem", true);
    if (!self)
        return {};
    return self->AddPromoItem(a0, a1);
}

bool SteamAPI_ISteamInventory_AddPromoItems(ISteamInventory* self, SteamInventoryResult_t* a0, const SteamItemDef_t* a1, uint32_t a2) {
    SF_FLAT("SteamAPI_ISteamInventory_AddPromoItems", true);
    if (!self)
        return {};
    return self->AddPromoItems(a0, a1, a2);
}

bool SteamAPI_ISteamInventory_ConsumeItem(ISteamInventory* self, SteamInventoryResult_t* a0, SteamItemInstanceID_t a1, uint32_t a2) {
    SF_FLAT("SteamAPI_ISteamInventory_ConsumeItem", true);
    if (!self)
        return {};
    return self->ConsumeItem(a0, a1, a2);
}

bool SteamAPI_ISteamInventory_ExchangeItems(ISteamInventory* self, SteamInventoryResult_t* a0, const SteamItemDef_t* a1, const uint32_t* a2, uint32_t a3, const SteamItemInstanceID_t* a4, const uint32_t* a5, uint32_t a6) {
    SF_FLAT("SteamAPI_ISteamInventory_ExchangeItems", true);
    if (!self)
        return {};
    return self->ExchangeItems(a0, a1, a2, a3, a4, a5, a6);
}

bool SteamAPI_ISteamInventory_TransferItemQuantity(ISteamInventory* self, SteamInventoryResult_t* a0, SteamItemInstanceID_t a1, uint32_t a2, SteamItemInstanceID_t a3) {
    SF_FLAT("SteamAPI_ISteamInventory_TransferItemQuantity", true);
    if (!self)
        return {};
    return self->TransferItemQuantity(a0, a1, a2, a3);
}

void SteamAPI_ISteamInventory_SendItemDropHeartbeat(ISteamInventory* self) {
    SF_FLAT("SteamAPI_ISteamInventory_SendItemDropHeartbeat", true);
    if (self)
        self->SendItemDropHeartbeat();
}

bool SteamAPI_ISteamInventory_TriggerItemDrop(ISteamInventory* self, SteamInventoryResult_t* a0, SteamItemDef_t a1) {
    SF_FLAT("SteamAPI_ISteamInventory_TriggerItemDrop", true);
    if (!self)
        return {};
    return self->TriggerItemDrop(a0, a1);
}

bool SteamAPI_ISteamInventory_TradeItems(ISteamInventory* self, SteamInventoryResult_t* a0, CSteamID a1, const SteamItemInstanceID_t* a2, const uint32_t* a3, uint32_t a4, const SteamItemInstanceID_t* a5, const uint32_t* a6, uint32_t a7) {
    SF_FLAT("SteamAPI_ISteamInventory_TradeItems", true);
    if (!self)
        return {};
    return self->TradeItems(a0, a1, a2, a3, a4, a5, a6, a7);
}

bool SteamAPI_ISteamInventory_LoadItemDefinitions(ISteamInventory* self) {
    SF_FLAT("SteamAPI_ISteamInventory_LoadItemDefinitions", true);
    if (!self)
        return {};
    return self->LoadItemDefinitions();
}

bool SteamAPI_ISteamInventory_GetItemDefinitionIDs(ISteamInventory* self, SteamItemDef_t* a0, uint32_t* a1) {
    SF_FLAT("SteamAPI_ISteamInventory_GetItemDefinitionIDs", true);
    if (!self)
        return {};
    return self->GetItemDefinitionIDs(a0, a1);
}

bool SteamAPI_ISteamInventory_GetItemDefinitionProperty(ISteamInventory* self, SteamItemDef_t a0, const char* a1, char* a2, uint32_t* a3) {
    SF_FLAT("SteamAPI_ISteamInventory_GetItemDefinitionProperty", true);
    if (!self)
        return {};
    return self->GetItemDefinitionProperty(a0, a1, a2, a3);
}

SteamAPICall_t SteamAPI_ISteamInventory_RequestEligiblePromoItemDefinitionsIDs(ISteamInventory* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamInventory_RequestEligiblePromoItemDefinitionsIDs", true);
    if (!self)
        return {};
    return self->RequestEligiblePromoItemDefinitionsIDs(a0);
}

bool SteamAPI_ISteamInventory_GetEligiblePromoItemDefinitionIDs(ISteamInventory* self, CSteamID a0, SteamItemDef_t* a1, uint32_t* a2) {
    SF_FLAT("SteamAPI_ISteamInventory_GetEligiblePromoItemDefinitionIDs", true);
    if (!self)
        return {};
    return self->GetEligiblePromoItemDefinitionIDs(a0, a1, a2);
}

SteamAPICall_t SteamAPI_ISteamInventory_StartPurchase(ISteamInventory* self, const SteamItemDef_t* a0, const uint32_t* a1, uint32_t a2) {
    SF_FLAT("SteamAPI_ISteamInventory_StartPurchase", true);
    if (!self)
        return {};
    return self->StartPurchase(a0, a1, a2);
}

SteamAPICall_t SteamAPI_ISteamInventory_RequestPrices(ISteamInventory* self) {
    SF_FLAT("SteamAPI_ISteamInventory_RequestPrices", true);
    if (!self)
        return {};
    return self->RequestPrices();
}

uint32_t SteamAPI_ISteamInventory_GetNumItemsWithPrices(ISteamInventory* self) {
    SF_FLAT("SteamAPI_ISteamInventory_GetNumItemsWithPrices", true);
    if (!self)
        return {};
    return self->GetNumItemsWithPrices();
}

bool SteamAPI_ISteamInventory_GetItemsWithPrices(ISteamInventory* self, SteamItemDef_t* a0, uint64_t* a1, uint64_t* a2, uint32_t a3) {
    SF_FLAT("SteamAPI_ISteamInventory_GetItemsWithPrices", true);
    if (!self)
        return {};
    return self->GetItemsWithPrices(a0, a1, a2, a3);
}

bool SteamAPI_ISteamInventory_GetItemPrice(ISteamInventory* self, SteamItemDef_t a0, uint64_t* a1, uint64_t* a2) {
    SF_FLAT("SteamAPI_ISteamInventory_GetItemPrice", true);
    if (!self)
        return {};
    return self->GetItemPrice(a0, a1, a2);
}

SteamInventoryUpdateHandle_t SteamAPI_ISteamInventory_StartUpdateProperties(ISteamInventory* self) {
    SF_FLAT("SteamAPI_ISteamInventory_StartUpdateProperties", true);
    if (!self)
        return {};
    return self->StartUpdateProperties();
}

bool SteamAPI_ISteamInventory_RemoveProperty(ISteamInventory* self, SteamInventoryUpdateHandle_t a0, SteamItemInstanceID_t a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamInventory_RemoveProperty", true);
    if (!self)
        return {};
    return self->RemoveProperty(a0, a1, a2);
}

bool SteamAPI_ISteamInventory_SetPropertyBool(ISteamInventory* self, SteamInventoryUpdateHandle_t a0, SteamItemInstanceID_t a1, const char* a2, bool a3) {
    SF_FLAT("SteamAPI_ISteamInventory_SetPropertyBool", true);
    if (!self)
        return {};
    return self->SetProperty(a0, a1, a2, a3);
}

bool SteamAPI_ISteamInventory_SetPropertyInt64(ISteamInventory* self, SteamInventoryUpdateHandle_t a0, SteamItemInstanceID_t a1, const char* a2, int64_t a3) {
    SF_FLAT("SteamAPI_ISteamInventory_SetPropertyInt64", true);
    if (!self)
        return {};
    return self->SetProperty(a0, a1, a2, a3);
}

bool SteamAPI_ISteamInventory_SetPropertyFloat(ISteamInventory* self, SteamInventoryUpdateHandle_t a0, SteamItemInstanceID_t a1, const char* a2, float a3) {
    SF_FLAT("SteamAPI_ISteamInventory_SetPropertyFloat", true);
    if (!self)
        return {};
    return self->SetProperty(a0, a1, a2, a3);
}

bool SteamAPI_ISteamInventory_SetPropertyString(ISteamInventory* self, SteamInventoryUpdateHandle_t a0, SteamItemInstanceID_t a1, const char* a2, const char* a3) {
    SF_FLAT("SteamAPI_ISteamInventory_SetPropertyString", true);
    if (!self)
        return {};
    return self->SetProperty(a0, a1, a2, a3);
}

bool SteamAPI_ISteamInventory_SubmitUpdateProperties(ISteamInventory* self, SteamInventoryUpdateHandle_t a0, SteamInventoryResult_t* a1) {
    SF_FLAT("SteamAPI_ISteamInventory_SubmitUpdateProperties", true);
    if (!self)
        return {};
    return self->SubmitUpdateProperties(a0, a1);
}

bool SteamAPI_ISteamInventory_InspectItem(ISteamInventory* self, SteamInventoryResult_t* a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamInventory_InspectItem", true);
    if (!self)
        return {};
    return self->InspectItem(a0, a1);
}

HTTPRequestHandle SteamAPI_ISteamHTTP_CreateHTTPRequest(ISteamHTTP* self, int a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamHTTP_CreateHTTPRequest", true);
    if (!self)
        return {};
    return self->CreateHTTPRequest(a0, a1);
}

bool SteamAPI_ISteamHTTP_SetHTTPRequestContextValue(ISteamHTTP* self, HTTPRequestHandle a0, uint64_t a1) {
    SF_FLAT("SteamAPI_ISteamHTTP_SetHTTPRequestContextValue", true);
    if (!self)
        return {};
    return self->SetHTTPRequestContextValue(a0, a1);
}

bool SteamAPI_ISteamHTTP_SetHTTPRequestNetworkActivityTimeout(ISteamHTTP* self, HTTPRequestHandle a0, uint32_t a1) {
    SF_FLAT("SteamAPI_ISteamHTTP_SetHTTPRequestNetworkActivityTimeout", true);
    if (!self)
        return {};
    return self->SetHTTPRequestNetworkActivityTimeout(a0, a1);
}

bool SteamAPI_ISteamHTTP_SetHTTPRequestHeaderValue(ISteamHTTP* self, HTTPRequestHandle a0, const char* a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamHTTP_SetHTTPRequestHeaderValue", true);
    if (!self)
        return {};
    return self->SetHTTPRequestHeaderValue(a0, a1, a2);
}

bool SteamAPI_ISteamHTTP_SetHTTPRequestGetOrPostParameter(ISteamHTTP* self, HTTPRequestHandle a0, const char* a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamHTTP_SetHTTPRequestGetOrPostParameter", true);
    if (!self)
        return {};
    return self->SetHTTPRequestGetOrPostParameter(a0, a1, a2);
}

bool SteamAPI_ISteamHTTP_SendHTTPRequest(ISteamHTTP* self, HTTPRequestHandle a0, SteamAPICall_t* a1) {
    SF_FLAT("SteamAPI_ISteamHTTP_SendHTTPRequest", true);
    if (!self)
        return {};
    return self->SendHTTPRequest(a0, a1);
}

bool SteamAPI_ISteamHTTP_SendHTTPRequestAndStreamResponse(ISteamHTTP* self, HTTPRequestHandle a0, SteamAPICall_t* a1) {
    SF_FLAT("SteamAPI_ISteamHTTP_SendHTTPRequestAndStreamResponse", true);
    if (!self)
        return {};
    return self->SendHTTPRequestAndStreamResponse(a0, a1);
}

bool SteamAPI_ISteamHTTP_DeferHTTPRequest(ISteamHTTP* self, HTTPRequestHandle a0) {
    SF_FLAT("SteamAPI_ISteamHTTP_DeferHTTPRequest", true);
    if (!self)
        return {};
    return self->DeferHTTPRequest(a0);
}

bool SteamAPI_ISteamHTTP_PrioritizeHTTPRequest(ISteamHTTP* self, HTTPRequestHandle a0) {
    SF_FLAT("SteamAPI_ISteamHTTP_PrioritizeHTTPRequest", true);
    if (!self)
        return {};
    return self->PrioritizeHTTPRequest(a0);
}

bool SteamAPI_ISteamHTTP_GetHTTPResponseHeaderSize(ISteamHTTP* self, HTTPRequestHandle a0, const char* a1, uint32_t* a2) {
    SF_FLAT("SteamAPI_ISteamHTTP_GetHTTPResponseHeaderSize", true);
    if (!self)
        return {};
    return self->GetHTTPResponseHeaderSize(a0, a1, a2);
}

bool SteamAPI_ISteamHTTP_GetHTTPResponseHeaderValue(ISteamHTTP* self, HTTPRequestHandle a0, const char* a1, uint8_t* a2, uint32_t a3) {
    SF_FLAT("SteamAPI_ISteamHTTP_GetHTTPResponseHeaderValue", true);
    if (!self)
        return {};
    return self->GetHTTPResponseHeaderValue(a0, a1, a2, a3);
}

bool SteamAPI_ISteamHTTP_GetHTTPResponseBodySize(ISteamHTTP* self, HTTPRequestHandle a0, uint32_t* a1) {
    SF_FLAT("SteamAPI_ISteamHTTP_GetHTTPResponseBodySize", true);
    if (!self)
        return {};
    return self->GetHTTPResponseBodySize(a0, a1);
}

bool SteamAPI_ISteamHTTP_GetHTTPResponseBodyData(ISteamHTTP* self, HTTPRequestHandle a0, uint8_t* a1, uint32_t a2) {
    SF_FLAT("SteamAPI_ISteamHTTP_GetHTTPResponseBodyData", true);
    if (!self)
        return {};
    return self->GetHTTPResponseBodyData(a0, a1, a2);
}

bool SteamAPI_ISteamHTTP_GetHTTPStreamingResponseBodyData(ISteamHTTP* self, HTTPRequestHandle a0, uint32_t a1, uint8_t* a2, uint32_t a3) {
    SF_FLAT("SteamAPI_ISteamHTTP_GetHTTPStreamingResponseBodyData", true);
    if (!self)
        return {};
    return self->GetHTTPStreamingResponseBodyData(a0, a1, a2, a3);
}

bool SteamAPI_ISteamHTTP_ReleaseHTTPRequest(ISteamHTTP* self, HTTPRequestHandle a0) {
    SF_FLAT("SteamAPI_ISteamHTTP_ReleaseHTTPRequest", true);
    if (!self)
        return {};
    return self->ReleaseHTTPRequest(a0);
}

bool SteamAPI_ISteamHTTP_GetHTTPDownloadProgressPct(ISteamHTTP* self, HTTPRequestHandle a0, float* a1) {
    SF_FLAT("SteamAPI_ISteamHTTP_GetHTTPDownloadProgressPct", true);
    if (!self)
        return {};
    return self->GetHTTPDownloadProgressPct(a0, a1);
}

bool SteamAPI_ISteamHTTP_SetHTTPRequestRawPostBody(ISteamHTTP* self, HTTPRequestHandle a0, const char* a1, uint8_t* a2, uint32_t a3) {
    SF_FLAT("SteamAPI_ISteamHTTP_SetHTTPRequestRawPostBody", true);
    if (!self)
        return {};
    return self->SetHTTPRequestRawPostBody(a0, a1, a2, a3);
}

HTTPCookieContainerHandle SteamAPI_ISteamHTTP_CreateCookieContainer(ISteamHTTP* self, bool a0) {
    SF_FLAT("SteamAPI_ISteamHTTP_CreateCookieContainer", true);
    if (!self)
        return {};
    return self->CreateCookieContainer(a0);
}

bool SteamAPI_ISteamHTTP_ReleaseCookieContainer(ISteamHTTP* self, HTTPCookieContainerHandle a0) {
    SF_FLAT("SteamAPI_ISteamHTTP_ReleaseCookieContainer", true);
    if (!self)
        return {};
    return self->ReleaseCookieContainer(a0);
}

bool SteamAPI_ISteamHTTP_SetCookie(ISteamHTTP* self, HTTPCookieContainerHandle a0, const char* a1, const char* a2, const char* a3) {
    SF_FLAT("SteamAPI_ISteamHTTP_SetCookie", true);
    if (!self)
        return {};
    return self->SetCookie(a0, a1, a2, a3);
}

bool SteamAPI_ISteamHTTP_SetHTTPRequestCookieContainer(ISteamHTTP* self, HTTPRequestHandle a0, HTTPCookieContainerHandle a1) {
    SF_FLAT("SteamAPI_ISteamHTTP_SetHTTPRequestCookieContainer", true);
    if (!self)
        return {};
    return self->SetHTTPRequestCookieContainer(a0, a1);
}

bool SteamAPI_ISteamHTTP_SetHTTPRequestUserAgentInfo(ISteamHTTP* self, HTTPRequestHandle a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamHTTP_SetHTTPRequestUserAgentInfo", true);
    if (!self)
        return {};
    return self->SetHTTPRequestUserAgentInfo(a0, a1);
}

bool SteamAPI_ISteamHTTP_SetHTTPRequestRequiresVerifiedCertificate(ISteamHTTP* self, HTTPRequestHandle a0, bool a1) {
    SF_FLAT("SteamAPI_ISteamHTTP_SetHTTPRequestRequiresVerifiedCertificate", true);
    if (!self)
        return {};
    return self->SetHTTPRequestRequiresVerifiedCertificate(a0, a1);
}

bool SteamAPI_ISteamHTTP_SetHTTPRequestAbsoluteTimeoutMS(ISteamHTTP* self, HTTPRequestHandle a0, uint32_t a1) {
    SF_FLAT("SteamAPI_ISteamHTTP_SetHTTPRequestAbsoluteTimeoutMS", true);
    if (!self)
        return {};
    return self->SetHTTPRequestAbsoluteTimeoutMS(a0, a1);
}

bool SteamAPI_ISteamHTTP_GetHTTPRequestWasTimedOut(ISteamHTTP* self, HTTPRequestHandle a0, bool* a1) {
    SF_FLAT("SteamAPI_ISteamHTTP_GetHTTPRequestWasTimedOut", true);
    if (!self)
        return {};
    return self->GetHTTPRequestWasTimedOut(a0, a1);
}

ScreenshotHandle SteamAPI_ISteamScreenshots_WriteScreenshot(ISteamScreenshots* self, void* a0, uint32_t a1, int a2, int a3) {
    SF_FLAT("SteamAPI_ISteamScreenshots_WriteScreenshot", true);
    if (!self)
        return {};
    return self->WriteScreenshot(a0, a1, a2, a3);
}

ScreenshotHandle SteamAPI_ISteamScreenshots_AddScreenshotToLibrary(ISteamScreenshots* self, const char* a0, const char* a1, int a2, int a3) {
    SF_FLAT("SteamAPI_ISteamScreenshots_AddScreenshotToLibrary", true);
    if (!self)
        return {};
    return self->AddScreenshotToLibrary(a0, a1, a2, a3);
}

void SteamAPI_ISteamScreenshots_TriggerScreenshot(ISteamScreenshots* self) {
    SF_FLAT("SteamAPI_ISteamScreenshots_TriggerScreenshot", true);
    if (self)
        self->TriggerScreenshot();
}

void SteamAPI_ISteamScreenshots_HookScreenshots(ISteamScreenshots* self, bool a0) {
    SF_FLAT("SteamAPI_ISteamScreenshots_HookScreenshots", true);
    if (self)
        self->HookScreenshots(a0);
}

bool SteamAPI_ISteamScreenshots_SetLocation(ISteamScreenshots* self, ScreenshotHandle a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamScreenshots_SetLocation", true);
    if (!self)
        return {};
    return self->SetLocation(a0, a1);
}

bool SteamAPI_ISteamScreenshots_TagUser(ISteamScreenshots* self, ScreenshotHandle a0, CSteamID a1) {
    SF_FLAT("SteamAPI_ISteamScreenshots_TagUser", true);
    if (!self)
        return {};
    return self->TagUser(a0, a1);
}

bool SteamAPI_ISteamScreenshots_TagPublishedFile(ISteamScreenshots* self, ScreenshotHandle a0, PublishedFileId_t a1) {
    SF_FLAT("SteamAPI_ISteamScreenshots_TagPublishedFile", true);
    if (!self)
        return {};
    return self->TagPublishedFile(a0, a1);
}

bool SteamAPI_ISteamScreenshots_IsScreenshotsHooked(ISteamScreenshots* self) {
    SF_FLAT("SteamAPI_ISteamScreenshots_IsScreenshotsHooked", true);
    if (!self)
        return {};
    return self->IsScreenshotsHooked();
}

ScreenshotHandle SteamAPI_ISteamScreenshots_AddVRScreenshotToLibrary(ISteamScreenshots* self, int a0, const char* a1, const char* a2) {
    SF_FLAT("SteamAPI_ISteamScreenshots_AddVRScreenshotToLibrary", true);
    if (!self)
        return {};
    return self->AddVRScreenshotToLibrary(a0, a1, a2);
}

bool SteamAPI_ISteamController_Init(ISteamController* self) {
    SF_FLAT("SteamAPI_ISteamController_Init", true);
    if (!self)
        return {};
    return self->Init();
}

bool SteamAPI_ISteamController_Shutdown(ISteamController* self) {
    SF_FLAT("SteamAPI_ISteamController_Shutdown", true);
    if (!self)
        return {};
    return self->Shutdown();
}

void SteamAPI_ISteamController_RunFrame(ISteamController* self) {
    SF_FLAT("SteamAPI_ISteamController_RunFrame", true);
    if (self)
        self->RunFrame();
}

int SteamAPI_ISteamController_GetConnectedControllers(ISteamController* self, ControllerHandle_t* a0) {
    SF_FLAT("SteamAPI_ISteamController_GetConnectedControllers", true);
    if (!self)
        return {};
    return self->GetConnectedControllers(a0);
}

ControllerActionSetHandle_t SteamAPI_ISteamController_GetActionSetHandle(ISteamController* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamController_GetActionSetHandle", true);
    if (!self)
        return {};
    return self->GetActionSetHandle(a0);
}

void SteamAPI_ISteamController_ActivateActionSet(ISteamController* self, ControllerHandle_t a0, ControllerActionSetHandle_t a1) {
    SF_FLAT("SteamAPI_ISteamController_ActivateActionSet", true);
    if (self)
        self->ActivateActionSet(a0, a1);
}

ControllerActionSetHandle_t SteamAPI_ISteamController_GetCurrentActionSet(ISteamController* self, ControllerHandle_t a0) {
    SF_FLAT("SteamAPI_ISteamController_GetCurrentActionSet", true);
    if (!self)
        return {};
    return self->GetCurrentActionSet(a0);
}

void SteamAPI_ISteamController_ActivateActionSetLayer(ISteamController* self, ControllerHandle_t a0, ControllerActionSetHandle_t a1) {
    SF_FLAT("SteamAPI_ISteamController_ActivateActionSetLayer", true);
    if (self)
        self->ActivateActionSetLayer(a0, a1);
}

void SteamAPI_ISteamController_DeactivateActionSetLayer(ISteamController* self, ControllerHandle_t a0, ControllerActionSetHandle_t a1) {
    SF_FLAT("SteamAPI_ISteamController_DeactivateActionSetLayer", true);
    if (self)
        self->DeactivateActionSetLayer(a0, a1);
}

void SteamAPI_ISteamController_DeactivateAllActionSetLayers(ISteamController* self, ControllerHandle_t a0) {
    SF_FLAT("SteamAPI_ISteamController_DeactivateAllActionSetLayers", true);
    if (self)
        self->DeactivateAllActionSetLayers(a0);
}

int SteamAPI_ISteamController_GetActiveActionSetLayers(ISteamController* self, ControllerHandle_t a0, ControllerActionSetHandle_t* a1) {
    SF_FLAT("SteamAPI_ISteamController_GetActiveActionSetLayers", true);
    if (!self)
        return {};
    return self->GetActiveActionSetLayers(a0, a1);
}

ControllerDigitalActionHandle_t SteamAPI_ISteamController_GetDigitalActionHandle(ISteamController* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamController_GetDigitalActionHandle", true);
    if (!self)
        return {};
    return self->GetDigitalActionHandle(a0);
}

InputDigitalActionData_t SteamAPI_ISteamController_GetDigitalActionData(ISteamController* self, ControllerHandle_t a0, ControllerDigitalActionHandle_t a1) {
    SF_FLAT("SteamAPI_ISteamController_GetDigitalActionData", true);
    if (!self)
        return {};
    return self->GetDigitalActionData(a0, a1);
}

int SteamAPI_ISteamController_GetDigitalActionOrigins(ISteamController* self, ControllerHandle_t a0, ControllerActionSetHandle_t a1, ControllerDigitalActionHandle_t a2, int* a3) {
    SF_FLAT("SteamAPI_ISteamController_GetDigitalActionOrigins", true);
    if (!self)
        return {};
    return self->GetDigitalActionOrigins(a0, a1, a2, a3);
}

ControllerAnalogActionHandle_t SteamAPI_ISteamController_GetAnalogActionHandle(ISteamController* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamController_GetAnalogActionHandle", true);
    if (!self)
        return {};
    return self->GetAnalogActionHandle(a0);
}

InputAnalogActionData_t SteamAPI_ISteamController_GetAnalogActionData(ISteamController* self, ControllerHandle_t a0, ControllerAnalogActionHandle_t a1) {
    SF_FLAT("SteamAPI_ISteamController_GetAnalogActionData", true);
    if (!self)
        return {};
    return self->GetAnalogActionData(a0, a1);
}

int SteamAPI_ISteamController_GetAnalogActionOrigins(ISteamController* self, ControllerHandle_t a0, ControllerActionSetHandle_t a1, ControllerAnalogActionHandle_t a2, int* a3) {
    SF_FLAT("SteamAPI_ISteamController_GetAnalogActionOrigins", true);
    if (!self)
        return {};
    return self->GetAnalogActionOrigins(a0, a1, a2, a3);
}

const char* SteamAPI_ISteamController_GetGlyphForActionOrigin(ISteamController* self, int a0) {
    SF_FLAT("SteamAPI_ISteamController_GetGlyphForActionOrigin", true);
    if (!self)
        return {};
    return self->GetGlyphForActionOrigin(a0);
}

const char* SteamAPI_ISteamController_GetStringForActionOrigin(ISteamController* self, int a0) {
    SF_FLAT("SteamAPI_ISteamController_GetStringForActionOrigin", true);
    if (!self)
        return {};
    return self->GetStringForActionOrigin(a0);
}

void SteamAPI_ISteamController_StopAnalogActionMomentum(ISteamController* self, ControllerHandle_t a0, ControllerAnalogActionHandle_t a1) {
    SF_FLAT("SteamAPI_ISteamController_StopAnalogActionMomentum", true);
    if (self)
        self->StopAnalogActionMomentum(a0, a1);
}

InputMotionData_t SteamAPI_ISteamController_GetMotionData(ISteamController* self, ControllerHandle_t a0) {
    SF_FLAT("SteamAPI_ISteamController_GetMotionData", true);
    if (!self)
        return {};
    return self->GetMotionData(a0);
}

void SteamAPI_ISteamController_TriggerHapticPulse(ISteamController* self, ControllerHandle_t a0, int a1, unsigned short a2) {
    SF_FLAT("SteamAPI_ISteamController_TriggerHapticPulse", true);
    if (self)
        self->TriggerHapticPulse(a0, a1, a2);
}

void SteamAPI_ISteamController_TriggerRepeatedHapticPulse(ISteamController* self, ControllerHandle_t a0, int a1, unsigned short a2, unsigned short a3, unsigned short a4, unsigned int a5) {
    SF_FLAT("SteamAPI_ISteamController_TriggerRepeatedHapticPulse", true);
    if (self)
        self->TriggerRepeatedHapticPulse(a0, a1, a2, a3, a4, a5);
}

void SteamAPI_ISteamController_TriggerVibration(ISteamController* self, ControllerHandle_t a0, unsigned short a1, unsigned short a2) {
    SF_FLAT("SteamAPI_ISteamController_TriggerVibration", true);
    if (self)
        self->TriggerVibration(a0, a1, a2);
}

void SteamAPI_ISteamController_SetLEDColor(ISteamController* self, ControllerHandle_t a0, uint8_t a1, uint8_t a2, uint8_t a3, unsigned int a4) {
    SF_FLAT("SteamAPI_ISteamController_SetLEDColor", true);
    if (self)
        self->SetLEDColor(a0, a1, a2, a3, a4);
}

bool SteamAPI_ISteamController_ShowBindingPanel(ISteamController* self, ControllerHandle_t a0) {
    SF_FLAT("SteamAPI_ISteamController_ShowBindingPanel", true);
    if (!self)
        return {};
    return self->ShowBindingPanel(a0);
}

int SteamAPI_ISteamController_GetInputTypeForHandle(ISteamController* self, ControllerHandle_t a0) {
    SF_FLAT("SteamAPI_ISteamController_GetInputTypeForHandle", true);
    if (!self)
        return {};
    return self->GetInputTypeForHandle(a0);
}

ControllerHandle_t SteamAPI_ISteamController_GetControllerForGamepadIndex(ISteamController* self, int a0) {
    SF_FLAT("SteamAPI_ISteamController_GetControllerForGamepadIndex", true);
    if (!self)
        return {};
    return self->GetControllerForGamepadIndex(a0);
}

int SteamAPI_ISteamController_GetGamepadIndexForController(ISteamController* self, ControllerHandle_t a0) {
    SF_FLAT("SteamAPI_ISteamController_GetGamepadIndexForController", true);
    if (!self)
        return {};
    return self->GetGamepadIndexForController(a0);
}

const char* SteamAPI_ISteamController_GetStringForXboxOrigin(ISteamController* self, int a0) {
    SF_FLAT("SteamAPI_ISteamController_GetStringForXboxOrigin", true);
    if (!self)
        return {};
    return self->GetStringForXboxOrigin(a0);
}

const char* SteamAPI_ISteamController_GetGlyphForXboxOrigin(ISteamController* self, int a0) {
    SF_FLAT("SteamAPI_ISteamController_GetGlyphForXboxOrigin", true);
    if (!self)
        return {};
    return self->GetGlyphForXboxOrigin(a0);
}

int SteamAPI_ISteamController_GetActionOriginFromXboxOrigin(ISteamController* self, ControllerHandle_t a0, int a1) {
    SF_FLAT("SteamAPI_ISteamController_GetActionOriginFromXboxOrigin", true);
    if (!self)
        return {};
    return self->GetActionOriginFromXboxOrigin(a0, a1);
}

int SteamAPI_ISteamController_TranslateActionOrigin(ISteamController* self, int a0, int a1) {
    SF_FLAT("SteamAPI_ISteamController_TranslateActionOrigin", true);
    if (!self)
        return {};
    return self->TranslateActionOrigin(a0, a1);
}

bool SteamAPI_ISteamController_GetControllerBindingRevision(ISteamController* self, ControllerHandle_t a0, int* a1) {
    SF_FLAT("SteamAPI_ISteamController_GetControllerBindingRevision", true);
    if (!self)
        return {};
    return self->GetControllerBindingRevision(a0, a1);
}

bool SteamAPI_ISteamInput_Init(ISteamInput* self, bool a0) {
    SF_FLAT("SteamAPI_ISteamInput_Init", true);
    if (!self)
        return {};
    return self->Init(a0);
}

bool SteamAPI_ISteamInput_Shutdown(ISteamInput* self) {
    SF_FLAT("SteamAPI_ISteamInput_Shutdown", true);
    if (!self)
        return {};
    return self->Shutdown();
}

void SteamAPI_ISteamInput_RunFrame(ISteamInput* self, bool a0) {
    SF_FLAT("SteamAPI_ISteamInput_RunFrame", true);
    if (self)
        self->RunFrame(a0);
}

int SteamAPI_ISteamInput_GetConnectedControllers(ISteamInput* self, ControllerHandle_t* a0) {
    SF_FLAT("SteamAPI_ISteamInput_GetConnectedControllers", true);
    if (!self)
        return {};
    return self->GetConnectedControllers(a0);
}

ControllerActionSetHandle_t SteamAPI_ISteamInput_GetActionSetHandle(ISteamInput* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamInput_GetActionSetHandle", true);
    if (!self)
        return {};
    return self->GetActionSetHandle(a0);
}

void SteamAPI_ISteamInput_ActivateActionSet(ISteamInput* self, ControllerHandle_t a0, ControllerActionSetHandle_t a1) {
    SF_FLAT("SteamAPI_ISteamInput_ActivateActionSet", true);
    if (self)
        self->ActivateActionSet(a0, a1);
}

ControllerActionSetHandle_t SteamAPI_ISteamInput_GetCurrentActionSet(ISteamInput* self, ControllerHandle_t a0) {
    SF_FLAT("SteamAPI_ISteamInput_GetCurrentActionSet", true);
    if (!self)
        return {};
    return self->GetCurrentActionSet(a0);
}

void SteamAPI_ISteamInput_ActivateActionSetLayer(ISteamInput* self, ControllerHandle_t a0, ControllerActionSetHandle_t a1) {
    SF_FLAT("SteamAPI_ISteamInput_ActivateActionSetLayer", true);
    if (self)
        self->ActivateActionSetLayer(a0, a1);
}

void SteamAPI_ISteamInput_DeactivateActionSetLayer(ISteamInput* self, ControllerHandle_t a0, ControllerActionSetHandle_t a1) {
    SF_FLAT("SteamAPI_ISteamInput_DeactivateActionSetLayer", true);
    if (self)
        self->DeactivateActionSetLayer(a0, a1);
}

void SteamAPI_ISteamInput_DeactivateAllActionSetLayers(ISteamInput* self, ControllerHandle_t a0) {
    SF_FLAT("SteamAPI_ISteamInput_DeactivateAllActionSetLayers", true);
    if (self)
        self->DeactivateAllActionSetLayers(a0);
}

int SteamAPI_ISteamInput_GetActiveActionSetLayers(ISteamInput* self, ControllerHandle_t a0, ControllerActionSetHandle_t* a1) {
    SF_FLAT("SteamAPI_ISteamInput_GetActiveActionSetLayers", true);
    if (!self)
        return {};
    return self->GetActiveActionSetLayers(a0, a1);
}

ControllerDigitalActionHandle_t SteamAPI_ISteamInput_GetDigitalActionHandle(ISteamInput* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamInput_GetDigitalActionHandle", true);
    if (!self)
        return {};
    return self->GetDigitalActionHandle(a0);
}

InputDigitalActionData_t SteamAPI_ISteamInput_GetDigitalActionData(ISteamInput* self, ControllerHandle_t a0, ControllerDigitalActionHandle_t a1) {
    SF_FLAT("SteamAPI_ISteamInput_GetDigitalActionData", true);
    if (!self)
        return {};
    return self->GetDigitalActionData(a0, a1);
}

int SteamAPI_ISteamInput_GetDigitalActionOrigins(ISteamInput* self, ControllerHandle_t a0, ControllerActionSetHandle_t a1, ControllerDigitalActionHandle_t a2, int* a3) {
    SF_FLAT("SteamAPI_ISteamInput_GetDigitalActionOrigins", true);
    if (!self)
        return {};
    return self->GetDigitalActionOrigins(a0, a1, a2, a3);
}

const char* SteamAPI_ISteamInput_GetStringForDigitalActionName(ISteamInput* self, ControllerDigitalActionHandle_t a0) {
    SF_FLAT("SteamAPI_ISteamInput_GetStringForDigitalActionName", true);
    if (!self)
        return {};
    return self->GetStringForDigitalActionName(a0);
}

ControllerAnalogActionHandle_t SteamAPI_ISteamInput_GetAnalogActionHandle(ISteamInput* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamInput_GetAnalogActionHandle", true);
    if (!self)
        return {};
    return self->GetAnalogActionHandle(a0);
}

InputAnalogActionData_t SteamAPI_ISteamInput_GetAnalogActionData(ISteamInput* self, ControllerHandle_t a0, ControllerAnalogActionHandle_t a1) {
    SF_FLAT("SteamAPI_ISteamInput_GetAnalogActionData", true);
    if (!self)
        return {};
    return self->GetAnalogActionData(a0, a1);
}

int SteamAPI_ISteamInput_GetAnalogActionOrigins(ISteamInput* self, ControllerHandle_t a0, ControllerActionSetHandle_t a1, ControllerAnalogActionHandle_t a2, int* a3) {
    SF_FLAT("SteamAPI_ISteamInput_GetAnalogActionOrigins", true);
    if (!self)
        return {};
    return self->GetAnalogActionOrigins(a0, a1, a2, a3);
}

const char* SteamAPI_ISteamInput_GetGlyphForActionOrigin_Legacy(ISteamInput* self, int a0) {
    SF_FLAT("SteamAPI_ISteamInput_GetGlyphForActionOrigin_Legacy", true);
    if (!self)
        return {};
    return self->GetGlyphForActionOrigin_Legacy(a0);
}

const char* SteamAPI_ISteamInput_GetStringForActionOrigin_Legacy(ISteamInput* self, int a0) {
    SF_FLAT("SteamAPI_ISteamInput_GetStringForActionOrigin_Legacy", true);
    if (!self)
        return {};
    return self->GetStringForActionOrigin_Legacy(a0);
}

const char* SteamAPI_ISteamInput_GetGlyphForXboxOrigin_Legacy(ISteamInput* self, int a0) {
    SF_FLAT("SteamAPI_ISteamInput_GetGlyphForXboxOrigin_Legacy", true);
    if (!self)
        return {};
    return self->GetGlyphForXboxOrigin_Legacy(a0);
}

const char* SteamAPI_ISteamInput_GetStringForXboxOrigin_Legacy(ISteamInput* self, int a0) {
    SF_FLAT("SteamAPI_ISteamInput_GetStringForXboxOrigin_Legacy", true);
    if (!self)
        return {};
    return self->GetStringForXboxOrigin_Legacy(a0);
}

int SteamAPI_ISteamInput_GetActionOriginFromXboxOrigin(ISteamInput* self, ControllerHandle_t a0, int a1) {
    SF_FLAT("SteamAPI_ISteamInput_GetActionOriginFromXboxOrigin", true);
    if (!self)
        return {};
    return self->GetActionOriginFromXboxOrigin(a0, a1);
}

int SteamAPI_ISteamInput_TranslateActionOrigin(ISteamInput* self, int a0, int a1) {
    SF_FLAT("SteamAPI_ISteamInput_TranslateActionOrigin", true);
    if (!self)
        return {};
    return self->TranslateActionOrigin(a0, a1);
}

bool SteamAPI_ISteamInput_GetDeviceBindingRevision(ISteamInput* self, ControllerHandle_t a0, int* a1, int* a2) {
    SF_FLAT("SteamAPI_ISteamInput_GetDeviceBindingRevision", true);
    if (!self)
        return {};
    return self->GetDeviceBindingRevision(a0, a1, a2);
}

uint32_t SteamAPI_ISteamInput_GetRemotePlaySessionID(ISteamInput* self, ControllerHandle_t a0) {
    SF_FLAT("SteamAPI_ISteamInput_GetRemotePlaySessionID", true);
    if (!self)
        return {};
    return self->GetRemotePlaySessionID(a0);
}

uint16_t SteamAPI_ISteamInput_GetSessionInputConfigurationSettings(ISteamInput* self) {
    SF_FLAT("SteamAPI_ISteamInput_GetSessionInputConfigurationSettings", true);
    if (!self)
        return {};
    return self->GetSessionInputConfigurationSettings();
}

void SteamAPI_ISteamInput_SetDualSenseTriggerEffect(ISteamInput* self, ControllerHandle_t a0, const void* a1) {
    SF_FLAT("SteamAPI_ISteamInput_SetDualSenseTriggerEffect", true);
    if (self)
        self->SetDualSenseTriggerEffect(a0, a1);
}

void SteamAPI_ISteamInput_StopAnalogActionMomentum(ISteamInput* self, ControllerHandle_t a0, ControllerAnalogActionHandle_t a1) {
    SF_FLAT("SteamAPI_ISteamInput_StopAnalogActionMomentum", true);
    if (self)
        self->StopAnalogActionMomentum(a0, a1);
}

InputMotionData_t SteamAPI_ISteamInput_GetMotionData(ISteamInput* self, ControllerHandle_t a0) {
    SF_FLAT("SteamAPI_ISteamInput_GetMotionData", true);
    if (!self)
        return {};
    return self->GetMotionData(a0);
}

void SteamAPI_ISteamInput_TriggerVibration(ISteamInput* self, ControllerHandle_t a0, unsigned short a1, unsigned short a2) {
    SF_FLAT("SteamAPI_ISteamInput_TriggerVibration", true);
    if (self)
        self->TriggerVibration(a0, a1, a2);
}

void SteamAPI_ISteamInput_SetLEDColor(ISteamInput* self, ControllerHandle_t a0, uint8_t a1, uint8_t a2, uint8_t a3, unsigned int a4) {
    SF_FLAT("SteamAPI_ISteamInput_SetLEDColor", true);
    if (self)
        self->SetLEDColor(a0, a1, a2, a3, a4);
}

void SteamAPI_ISteamInput_TriggerVibrationExtended(ISteamInput* self, ControllerHandle_t a0, unsigned short a1, unsigned short a2, unsigned short a3, unsigned short a4) {
    SF_FLAT("SteamAPI_ISteamInput_TriggerVibrationExtended", true);
    if (self)
        self->TriggerVibrationExtended(a0, a1, a2, a3, a4);
}

void SteamAPI_ISteamInput_TriggerSimpleHapticEvent(ISteamInput* self, ControllerHandle_t a0, int a1, uint8_t a2, uint8_t a3, uint8_t a4) {
    SF_FLAT("SteamAPI_ISteamInput_TriggerSimpleHapticEvent", true);
    if (self)
        self->TriggerSimpleHapticEvent(a0, a1, a2, a3, a4);
}

int SteamAPI_ISteamInput_GetActionOriginFromXboxOrigin_Legacy(ISteamInput* self, int a0) {
    SF_FLAT("SteamAPI_ISteamInput_GetActionOriginFromXboxOrigin_Legacy", true);
    if (!self)
        return {};
    return self->GetActionOriginFromXboxOrigin_Legacy(a0);
}

bool SteamAPI_ISteamMusic_BIsEnabled(ISteamMusic* self) {
    SF_FLAT("SteamAPI_ISteamMusic_BIsEnabled", true);
    if (!self)
        return {};
    return self->BIsEnabled();
}

bool SteamAPI_ISteamMusic_BIsPlaying(ISteamMusic* self) {
    SF_FLAT("SteamAPI_ISteamMusic_BIsPlaying", true);
    if (!self)
        return {};
    return self->BIsPlaying();
}

int SteamAPI_ISteamMusic_GetPlaybackStatus(ISteamMusic* self) {
    SF_FLAT("SteamAPI_ISteamMusic_GetPlaybackStatus", true);
    if (!self)
        return {};
    return self->GetPlaybackStatus();
}

void SteamAPI_ISteamMusic_Play(ISteamMusic* self) {
    SF_FLAT("SteamAPI_ISteamMusic_Play", true);
    if (self)
        self->Play();
}

void SteamAPI_ISteamMusic_Pause(ISteamMusic* self) {
    SF_FLAT("SteamAPI_ISteamMusic_Pause", true);
    if (self)
        self->Pause();
}

void SteamAPI_ISteamMusic_PlayPrevious(ISteamMusic* self) {
    SF_FLAT("SteamAPI_ISteamMusic_PlayPrevious", true);
    if (self)
        self->PlayPrevious();
}

void SteamAPI_ISteamMusic_PlayNext(ISteamMusic* self) {
    SF_FLAT("SteamAPI_ISteamMusic_PlayNext", true);
    if (self)
        self->PlayNext();
}

void SteamAPI_ISteamMusic_SetVolume(ISteamMusic* self, float a0) {
    SF_FLAT("SteamAPI_ISteamMusic_SetVolume", true);
    if (self)
        self->SetVolume(a0);
}

float SteamAPI_ISteamMusic_GetVolume(ISteamMusic* self) {
    SF_FLAT("SteamAPI_ISteamMusic_GetVolume", true);
    if (!self)
        return {};
    return self->GetVolume();
}

void SteamAPI_ISteamVideo_GetVideoURL(ISteamVideo* self, AppId_t a0) {
    SF_FLAT("SteamAPI_ISteamVideo_GetVideoURL", true);
    if (self)
        self->GetVideoURL(a0);
}

bool SteamAPI_ISteamVideo_IsBroadcasting(ISteamVideo* self, int* a0) {
    SF_FLAT("SteamAPI_ISteamVideo_IsBroadcasting", true);
    if (!self)
        return {};
    return self->IsBroadcasting(a0);
}

void SteamAPI_ISteamVideo_GetOPFSettings(ISteamVideo* self, AppId_t a0) {
    SF_FLAT("SteamAPI_ISteamVideo_GetOPFSettings", true);
    if (self)
        self->GetOPFSettings(a0);
}

bool SteamAPI_ISteamVideo_GetOPFStringForApp(ISteamVideo* self, AppId_t a0, char* a1, int32_t* a2) {
    SF_FLAT("SteamAPI_ISteamVideo_GetOPFStringForApp", true);
    if (!self)
        return {};
    return self->GetOPFStringForApp(a0, a1, a2);
}

bool SteamAPI_ISteamVideo_GetOPFIntForApp(ISteamVideo* self, AppId_t a0, int32_t* a1) {
    SF_FLAT("SteamAPI_ISteamVideo_GetOPFIntForApp", true);
    if (!self)
        return {};
    return self->GetOPFIntForApp(a0, a1);
}

bool SteamAPI_ISteamParentalSettings_BIsParentalLockEnabled(ISteamParentalSettings* self) {
    SF_FLAT("SteamAPI_ISteamParentalSettings_BIsParentalLockEnabled", true);
    if (!self)
        return {};
    return self->BIsParentalLockEnabled();
}

bool SteamAPI_ISteamParentalSettings_BIsParentalLockLocked(ISteamParentalSettings* self) {
    SF_FLAT("SteamAPI_ISteamParentalSettings_BIsParentalLockLocked", true);
    if (!self)
        return {};
    return self->BIsParentalLockLocked();
}

bool SteamAPI_ISteamParentalSettings_BIsAppBlocked(ISteamParentalSettings* self, AppId_t a0) {
    SF_FLAT("SteamAPI_ISteamParentalSettings_BIsAppBlocked", true);
    if (!self)
        return {};
    return self->BIsAppBlocked(a0);
}

bool SteamAPI_ISteamParentalSettings_BIsAppInBlockList(ISteamParentalSettings* self, AppId_t a0) {
    SF_FLAT("SteamAPI_ISteamParentalSettings_BIsAppInBlockList", true);
    if (!self)
        return {};
    return self->BIsAppInBlockList(a0);
}

bool SteamAPI_ISteamParentalSettings_BIsFeatureBlocked(ISteamParentalSettings* self, int a0) {
    SF_FLAT("SteamAPI_ISteamParentalSettings_BIsFeatureBlocked", true);
    if (!self)
        return {};
    return self->BIsFeatureBlocked(a0);
}

bool SteamAPI_ISteamParentalSettings_BIsFeatureInBlockList(ISteamParentalSettings* self, int a0) {
    SF_FLAT("SteamAPI_ISteamParentalSettings_BIsFeatureInBlockList", true);
    if (!self)
        return {};
    return self->BIsFeatureInBlockList(a0);
}

uint32_t SteamAPI_ISteamAppList_GetNumInstalledApps(ISteamAppList* self) {
    SF_FLAT("SteamAPI_ISteamAppList_GetNumInstalledApps", true);
    if (!self)
        return {};
    return self->GetNumInstalledApps();
}

uint32_t SteamAPI_ISteamAppList_GetInstalledApps(ISteamAppList* self, AppId_t* a0, uint32_t a1) {
    SF_FLAT("SteamAPI_ISteamAppList_GetInstalledApps", true);
    if (!self)
        return {};
    return self->GetInstalledApps(a0, a1);
}

int SteamAPI_ISteamAppList_GetAppName(ISteamAppList* self, AppId_t a0, char* a1, int a2) {
    SF_FLAT("SteamAPI_ISteamAppList_GetAppName", true);
    if (!self)
        return {};
    return self->GetAppName(a0, a1, a2);
}

int SteamAPI_ISteamAppList_GetAppInstallDir(ISteamAppList* self, AppId_t a0, char* a1, int a2) {
    SF_FLAT("SteamAPI_ISteamAppList_GetAppInstallDir", true);
    if (!self)
        return {};
    return self->GetAppInstallDir(a0, a1, a2);
}

int SteamAPI_ISteamAppList_GetAppBuildId(ISteamAppList* self, AppId_t a0) {
    SF_FLAT("SteamAPI_ISteamAppList_GetAppBuildId", true);
    if (!self)
        return {};
    return self->GetAppBuildId(a0);
}

bool SteamAPI_ISteamGameServer_InitGameServer(ISteamGameServer* self, uint32_t a0, uint16_t a1, uint16_t a2, uint16_t a3, int a4, const char* a5) {
    SF_FLAT("SteamAPI_ISteamGameServer_InitGameServer", true);
    if (!self)
        return {};
    return self->InitGameServer(a0, a1, a2, a3, a4, a5);
}

void SteamAPI_ISteamGameServer_SetProduct(ISteamGameServer* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_SetProduct", true);
    if (self)
        self->SetProduct(a0);
}

void SteamAPI_ISteamGameServer_SetGameDescription(ISteamGameServer* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_SetGameDescription", true);
    if (self)
        self->SetGameDescription(a0);
}

void SteamAPI_ISteamGameServer_SetModDir(ISteamGameServer* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_SetModDir", true);
    if (self)
        self->SetModDir(a0);
}

void SteamAPI_ISteamGameServer_SetDedicatedServer(ISteamGameServer* self, bool a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_SetDedicatedServer", true);
    if (self)
        self->SetDedicatedServer(a0);
}

void SteamAPI_ISteamGameServer_LogOn(ISteamGameServer* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_LogOn", true);
    if (self)
        self->LogOn(a0);
}

void SteamAPI_ISteamGameServer_LogOnAnonymous(ISteamGameServer* self) {
    SF_FLAT("SteamAPI_ISteamGameServer_LogOnAnonymous", true);
    if (self)
        self->LogOnAnonymous();
}

void SteamAPI_ISteamGameServer_LogOff(ISteamGameServer* self) {
    SF_FLAT("SteamAPI_ISteamGameServer_LogOff", true);
    if (self)
        self->LogOff();
}

bool SteamAPI_ISteamGameServer_BLoggedOn(ISteamGameServer* self) {
    SF_FLAT("SteamAPI_ISteamGameServer_BLoggedOn", true);
    if (!self)
        return {};
    return self->BLoggedOn();
}

bool SteamAPI_ISteamGameServer_BSecure(ISteamGameServer* self) {
    SF_FLAT("SteamAPI_ISteamGameServer_BSecure", true);
    if (!self)
        return {};
    return self->BSecure();
}

CSteamID SteamAPI_ISteamGameServer_GetSteamID(ISteamGameServer* self) {
    SF_FLAT("SteamAPI_ISteamGameServer_GetSteamID", true);
    if (!self)
        return {};
    return self->GetSteamID();
}

bool SteamAPI_ISteamGameServer_WasRestartRequested(ISteamGameServer* self) {
    SF_FLAT("SteamAPI_ISteamGameServer_WasRestartRequested", true);
    if (!self)
        return {};
    return self->WasRestartRequested();
}

void SteamAPI_ISteamGameServer_SetMaxPlayerCount(ISteamGameServer* self, int a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_SetMaxPlayerCount", true);
    if (self)
        self->SetMaxPlayerCount(a0);
}

void SteamAPI_ISteamGameServer_SetBotPlayerCount(ISteamGameServer* self, int a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_SetBotPlayerCount", true);
    if (self)
        self->SetBotPlayerCount(a0);
}

void SteamAPI_ISteamGameServer_SetServerName(ISteamGameServer* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_SetServerName", true);
    if (self)
        self->SetServerName(a0);
}

void SteamAPI_ISteamGameServer_SetMapName(ISteamGameServer* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_SetMapName", true);
    if (self)
        self->SetMapName(a0);
}

void SteamAPI_ISteamGameServer_SetPasswordProtected(ISteamGameServer* self, bool a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_SetPasswordProtected", true);
    if (self)
        self->SetPasswordProtected(a0);
}

void SteamAPI_ISteamGameServer_SetSpectatorPort(ISteamGameServer* self, uint16_t a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_SetSpectatorPort", true);
    if (self)
        self->SetSpectatorPort(a0);
}

void SteamAPI_ISteamGameServer_SetSpectatorServerName(ISteamGameServer* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_SetSpectatorServerName", true);
    if (self)
        self->SetSpectatorServerName(a0);
}

void SteamAPI_ISteamGameServer_ClearAllKeyValues(ISteamGameServer* self) {
    SF_FLAT("SteamAPI_ISteamGameServer_ClearAllKeyValues", true);
    if (self)
        self->ClearAllKeyValues();
}

void SteamAPI_ISteamGameServer_SetKeyValue(ISteamGameServer* self, const char* a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamGameServer_SetKeyValue", true);
    if (self)
        self->SetKeyValue(a0, a1);
}

void SteamAPI_ISteamGameServer_SetGameTags(ISteamGameServer* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_SetGameTags", true);
    if (self)
        self->SetGameTags(a0);
}

void SteamAPI_ISteamGameServer_SetGameData(ISteamGameServer* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_SetGameData", true);
    if (self)
        self->SetGameData(a0);
}

void SteamAPI_ISteamGameServer_SetRegion(ISteamGameServer* self, const char* a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_SetRegion", true);
    if (self)
        self->SetRegion(a0);
}

void SteamAPI_ISteamGameServer_SetAdvertiseServerActive(ISteamGameServer* self, bool a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_SetAdvertiseServerActive", true);
    if (self)
        self->SetAdvertiseServerActive(a0);
}

HAuthTicket SteamAPI_ISteamGameServer_GetAuthSessionTicket(ISteamGameServer* self, void* a0, int a1, uint32_t* a2) {
    SF_FLAT("SteamAPI_ISteamGameServer_GetAuthSessionTicket", true);
    if (!self)
        return {};
    return self->GetAuthSessionTicket(a0, a1, a2);
}

EBeginAuthSessionResult SteamAPI_ISteamGameServer_BeginAuthSession(ISteamGameServer* self, const void* a0, int a1, CSteamID a2) {
    SF_FLAT("SteamAPI_ISteamGameServer_BeginAuthSession", true);
    if (!self)
        return {};
    return self->BeginAuthSession(a0, a1, a2);
}

void SteamAPI_ISteamGameServer_EndAuthSession(ISteamGameServer* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_EndAuthSession", true);
    if (self)
        self->EndAuthSession(a0);
}

void SteamAPI_ISteamGameServer_CancelAuthTicket(ISteamGameServer* self, HAuthTicket a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_CancelAuthTicket", true);
    if (self)
        self->CancelAuthTicket(a0);
}

EUserHasLicenseForAppResult SteamAPI_ISteamGameServer_UserHasLicenseForApp(ISteamGameServer* self, CSteamID a0, AppId_t a1) {
    SF_FLAT("SteamAPI_ISteamGameServer_UserHasLicenseForApp", true);
    if (!self)
        return {};
    return self->UserHasLicenseForApp(a0, a1);
}

bool SteamAPI_ISteamGameServer_RequestUserGroupStatus(ISteamGameServer* self, CSteamID a0, CSteamID a1) {
    SF_FLAT("SteamAPI_ISteamGameServer_RequestUserGroupStatus", true);
    if (!self)
        return {};
    return self->RequestUserGroupStatus(a0, a1);
}

void SteamAPI_ISteamGameServer_GetGameplayStats(ISteamGameServer* self) {
    SF_FLAT("SteamAPI_ISteamGameServer_GetGameplayStats", true);
    if (self)
        self->GetGameplayStats();
}

SteamAPICall_t SteamAPI_ISteamGameServer_GetServerReputation(ISteamGameServer* self) {
    SF_FLAT("SteamAPI_ISteamGameServer_GetServerReputation", true);
    if (!self)
        return {};
    return self->GetServerReputation();
}

SteamIPAddress_t SteamAPI_ISteamGameServer_GetPublicIP(ISteamGameServer* self) {
    SF_FLAT("SteamAPI_ISteamGameServer_GetPublicIP", true);
    if (!self)
        return {};
    return self->GetPublicIP();
}

bool SteamAPI_ISteamGameServer_HandleIncomingPacket(ISteamGameServer* self, const void* a0, int a1, uint32_t a2, uint16_t a3) {
    SF_FLAT("SteamAPI_ISteamGameServer_HandleIncomingPacket", true);
    if (!self)
        return {};
    return self->HandleIncomingPacket(a0, a1, a2, a3);
}

int SteamAPI_ISteamGameServer_GetNextOutgoingPacket(ISteamGameServer* self, void* a0, int a1, uint32_t* a2, uint16_t* a3) {
    SF_FLAT("SteamAPI_ISteamGameServer_GetNextOutgoingPacket", true);
    if (!self)
        return {};
    return self->GetNextOutgoingPacket(a0, a1, a2, a3);
}

SteamAPICall_t SteamAPI_ISteamGameServer_AssociateWithClan(ISteamGameServer* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_AssociateWithClan", true);
    if (!self)
        return {};
    return self->AssociateWithClan(a0);
}

SteamAPICall_t SteamAPI_ISteamGameServer_ComputeNewPlayerCompatibility(ISteamGameServer* self, SteamAPICall_t a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_ComputeNewPlayerCompatibility", true);
    if (!self)
        return {};
    return self->ComputeNewPlayerCompatibility(a0);
}

bool SteamAPI_ISteamGameServer_SendUserConnectAndAuthenticate_DEPRECATED(ISteamGameServer* self, uint32_t a0, const void* a1, uint32_t a2, CSteamID* a3) {
    SF_FLAT("SteamAPI_ISteamGameServer_SendUserConnectAndAuthenticate_DEPRECATED", true);
    if (!self)
        return {};
    return self->SendUserConnectAndAuthenticate_DEPRECATED(a0, a1, a2, a3);
}

CSteamID SteamAPI_ISteamGameServer_CreateUnauthenticatedUserConnection(ISteamGameServer* self) {
    SF_FLAT("SteamAPI_ISteamGameServer_CreateUnauthenticatedUserConnection", true);
    if (!self)
        return {};
    return self->CreateUnauthenticatedUserConnection();
}

void SteamAPI_ISteamGameServer_SendUserDisconnect_DEPRECATED(ISteamGameServer* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamGameServer_SendUserDisconnect_DEPRECATED", true);
    if (self)
        self->SendUserDisconnect_DEPRECATED(a0);
}

bool SteamAPI_ISteamGameServer_BUpdateUserData(ISteamGameServer* self, CSteamID a0, const char* a1, uint32_t a2) {
    SF_FLAT("SteamAPI_ISteamGameServer_BUpdateUserData", true);
    if (!self)
        return {};
    return self->BUpdateUserData(a0, a1, a2);
}

bool SteamAPI_ISteamGameServer_GetCertificateValidationBundle(ISteamGameServer* self, void* a0, int a1) {
    SF_FLAT("SteamAPI_ISteamGameServer_GetCertificateValidationBundle", true);
    if (!self)
        return {};
    return self->GetCertificateValidationBundle(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamGameServerStats_RequestUserStats(ISteamGameServerStats* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamGameServerStats_RequestUserStats", true);
    if (!self)
        return {};
    return self->RequestUserStats(a0);
}

bool SteamAPI_ISteamGameServerStats_GetUserStatInt32(ISteamGameServerStats* self, CSteamID a0, const char* a1, int32_t* a2) {
    SF_FLAT("SteamAPI_ISteamGameServerStats_GetUserStatInt32", true);
    if (!self)
        return {};
    return self->GetUserStat(a0, a1, a2);
}

bool SteamAPI_ISteamGameServerStats_GetUserStatFloat(ISteamGameServerStats* self, CSteamID a0, const char* a1, float* a2) {
    SF_FLAT("SteamAPI_ISteamGameServerStats_GetUserStatFloat", true);
    if (!self)
        return {};
    return self->GetUserStat(a0, a1, a2);
}

bool SteamAPI_ISteamGameServerStats_GetUserAchievement(ISteamGameServerStats* self, CSteamID a0, const char* a1, bool* a2) {
    SF_FLAT("SteamAPI_ISteamGameServerStats_GetUserAchievement", true);
    if (!self)
        return {};
    return self->GetUserAchievement(a0, a1, a2);
}

bool SteamAPI_ISteamGameServerStats_SetUserStatInt32(ISteamGameServerStats* self, CSteamID a0, const char* a1, int32_t a2) {
    SF_FLAT("SteamAPI_ISteamGameServerStats_SetUserStatInt32", true);
    if (!self)
        return {};
    return self->SetUserStat(a0, a1, a2);
}

bool SteamAPI_ISteamGameServerStats_SetUserStatFloat(ISteamGameServerStats* self, CSteamID a0, const char* a1, float a2) {
    SF_FLAT("SteamAPI_ISteamGameServerStats_SetUserStatFloat", true);
    if (!self)
        return {};
    return self->SetUserStat(a0, a1, a2);
}

bool SteamAPI_ISteamGameServerStats_UpdateUserAvgRateStat(ISteamGameServerStats* self, CSteamID a0, const char* a1, float a2, double a3) {
    SF_FLAT("SteamAPI_ISteamGameServerStats_UpdateUserAvgRateStat", true);
    if (!self)
        return {};
    return self->UpdateUserAvgRateStat(a0, a1, a2, a3);
}

bool SteamAPI_ISteamGameServerStats_SetUserAchievement(ISteamGameServerStats* self, CSteamID a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamGameServerStats_SetUserAchievement", true);
    if (!self)
        return {};
    return self->SetUserAchievement(a0, a1);
}

bool SteamAPI_ISteamGameServerStats_ClearUserAchievement(ISteamGameServerStats* self, CSteamID a0, const char* a1) {
    SF_FLAT("SteamAPI_ISteamGameServerStats_ClearUserAchievement", true);
    if (!self)
        return {};
    return self->ClearUserAchievement(a0, a1);
}

SteamAPICall_t SteamAPI_ISteamGameServerStats_StoreUserStats(ISteamGameServerStats* self, CSteamID a0) {
    SF_FLAT("SteamAPI_ISteamGameServerStats_StoreUserStats", true);
    if (!self)
        return {};
    return self->StoreUserStats(a0);
}

// 234 entradas que Steamworks.NET importa y el emulador no implementa: valor neutro.

void SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialogConnectString() {
    SF_FLAT("SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialogConnectString", false);
}

uint64_t SteamAPI_ISteamFriends_BHasEquippedProfileItem() {
    SF_FLAT("SteamAPI_ISteamFriends_BHasEquippedProfileItem", false);
    return 0;
}

const char* SteamAPI_ISteamFriends_GetProfileItemPropertyString() {
    SF_FLAT("SteamAPI_ISteamFriends_GetProfileItemPropertyString", false);
    return "";
}

uint64_t SteamAPI_ISteamFriends_GetProfileItemPropertyUint() {
    SF_FLAT("SteamAPI_ISteamFriends_GetProfileItemPropertyUint", false);
    return 0;
}

uint64_t SteamAPI_ISteamFriends_RegisterProtocolInOverlayBrowser() {
    SF_FLAT("SteamAPI_ISteamFriends_RegisterProtocolInOverlayBrowser", false);
    return 0;
}

uint64_t SteamAPI_ISteamFriends_RequestEquippedProfileItems() {
    SF_FLAT("SteamAPI_ISteamFriends_RequestEquippedProfileItems", false);
    return 0;
}

uint64_t SteamAPI_ISteamGameSearch_AcceptGame() {
    SF_FLAT("SteamAPI_ISteamGameSearch_AcceptGame", false);
    return 0;
}

uint64_t SteamAPI_ISteamGameSearch_AddGameSearchParams() {
    SF_FLAT("SteamAPI_ISteamGameSearch_AddGameSearchParams", false);
    return 0;
}

uint64_t SteamAPI_ISteamGameSearch_CancelRequestPlayersForGame() {
    SF_FLAT("SteamAPI_ISteamGameSearch_CancelRequestPlayersForGame", false);
    return 0;
}

uint64_t SteamAPI_ISteamGameSearch_DeclineGame() {
    SF_FLAT("SteamAPI_ISteamGameSearch_DeclineGame", false);
    return 0;
}

uint64_t SteamAPI_ISteamGameSearch_EndGame() {
    SF_FLAT("SteamAPI_ISteamGameSearch_EndGame", false);
    return 0;
}

uint64_t SteamAPI_ISteamGameSearch_EndGameSearch() {
    SF_FLAT("SteamAPI_ISteamGameSearch_EndGameSearch", false);
    return 0;
}

uint64_t SteamAPI_ISteamGameSearch_HostConfirmGameStart() {
    SF_FLAT("SteamAPI_ISteamGameSearch_HostConfirmGameStart", false);
    return 0;
}

uint64_t SteamAPI_ISteamGameSearch_RequestPlayersForGame() {
    SF_FLAT("SteamAPI_ISteamGameSearch_RequestPlayersForGame", false);
    return 0;
}

uint64_t SteamAPI_ISteamGameSearch_RetrieveConnectionDetails() {
    SF_FLAT("SteamAPI_ISteamGameSearch_RetrieveConnectionDetails", false);
    return 0;
}

uint64_t SteamAPI_ISteamGameSearch_SearchForGameSolo() {
    SF_FLAT("SteamAPI_ISteamGameSearch_SearchForGameSolo", false);
    return 0;
}

uint64_t SteamAPI_ISteamGameSearch_SearchForGameWithLobby() {
    SF_FLAT("SteamAPI_ISteamGameSearch_SearchForGameWithLobby", false);
    return 0;
}

uint64_t SteamAPI_ISteamGameSearch_SetConnectionDetails() {
    SF_FLAT("SteamAPI_ISteamGameSearch_SetConnectionDetails", false);
    return 0;
}

uint64_t SteamAPI_ISteamGameSearch_SetGameHostParams() {
    SF_FLAT("SteamAPI_ISteamGameSearch_SetGameHostParams", false);
    return 0;
}

uint64_t SteamAPI_ISteamGameSearch_SubmitPlayerResult() {
    SF_FLAT("SteamAPI_ISteamGameSearch_SubmitPlayerResult", false);
    return 0;
}

void SteamAPI_ISteamHTMLSurface_AddHeader() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_AddHeader", false);
}

void SteamAPI_ISteamHTMLSurface_AllowStartRequest() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_AllowStartRequest", false);
}

void SteamAPI_ISteamHTMLSurface_CopyToClipboard() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_CopyToClipboard", false);
}

uint64_t SteamAPI_ISteamHTMLSurface_CreateBrowser() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_CreateBrowser", false);
    return 0;
}

void SteamAPI_ISteamHTMLSurface_ExecuteJavascript() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_ExecuteJavascript", false);
}

void SteamAPI_ISteamHTMLSurface_FileLoadDialogResponse() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_FileLoadDialogResponse", false);
}

void SteamAPI_ISteamHTMLSurface_Find() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_Find", false);
}

void SteamAPI_ISteamHTMLSurface_GetLinkAtPosition() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_GetLinkAtPosition", false);
}

void SteamAPI_ISteamHTMLSurface_GoBack() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_GoBack", false);
}

void SteamAPI_ISteamHTMLSurface_GoForward() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_GoForward", false);
}

uint64_t SteamAPI_ISteamHTMLSurface_Init() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_Init", false);
    return 0;
}

void SteamAPI_ISteamHTMLSurface_JSDialogResponse() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_JSDialogResponse", false);
}

void SteamAPI_ISteamHTMLSurface_KeyChar() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_KeyChar", false);
}

void SteamAPI_ISteamHTMLSurface_KeyDown() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_KeyDown", false);
}

void SteamAPI_ISteamHTMLSurface_KeyUp() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_KeyUp", false);
}

void SteamAPI_ISteamHTMLSurface_LoadURL() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_LoadURL", false);
}

void SteamAPI_ISteamHTMLSurface_MouseDoubleClick() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_MouseDoubleClick", false);
}

void SteamAPI_ISteamHTMLSurface_MouseDown() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_MouseDown", false);
}

void SteamAPI_ISteamHTMLSurface_MouseMove() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_MouseMove", false);
}

void SteamAPI_ISteamHTMLSurface_MouseUp() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_MouseUp", false);
}

void SteamAPI_ISteamHTMLSurface_MouseWheel() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_MouseWheel", false);
}

void SteamAPI_ISteamHTMLSurface_OpenDeveloperTools() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_OpenDeveloperTools", false);
}

void SteamAPI_ISteamHTMLSurface_PasteFromClipboard() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_PasteFromClipboard", false);
}

void SteamAPI_ISteamHTMLSurface_Reload() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_Reload", false);
}

void SteamAPI_ISteamHTMLSurface_RemoveBrowser() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_RemoveBrowser", false);
}

void SteamAPI_ISteamHTMLSurface_SetBackgroundMode() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_SetBackgroundMode", false);
}

void SteamAPI_ISteamHTMLSurface_SetCookie() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_SetCookie", false);
}

void SteamAPI_ISteamHTMLSurface_SetDPIScalingFactor() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_SetDPIScalingFactor", false);
}

void SteamAPI_ISteamHTMLSurface_SetHorizontalScroll() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_SetHorizontalScroll", false);
}

void SteamAPI_ISteamHTMLSurface_SetKeyFocus() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_SetKeyFocus", false);
}

void SteamAPI_ISteamHTMLSurface_SetPageScaleFactor() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_SetPageScaleFactor", false);
}

void SteamAPI_ISteamHTMLSurface_SetSize() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_SetSize", false);
}

void SteamAPI_ISteamHTMLSurface_SetVerticalScroll() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_SetVerticalScroll", false);
}

uint64_t SteamAPI_ISteamHTMLSurface_Shutdown() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_Shutdown", false);
    return 0;
}

void SteamAPI_ISteamHTMLSurface_StopFind() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_StopFind", false);
}

void SteamAPI_ISteamHTMLSurface_StopLoad() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_StopLoad", false);
}

void SteamAPI_ISteamHTMLSurface_ViewSource() {
    SF_FLAT("SteamAPI_ISteamHTMLSurface_ViewSource", false);
}

uint64_t SteamAPI_ISteamInput_BNewDataAvailable() {
    SF_FLAT("SteamAPI_ISteamInput_BNewDataAvailable", false);
    return 0;
}

uint64_t SteamAPI_ISteamInput_BWaitForData() {
    SF_FLAT("SteamAPI_ISteamInput_BWaitForData", false);
    return 0;
}

void SteamAPI_ISteamInput_EnableActionEventCallbacks() {
    SF_FLAT("SteamAPI_ISteamInput_EnableActionEventCallbacks", false);
}

void SteamAPI_ISteamInput_EnableDeviceCallbacks() {
    SF_FLAT("SteamAPI_ISteamInput_EnableDeviceCallbacks", false);
}

uint64_t SteamAPI_ISteamInput_GetControllerForGamepadIndex() {
    SF_FLAT("SteamAPI_ISteamInput_GetControllerForGamepadIndex", false);
    return 0;
}

uint64_t SteamAPI_ISteamInput_GetGamepadIndexForController() {
    SF_FLAT("SteamAPI_ISteamInput_GetGamepadIndexForController", false);
    return 0;
}

const char* SteamAPI_ISteamInput_GetGlyphForXboxOrigin() {
    SF_FLAT("SteamAPI_ISteamInput_GetGlyphForXboxOrigin", false);
    return "";
}

const char* SteamAPI_ISteamInput_GetGlyphPNGForActionOrigin() {
    SF_FLAT("SteamAPI_ISteamInput_GetGlyphPNGForActionOrigin", false);
    return "";
}

const char* SteamAPI_ISteamInput_GetGlyphSVGForActionOrigin() {
    SF_FLAT("SteamAPI_ISteamInput_GetGlyphSVGForActionOrigin", false);
    return "";
}

uint64_t SteamAPI_ISteamInput_GetInputTypeForHandle() {
    SF_FLAT("SteamAPI_ISteamInput_GetInputTypeForHandle", false);
    return 0;
}

const char* SteamAPI_ISteamInput_GetStringForActionOrigin() {
    SF_FLAT("SteamAPI_ISteamInput_GetStringForActionOrigin", false);
    return "";
}

const char* SteamAPI_ISteamInput_GetStringForAnalogActionName() {
    SF_FLAT("SteamAPI_ISteamInput_GetStringForAnalogActionName", false);
    return "";
}

const char* SteamAPI_ISteamInput_GetStringForXboxOrigin() {
    SF_FLAT("SteamAPI_ISteamInput_GetStringForXboxOrigin", false);
    return "";
}

void SteamAPI_ISteamInput_Legacy_TriggerHapticPulse() {
    SF_FLAT("SteamAPI_ISteamInput_Legacy_TriggerHapticPulse", false);
}

void SteamAPI_ISteamInput_Legacy_TriggerRepeatedHapticPulse() {
    SF_FLAT("SteamAPI_ISteamInput_Legacy_TriggerRepeatedHapticPulse", false);
}

uint64_t SteamAPI_ISteamInput_SetInputActionManifestFilePath() {
    SF_FLAT("SteamAPI_ISteamInput_SetInputActionManifestFilePath", false);
    return 0;
}

uint64_t SteamAPI_ISteamInput_ShowBindingPanel() {
    SF_FLAT("SteamAPI_ISteamInput_ShowBindingPanel", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_BActivationSuccess() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_BActivationSuccess", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_BIsCurrentMusicRemote() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_BIsCurrentMusicRemote", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_CurrentEntryDidChange() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_CurrentEntryDidChange", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_CurrentEntryIsAvailable() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_CurrentEntryIsAvailable", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_CurrentEntryWillChange() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_CurrentEntryWillChange", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_DeregisterSteamMusicRemote() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_DeregisterSteamMusicRemote", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_EnableLooped() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_EnableLooped", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_EnablePlayNext() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_EnablePlayNext", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_EnablePlayPrevious() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_EnablePlayPrevious", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_EnablePlaylists() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_EnablePlaylists", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_EnableQueue() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_EnableQueue", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_EnableShuffled() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_EnableShuffled", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_PlaylistDidChange() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_PlaylistDidChange", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_PlaylistWillChange() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_PlaylistWillChange", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_QueueDidChange() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_QueueDidChange", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_QueueWillChange() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_QueueWillChange", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_RegisterSteamMusicRemote() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_RegisterSteamMusicRemote", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_ResetPlaylistEntries() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_ResetPlaylistEntries", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_ResetQueueEntries() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_ResetQueueEntries", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_SetCurrentPlaylistEntry() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_SetCurrentPlaylistEntry", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_SetCurrentQueueEntry() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_SetCurrentQueueEntry", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_SetDisplayName() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_SetDisplayName", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_SetPNGIcon_64x64() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_SetPNGIcon_64x64", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_SetPlaylistEntry() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_SetPlaylistEntry", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_SetQueueEntry() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_SetQueueEntry", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_UpdateCurrentEntryCoverArt() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_UpdateCurrentEntryCoverArt", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_UpdateCurrentEntryElapsedSeconds() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_UpdateCurrentEntryElapsedSeconds", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_UpdateCurrentEntryText() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_UpdateCurrentEntryText", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_UpdateLooped() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_UpdateLooped", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_UpdatePlaybackStatus() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_UpdatePlaybackStatus", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_UpdateShuffled() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_UpdateShuffled", false);
    return 0;
}

uint64_t SteamAPI_ISteamMusicRemote_UpdateVolume() {
    SF_FLAT("SteamAPI_ISteamMusicRemote_UpdateVolume", false);
    return 0;
}

void SteamAPI_ISteamNetworkingConnectionSignaling_Release() {
    SF_FLAT("SteamAPI_ISteamNetworkingConnectionSignaling_Release", false);
}

uint64_t SteamAPI_ISteamNetworkingConnectionSignaling_SendSignal() {
    SF_FLAT("SteamAPI_ISteamNetworkingConnectionSignaling_SendSignal", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingMessages_AcceptSessionWithUser() {
    SF_FLAT("SteamAPI_ISteamNetworkingMessages_AcceptSessionWithUser", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingMessages_CloseChannelWithUser() {
    SF_FLAT("SteamAPI_ISteamNetworkingMessages_CloseChannelWithUser", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingMessages_CloseSessionWithUser() {
    SF_FLAT("SteamAPI_ISteamNetworkingMessages_CloseSessionWithUser", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingMessages_GetSessionConnectionInfo() {
    SF_FLAT("SteamAPI_ISteamNetworkingMessages_GetSessionConnectionInfo", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingMessages_ReceiveMessagesOnChannel() {
    SF_FLAT("SteamAPI_ISteamNetworkingMessages_ReceiveMessagesOnChannel", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingMessages_SendMessageToUser() {
    SF_FLAT("SteamAPI_ISteamNetworkingMessages_SendMessageToUser", false);
    return 0;
}

void* SteamAPI_ISteamNetworkingSignalingRecvContext_OnConnectRequest() {
    SF_FLAT("SteamAPI_ISteamNetworkingSignalingRecvContext_OnConnectRequest", false);
    return nullptr;
}

void SteamAPI_ISteamNetworkingSignalingRecvContext_SendRejectionSignal() {
    SF_FLAT("SteamAPI_ISteamNetworkingSignalingRecvContext_SendRejectionSignal", false);
}

uint64_t SteamAPI_ISteamNetworkingSockets_AcceptConnection() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_AcceptConnection", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_BeginAsyncRequestFakeIP() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_BeginAsyncRequestFakeIP", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_CloseConnection() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_CloseConnection", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_CloseListenSocket() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_CloseListenSocket", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_ConfigureConnectionLanes() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_ConfigureConnectionLanes", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_ConnectByIPAddress() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_ConnectByIPAddress", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_ConnectP2P() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_ConnectP2P", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_ConnectP2PCustomSignaling() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_ConnectP2PCustomSignaling", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_ConnectToHostedDedicatedServer() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_ConnectToHostedDedicatedServer", false);
    return 0;
}

void* SteamAPI_ISteamNetworkingSockets_CreateFakeUDPPort() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_CreateFakeUDPPort", false);
    return nullptr;
}

uint64_t SteamAPI_ISteamNetworkingSockets_CreateHostedDedicatedServerListenSocket() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_CreateHostedDedicatedServerListenSocket", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_CreateListenSocketIP() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_CreateListenSocketIP", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_CreateListenSocketP2P() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_CreateListenSocketP2P", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_CreateListenSocketP2PFakeIP() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_CreateListenSocketP2PFakeIP", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_CreatePollGroup() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_CreatePollGroup", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_CreateSocketPair() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_CreateSocketPair", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_DestroyPollGroup() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_DestroyPollGroup", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_FindRelayAuthTicketForServer() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_FindRelayAuthTicketForServer", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_FlushMessagesOnConnection() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_FlushMessagesOnConnection", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_GetAuthenticationStatus() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_GetAuthenticationStatus", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_GetCertificateRequest() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_GetCertificateRequest", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_GetConnectionInfo() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_GetConnectionInfo", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_GetConnectionName() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_GetConnectionName", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_GetConnectionRealTimeStatus() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_GetConnectionRealTimeStatus", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_GetConnectionUserData() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_GetConnectionUserData", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_GetDetailedConnectionStatus() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_GetDetailedConnectionStatus", false);
    return 0;
}

void SteamAPI_ISteamNetworkingSockets_GetFakeIP() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_GetFakeIP", false);
}

uint64_t SteamAPI_ISteamNetworkingSockets_GetGameCoordinatorServerLogin() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_GetGameCoordinatorServerLogin", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_GetHostedDedicatedServerAddress() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_GetHostedDedicatedServerAddress", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_GetHostedDedicatedServerPOPID() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_GetHostedDedicatedServerPOPID", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_GetHostedDedicatedServerPort() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_GetHostedDedicatedServerPort", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_GetIdentity() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_GetIdentity", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_GetListenSocketAddress() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_GetListenSocketAddress", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_GetRemoteFakeIPForConnection() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_GetRemoteFakeIPForConnection", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_InitAuthentication() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_InitAuthentication", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_ReceiveMessagesOnConnection() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_ReceiveMessagesOnConnection", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_ReceiveMessagesOnPollGroup() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_ReceiveMessagesOnPollGroup", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_ReceivedP2PCustomSignal() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_ReceivedP2PCustomSignal", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_ReceivedRelayAuthTicket() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_ReceivedRelayAuthTicket", false);
    return 0;
}

void SteamAPI_ISteamNetworkingSockets_ResetIdentity() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_ResetIdentity", false);
}

void SteamAPI_ISteamNetworkingSockets_RunCallbacks() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_RunCallbacks", false);
}

uint64_t SteamAPI_ISteamNetworkingSockets_SendMessageToConnection() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_SendMessageToConnection", false);
    return 0;
}

void SteamAPI_ISteamNetworkingSockets_SendMessages() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_SendMessages", false);
}

uint64_t SteamAPI_ISteamNetworkingSockets_SetCertificate() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_SetCertificate", false);
    return 0;
}

void SteamAPI_ISteamNetworkingSockets_SetConnectionName() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_SetConnectionName", false);
}

uint64_t SteamAPI_ISteamNetworkingSockets_SetConnectionPollGroup() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_SetConnectionPollGroup", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingSockets_SetConnectionUserData() {
    SF_FLAT("SteamAPI_ISteamNetworkingSockets_SetConnectionUserData", false);
    return 0;
}

void* SteamAPI_ISteamNetworkingUtils_AllocateMessage() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_AllocateMessage", false);
    return nullptr;
}

uint64_t SteamAPI_ISteamNetworkingUtils_CheckPingDataUpToDate() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_CheckPingDataUpToDate", false);
    return 0;
}

void SteamAPI_ISteamNetworkingUtils_ConvertPingLocationToString() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_ConvertPingLocationToString", false);
}

uint64_t SteamAPI_ISteamNetworkingUtils_EstimatePingTimeBetweenTwoLocations() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_EstimatePingTimeBetweenTwoLocations", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingUtils_EstimatePingTimeFromLocalHost() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_EstimatePingTimeFromLocalHost", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingUtils_GetConfigValue() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_GetConfigValue", false);
    return 0;
}

const char* SteamAPI_ISteamNetworkingUtils_GetConfigValueInfo() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_GetConfigValueInfo", false);
    return "";
}

uint64_t SteamAPI_ISteamNetworkingUtils_GetDirectPingToPOP() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_GetDirectPingToPOP", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingUtils_GetIPv4FakeIPType() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_GetIPv4FakeIPType", false);
    return 0;
}

float SteamAPI_ISteamNetworkingUtils_GetLocalPingLocation() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_GetLocalPingLocation", false);
    return 0.0f;
}

uint64_t SteamAPI_ISteamNetworkingUtils_GetLocalTimestamp() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_GetLocalTimestamp", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingUtils_GetPOPCount() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_GetPOPCount", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingUtils_GetPOPList() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_GetPOPList", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingUtils_GetPingToDataCenter() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_GetPingToDataCenter", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingUtils_GetRealIdentityForFakeIP() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_GetRealIdentityForFakeIP", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingUtils_GetRelayNetworkStatus() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_GetRelayNetworkStatus", false);
    return 0;
}

void SteamAPI_ISteamNetworkingUtils_InitRelayNetworkAccess() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_InitRelayNetworkAccess", false);
}

uint64_t SteamAPI_ISteamNetworkingUtils_IsFakeIPv4() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_IsFakeIPv4", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingUtils_IterateGenericEditableConfigValues() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_IterateGenericEditableConfigValues", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingUtils_ParsePingLocationString() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_ParsePingLocationString", false);
    return 0;
}

uint64_t SteamAPI_ISteamNetworkingUtils_SetConfigValue() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_SetConfigValue", false);
    return 0;
}

void SteamAPI_ISteamNetworkingUtils_SetDebugOutputFunction() {
    SF_FLAT("SteamAPI_ISteamNetworkingUtils_SetDebugOutputFunction", false);
}

void SteamAPI_ISteamParties_CancelReservation() {
    SF_FLAT("SteamAPI_ISteamParties_CancelReservation", false);
}

uint64_t SteamAPI_ISteamParties_ChangeNumOpenSlots() {
    SF_FLAT("SteamAPI_ISteamParties_ChangeNumOpenSlots", false);
    return 0;
}

uint64_t SteamAPI_ISteamParties_CreateBeacon() {
    SF_FLAT("SteamAPI_ISteamParties_CreateBeacon", false);
    return 0;
}

uint64_t SteamAPI_ISteamParties_DestroyBeacon() {
    SF_FLAT("SteamAPI_ISteamParties_DestroyBeacon", false);
    return 0;
}

uint64_t SteamAPI_ISteamParties_GetAvailableBeaconLocations() {
    SF_FLAT("SteamAPI_ISteamParties_GetAvailableBeaconLocations", false);
    return 0;
}

uint64_t SteamAPI_ISteamParties_GetBeaconByIndex() {
    SF_FLAT("SteamAPI_ISteamParties_GetBeaconByIndex", false);
    return 0;
}

uint64_t SteamAPI_ISteamParties_GetBeaconDetails() {
    SF_FLAT("SteamAPI_ISteamParties_GetBeaconDetails", false);
    return 0;
}

uint64_t SteamAPI_ISteamParties_GetBeaconLocationData() {
    SF_FLAT("SteamAPI_ISteamParties_GetBeaconLocationData", false);
    return 0;
}

uint64_t SteamAPI_ISteamParties_GetNumActiveBeacons() {
    SF_FLAT("SteamAPI_ISteamParties_GetNumActiveBeacons", false);
    return 0;
}

uint64_t SteamAPI_ISteamParties_GetNumAvailableBeaconLocations() {
    SF_FLAT("SteamAPI_ISteamParties_GetNumAvailableBeaconLocations", false);
    return 0;
}

uint64_t SteamAPI_ISteamParties_JoinParty() {
    SF_FLAT("SteamAPI_ISteamParties_JoinParty", false);
    return 0;
}

void SteamAPI_ISteamParties_OnReservationCompleted() {
    SF_FLAT("SteamAPI_ISteamParties_OnReservationCompleted", false);
}

uint64_t SteamAPI_ISteamRemotePlay_BGetSessionClientResolution() {
    SF_FLAT("SteamAPI_ISteamRemotePlay_BGetSessionClientResolution", false);
    return 0;
}

uint64_t SteamAPI_ISteamRemotePlay_BSendRemotePlayTogetherInvite() {
    SF_FLAT("SteamAPI_ISteamRemotePlay_BSendRemotePlayTogetherInvite", false);
    return 0;
}

uint64_t SteamAPI_ISteamRemotePlay_GetSessionClientFormFactor() {
    SF_FLAT("SteamAPI_ISteamRemotePlay_GetSessionClientFormFactor", false);
    return 0;
}

const char* SteamAPI_ISteamRemotePlay_GetSessionClientName() {
    SF_FLAT("SteamAPI_ISteamRemotePlay_GetSessionClientName", false);
    return "";
}

uint64_t SteamAPI_ISteamRemotePlay_GetSessionCount() {
    SF_FLAT("SteamAPI_ISteamRemotePlay_GetSessionCount", false);
    return 0;
}

uint64_t SteamAPI_ISteamRemotePlay_GetSessionID() {
    SF_FLAT("SteamAPI_ISteamRemotePlay_GetSessionID", false);
    return 0;
}

uint64_t SteamAPI_ISteamRemotePlay_GetSessionSteamID() {
    SF_FLAT("SteamAPI_ISteamRemotePlay_GetSessionSteamID", false);
    return 0;
}

const char* SteamAPI_ISteamRemoteStorage_GetLocalFileChange() {
    SF_FLAT("SteamAPI_ISteamRemoteStorage_GetLocalFileChange", false);
    return "";
}

uint64_t SteamAPI_ISteamUGC_AddAppDependency() {
    SF_FLAT("SteamAPI_ISteamUGC_AddAppDependency", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_AddContentDescriptor() {
    SF_FLAT("SteamAPI_ISteamUGC_AddContentDescriptor", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_AddDependency() {
    SF_FLAT("SteamAPI_ISteamUGC_AddDependency", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_BInitWorkshopForGameServer() {
    SF_FLAT("SteamAPI_ISteamUGC_BInitWorkshopForGameServer", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_CreateQueryAllUGCRequestCursor() {
    SF_FLAT("SteamAPI_ISteamUGC_CreateQueryAllUGCRequestCursor", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_DownloadItem() {
    SF_FLAT("SteamAPI_ISteamUGC_DownloadItem", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_GetAppDependencies() {
    SF_FLAT("SteamAPI_ISteamUGC_GetAppDependencies", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_GetQueryFirstUGCKeyValueTag() {
    SF_FLAT("SteamAPI_ISteamUGC_GetQueryFirstUGCKeyValueTag", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_GetQueryUGCMetadata() {
    SF_FLAT("SteamAPI_ISteamUGC_GetQueryUGCMetadata", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_GetQueryUGCNumTags() {
    SF_FLAT("SteamAPI_ISteamUGC_GetQueryUGCNumTags", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_GetQueryUGCTag() {
    SF_FLAT("SteamAPI_ISteamUGC_GetQueryUGCTag", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_GetQueryUGCTagDisplayName() {
    SF_FLAT("SteamAPI_ISteamUGC_GetQueryUGCTagDisplayName", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_GetWorkshopEULAStatus() {
    SF_FLAT("SteamAPI_ISteamUGC_GetWorkshopEULAStatus", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_RemoveAppDependency() {
    SF_FLAT("SteamAPI_ISteamUGC_RemoveAppDependency", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_RemoveContentDescriptor() {
    SF_FLAT("SteamAPI_ISteamUGC_RemoveContentDescriptor", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_RemoveDependency() {
    SF_FLAT("SteamAPI_ISteamUGC_RemoveDependency", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_SetTimeCreatedDateRange() {
    SF_FLAT("SteamAPI_ISteamUGC_SetTimeCreatedDateRange", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_SetTimeUpdatedDateRange() {
    SF_FLAT("SteamAPI_ISteamUGC_SetTimeUpdatedDateRange", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_ShowWorkshopEULA() {
    SF_FLAT("SteamAPI_ISteamUGC_ShowWorkshopEULA", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_StartPlaytimeTracking() {
    SF_FLAT("SteamAPI_ISteamUGC_StartPlaytimeTracking", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_StopPlaytimeTracking() {
    SF_FLAT("SteamAPI_ISteamUGC_StopPlaytimeTracking", false);
    return 0;
}

uint64_t SteamAPI_ISteamUGC_StopPlaytimeTrackingForAllItems() {
    SF_FLAT("SteamAPI_ISteamUGC_StopPlaytimeTrackingForAllItems", false);
    return 0;
}

void SteamAPI_ISteamUGC_SuspendDownloads() {
    SF_FLAT("SteamAPI_ISteamUGC_SuspendDownloads", false);
}

uint64_t SteamAPI_ISteamUserStats_GetAchievementProgressLimitsFloat() {
    SF_FLAT("SteamAPI_ISteamUserStats_GetAchievementProgressLimitsFloat", false);
    return 0;
}

uint64_t SteamAPI_ISteamUserStats_GetAchievementProgressLimitsInt32() {
    SF_FLAT("SteamAPI_ISteamUserStats_GetAchievementProgressLimitsInt32", false);
    return 0;
}

uint64_t SteamAPI_ISteamUtils_DismissFloatingGamepadTextInput() {
    SF_FLAT("SteamAPI_ISteamUtils_DismissFloatingGamepadTextInput", false);
    return 0;
}

void SteamAPI_ISteamUtils_SetGameLauncherMode() {
    SF_FLAT("SteamAPI_ISteamUtils_SetGameLauncherMode", false);
}

uint64_t SteamAPI_ISteamUtils_ShowFloatingGamepadTextInput() {
    SF_FLAT("SteamAPI_ISteamUtils_ShowFloatingGamepadTextInput", false);
    return 0;
}

void SteamAPI_ISteamUtils_StartVRDashboard() {
    SF_FLAT("SteamAPI_ISteamUtils_StartVRDashboard", false);
}

} // extern "C"
