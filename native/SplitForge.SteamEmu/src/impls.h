#pragma once

// Instancias únicas (por proceso) de cada interfaz emulada. Sin destructores virtuales:
// los objetos viven toda la vida de la DLL y nunca se eliminan por la base.

#include "interfaces.h"

namespace sfemu {

ISteamClient* ClientInstance();
ISteamUser* UserInstance();
ISteamFriends* FriendsInstance();
ISteamUtils* UtilsInstance();
ISteamApps* AppsInstance();
ISteamMatchmaking* MatchmakingInstance();
ISteamMatchmakingServers* MatchmakingServersInstance();
ISteamUserStats* UserStatsInstance();
ISteamGameServerStats* GameServerStatsInstance();
ISteamNetworking* NetworkingInstance();
ISteamRemoteStorage* RemoteStorageInstance();
ISteamScreenshots* ScreenshotsInstance();
ISteamHTTP* HttpInstance();
ISteamController* ControllerInstance();
ISteamUGC* UgcInstance();
ISteamAppList* AppListInstance();
ISteamMusic* MusicInstance();
ISteamInventory* InventoryInstance();
ISteamVideo* VideoInstance();
ISteamParentalSettings* ParentalSettingsInstance();
ISteamInput* InputInstance();
ISteamGameServer* GameServerInstance();
void* GameSearchInstance();
void* MusicRemoteInstance();
void* HtmlSurfaceInstance();
void* PartiesInstance();
void* RemotePlayInstance();
void* NetworkingUtilsInstance();
void* NetworkingSocketsInstance();
void* NetworkingMessagesInstance();
void* ResolveGeneric(const char* version);

} // namespace sfemu
