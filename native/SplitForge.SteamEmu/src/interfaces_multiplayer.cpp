#include "impls.h"
#include "interfaces.h"
#include "state.h"

#include <algorithm>
#include <cstring>

namespace sfemu {

namespace {

class SteamMatchmakingImpl final : public ISteamMatchmaking {
public:
    int GetFavoriteGameCount() override { return 0; }
    bool GetFavoriteGame(int, AppId_t*, uint32_t*, uint16_t*, uint16_t*, uint32_t*, uint32_t*) override {
        return false;
    }
    int AddFavoriteGame(AppId_t, uint32_t, uint16_t, uint16_t, uint32_t, uint32_t) override { return 0; }
    bool RemoveFavoriteGame(AppId_t, uint32_t, uint16_t, uint16_t, uint32_t) override { return false; }
    // Devuelve los lobbies que anuncian las demás instancias (el emulador no tiene servidor de Steam).
    SteamAPICall_t RequestLobbyList() override {
        EmuState& emu = Emu();
        emu.RefreshRemoteLobbies();
        emu.lobbySearch.clear();
        for (const auto& [id, lobby] : emu.remoteLobbies) {
            bool match = true;
            for (const LobbyFilter& filter : emu.lobbyFilters) {
                const auto data = lobby.data.find(filter.key);
                const bool equal = data != lobby.data.end() && data->second == filter.value;
                // k_ELobbyComparisonEqual (0) y NotEqual (3); las comparaciones de orden no descartan.
                if ((filter.comparison == 0 && !equal) || (filter.comparison == 3 && equal)) {
                    match = false;
                    break;
                }
            }
            if (match)
                emu.lobbySearch.push_back(id);
        }
        emu.lobbyFilters.clear();
        static size_t lastLogged = static_cast<size_t>(-1);
        if (emu.lobbySearch.size() != lastLogged) { // Solo cuando cambia: el juego repite la búsqueda cada pocos segundos.
            lastLogged = emu.lobbySearch.size();
            emu.Log("Búsqueda de lobbies: %zu de %zu anunciados por otras instancias", emu.lobbySearch.size(),
                    emu.remoteLobbies.size());
        }
        LobbyMatchList_t result{};
        result.m_nLobbiesMatching = static_cast<uint32_t>(emu.lobbySearch.size());
        return emu.QueueCallResult(k_iCallback_LobbyMatchList, &result, sizeof(result));
    }
    void AddRequestLobbyListStringFilter(const char* key, const char* value, int comparison) override {
        if (key && value)
            Emu().lobbyFilters.push_back({key, value, comparison});
    }
    void AddRequestLobbyListNumericalFilter(const char*, int, int) override {}
    void AddRequestLobbyListNearValueFilter(const char*, int) override {}
    void AddRequestLobbyListFilterSlotsAvailable(int) override {}
    void AddRequestLobbyListDistanceFilter(int) override {}
    void AddRequestLobbyListResultCountFilter(int) override {}
    void AddRequestLobbyListCompatibleMembersFilter(CSteamID) override {}
    CSteamID GetLobbyByIndex(int index) override {
        EmuState& emu = Emu();
        return index >= 0 && index < static_cast<int>(emu.lobbySearch.size())
                   ? emu.lobbySearch[static_cast<size_t>(index)] : k_steamIDNil;
    }
    SteamAPICall_t CreateLobby(int type, int maxMembers) override {
        EmuState& emu = Emu();
        Lobby& lobby = emu.GetOrCreateLobby(emu.NewLobbyId());
        lobby.type = type;
        lobby.memberLimit = maxMembers > 0 ? maxMembers : 4;
        LobbyCreated_t created{};
        created.m_eResult = k_EResultOK;
        created.m_ulSteamIDLobby = lobby.id;
        // Steam lo entrega como call result y también como callback (7DTD usa Callback<LobbyCreated_t>),
        // y siempre antes que LobbyEnter: si no, el anfitrión toma su propio lobby por uno ajeno.
        emu.QueueCallback(k_iCallback_LobbyCreated, &created, sizeof(created));
        const SteamAPICall_t call = emu.QueueCallResult(k_iCallback_LobbyCreated, &created, sizeof(created));
        LobbyEnter_t entered{};
        entered.m_ulSteamIDLobby = lobby.id;
        entered.m_rgfChatPermissions = 0xFFFFFFFF;
        entered.m_bLocked = false;
        entered.m_EChatRoomEnterResponse = k_EChatRoomEnterResponseSuccess;
        emu.QueueCallback(k_iCallback_LobbyEnter, &entered, sizeof(entered));
        emu.AnnounceLobby(lobby);
        return call;
    }
    SteamAPICall_t JoinLobby(CSteamID lobbyId) override {
        EmuState& emu = Emu();
        if (!emu.lobbies.count(lobbyId))
            emu.RefreshRemoteLobbies();
        Lobby* lobby = emu.FindLobby(lobbyId);
        LobbyEnter_t entered{};
        entered.m_ulSteamIDLobby = lobbyId;
        entered.m_rgfChatPermissions = 0xFFFFFFFF;
        entered.m_bLocked = false;
        entered.m_EChatRoomEnterResponse = lobby ? k_EChatRoomEnterResponseSuccess : k_EChatRoomEnterResponseDoesntExist;
        if (lobby && std::find(lobby->members.begin(), lobby->members.end(), emu.config.steamId) == lobby->members.end())
            lobby->members.push_back(emu.config.steamId);
        // Como Steam: callback y call result, y después los datos del lobby.
        emu.QueueCallback(k_iCallback_LobbyEnter, &entered, sizeof(entered));
        const SteamAPICall_t call = emu.QueueCallResult(k_iCallback_LobbyEnter, &entered, sizeof(entered));
        if (lobby)
            QueueDataUpdate(lobbyId);
        return call;
    }
    void LeaveLobby(CSteamID lobbyId) override {
        EmuState& emu = Emu();
        emu.net.RemoveLocalLobby(lobbyId);
        emu.lobbies.erase(lobbyId);
        if (const auto it = emu.remoteLobbies.find(lobbyId); it != emu.remoteLobbies.end()) {
            auto& members = it->second.members;
            members.erase(std::remove(members.begin(), members.end(), emu.config.steamId), members.end());
        }
    }
    bool InviteUserToLobby(CSteamID, CSteamID) override { return false; }
    int GetNumLobbyMembers(CSteamID lobbyId) override {
        const Lobby* lobby = Emu().FindLobby(lobbyId);
        return lobby ? static_cast<int>(lobby->members.size()) : 0;
    }
    CSteamID GetLobbyMemberByIndex(CSteamID lobbyId, int index) override {
        const Lobby* lobby = Emu().FindLobby(lobbyId);
        if (!lobby || index < 0 || index >= static_cast<int>(lobby->members.size()))
            return k_steamIDNil;
        return lobby->members[static_cast<size_t>(index)];
    }
    const char* GetLobbyData(CSteamID lobbyId, const char* key) override {
        const Lobby* lobby = Emu().FindLobby(lobbyId);
        if (!lobby || !key)
            return "";
        const auto data = lobby->data.find(key);
        return data == lobby->data.end() ? "" : data->second.c_str();
    }
    bool SetLobbyData(CSteamID lobbyId, const char* key, const char* value) override {
        if (!key)
            return false;
        EmuState& emu = Emu();
        if (!emu.lobbies.count(lobbyId))
            return false;
        Lobby& lobby = emu.lobbies[lobbyId];
        lobby.data[key] = value ? value : "";
        QueueDataUpdate(lobbyId);
        emu.AnnounceLobby(lobby);
        return true;
    }
    static void QueueDataUpdate(CSteamID lobbyId) {
        LobbyDataUpdate_t update{};
        update.m_ulSteamIDLobby = lobbyId;
        update.m_ulSteamIDMember = lobbyId; // Igual que el lobby: cambiaron los datos del lobby, no los de un miembro.
        update.m_bSuccess = 1;
        Emu().QueueCallback(k_iCallback_LobbyDataUpdate, &update, sizeof(update));
    }
    int GetLobbyDataCount(CSteamID lobbyId) override {
        const Lobby* lobby = Emu().FindLobby(lobbyId);
        return lobby ? static_cast<int>(lobby->data.size()) : 0;
    }
    bool GetLobbyDataByIndex(CSteamID lobbyId, int index, char* key, int keySize, char* value,
                             int valueSize) override {
        const Lobby* found = Emu().FindLobby(lobbyId);
        if (!found || index < 0)
            return false;
        int i = 0;
        for (const auto& [k, v] : found->data) {
            if (i++ != index)
                continue;
            if (key && keySize > 0) {
                std::strncpy(key, k.c_str(), static_cast<size_t>(keySize) - 1);
                key[keySize - 1] = 0;
            }
            if (value && valueSize > 0) {
                std::strncpy(value, v.c_str(), static_cast<size_t>(valueSize) - 1);
                value[valueSize - 1] = 0;
            }
            return true;
        }
        return false;
    }
    bool DeleteLobbyData(CSteamID lobbyId, const char* key) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        if (it == emu.lobbies.end() || !key)
            return false;
        const bool erased = it->second.data.erase(key) > 0;
        if (erased)
            emu.AnnounceLobby(it->second);
        return erased;
    }
    const char* GetLobbyMemberData(CSteamID lobbyId, CSteamID member, const char* key) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        if (it == emu.lobbies.end() || !key)
            return "";
        const auto memberIt = it->second.memberData.find(member);
        if (memberIt == it->second.memberData.end())
            return "";
        const auto data = memberIt->second.find(key);
        return data == memberIt->second.end() ? "" : data->second.c_str();
    }
    void SetLobbyMemberData(CSteamID lobbyId, const char* key, const char* value) override {
        if (!key)
            return;
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        if (it == emu.lobbies.end())
            return;
        it->second.memberData[emu.config.steamId][key] = value ? value : "";
    }
    bool SendLobbyChatMsg(CSteamID lobbyId, const void* data, int size) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        if (it == emu.lobbies.end() || !data || size <= 0)
            return false;
        Lobby& lobby = it->second;
        ChatMessage message;
        message.sender = emu.config.steamId;
        message.type = k_EChatEntryTypeChatMsg;
        message.id = lobby.nextChatId++;
        message.data.assign(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + size);
        lobby.chat.push_back(std::move(message));
        LobbyChatMsg_t event{};
        event.m_ulSteamIDLobby = lobbyId;
        event.m_ulSteamIDUser = emu.config.steamId;
        event.m_eChatEntryType = k_EChatEntryTypeChatMsg;
        event.m_iChatID = lobby.nextChatId - 1;
        emu.QueueCallback(k_iCallback_LobbyChatMsg, &event, sizeof(event));
        return true;
    }
    int GetLobbyChatEntry(CSteamID lobbyId, int chatId, CSteamID* user, void* buffer, int size,
                          int* type) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        if (it == emu.lobbies.end())
            return 0;
        for (const auto& message : it->second.chat) {
            if (message.id != static_cast<uint32_t>(chatId))
                continue;
            if (user)
                *user = message.sender;
            if (type)
                *type = message.type;
            const size_t copy = size < static_cast<int>(message.data.size()) ? static_cast<size_t>(size)
                                                                             : message.data.size();
            if (buffer && copy)
                std::memcpy(buffer, message.data.data(), copy);
            return static_cast<int>(copy);
        }
        return 0;
    }
    // Datos de un lobby ajeno: se toma lo último que anunció su dueño y se avisa con LobbyDataUpdate_t.
    bool RequestLobbyData(CSteamID lobbyId) override {
        EmuState& emu = Emu();
        if (!emu.lobbies.count(lobbyId))
            emu.RefreshRemoteLobbies();
        LobbyDataUpdate_t update{};
        update.m_ulSteamIDLobby = lobbyId;
        update.m_ulSteamIDMember = lobbyId;
        update.m_bSuccess = emu.FindLobby(lobbyId) ? 1 : 0;
        emu.QueueCallback(k_iCallback_LobbyDataUpdate, &update, sizeof(update));
        return true;
    }
    void SetLobbyGameServer(CSteamID lobbyId, uint32_t ip, uint16_t port, CSteamID serverId) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        if (it == emu.lobbies.end())
            return;
        it->second.gameServerIp = ip;
        it->second.gameServerPort = port;
        it->second.gameServerSteamId = serverId;
        emu.AnnounceLobby(it->second);
    }
    bool GetLobbyGameServer(CSteamID lobbyId, uint32_t* ip, uint16_t* port, CSteamID* serverId) override {
        const Lobby* lobby = Emu().FindLobby(lobbyId);
        if (!lobby)
            return false;
        if (ip)
            *ip = lobby->gameServerIp;
        if (port)
            *port = lobby->gameServerPort;
        if (serverId)
            *serverId = lobby->gameServerSteamId;
        return lobby->gameServerIp != 0 || lobby->gameServerSteamId != 0;
    }
    bool SetLobbyMemberLimit(CSteamID lobbyId, int limit) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        if (it == emu.lobbies.end())
            return false;
        it->second.memberLimit = limit;
        emu.AnnounceLobby(it->second);
        return true;
    }
    int GetLobbyMemberLimit(CSteamID lobbyId) override {
        const Lobby* lobby = Emu().FindLobby(lobbyId);
        return lobby ? lobby->memberLimit : 0;
    }
    bool SetLobbyType(CSteamID lobbyId, int type) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        if (it == emu.lobbies.end())
            return false;
        it->second.type = type;
        emu.AnnounceLobby(it->second);
        return true;
    }
    bool SetLobbyJoinable(CSteamID, bool) override { return true; }
    CSteamID GetLobbyOwner(CSteamID lobbyId) override {
        const Lobby* lobby = Emu().FindLobby(lobbyId);
        return lobby ? lobby->owner : k_steamIDNil;
    }
    bool SetLobbyOwner(CSteamID lobbyId, CSteamID owner) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        if (it == emu.lobbies.end())
            return false;
        it->second.owner = owner;
        return true;
    }
    bool SetLinkedLobby(CSteamID, CSteamID) override { return true; }
};

class SteamMatchmakingServersImpl final : public ISteamMatchmakingServers {
public:
    HServerListRequest RequestInternetServerList(AppId_t, MatchMakingKeyValuePair_t**, uint32_t,
                                                 ISteamMatchmakingServerListResponse*) override {
        return nullptr;
    }
    HServerListRequest RequestLANServerList(AppId_t, ISteamMatchmakingServerListResponse*) override {
        return nullptr;
    }
    HServerListRequest RequestFriendsServerList(AppId_t, MatchMakingKeyValuePair_t**, uint32_t,
                                                ISteamMatchmakingServerListResponse*) override {
        return nullptr;
    }
    HServerListRequest RequestFavoritesServerList(AppId_t, MatchMakingKeyValuePair_t**, uint32_t,
                                                  ISteamMatchmakingServerListResponse*) override {
        return nullptr;
    }
    HServerListRequest RequestHistoryServerList(AppId_t, MatchMakingKeyValuePair_t**, uint32_t,
                                                ISteamMatchmakingServerListResponse*) override {
        return nullptr;
    }
    HServerListRequest RequestSpectatorServerList(AppId_t, MatchMakingKeyValuePair_t**, uint32_t,
                                                  ISteamMatchmakingServerListResponse*) override {
        return nullptr;
    }
    void ReleaseRequest(HServerListRequest) override {}
    gameserveritem_t* GetServerDetails(HServerListRequest, int) override { return nullptr; }
    void CancelQuery(HServerListRequest) override {}
    void RefreshQuery(HServerListRequest) override {}
    bool IsRefreshing(HServerListRequest) override { return false; }
    int GetServerCount(HServerListRequest) override { return 0; }
    void RefreshServer(HServerListRequest, int) override {}
    HServerQuery PingServer(uint32_t, uint16_t, ISteamMatchmakingServerListResponse*) override {
        return 0;
    }
    HServerQuery PlayerDetails(uint32_t, uint16_t, ISteamMatchmakingServerListResponse*) override {
        return 0;
    }
    HServerQuery ServerRules(uint32_t, uint16_t, ISteamMatchmakingServerListResponse*) override {
        return 0;
    }
    void CancelServerQuery(HServerQuery) override {}
};

class SteamNetworkingImpl final : public ISteamNetworking {
public:
    bool SendP2PPacket(CSteamID destination, const void* data, uint32_t size, int, int channel) override {
        if (!data || size == 0)
            return false;
        return Emu().net.Send(destination, static_cast<const uint8_t*>(data), size, channel);
    }
    bool IsP2PPacketAvailable(uint32_t* size, int channel) override {
        return Emu().net.HasPacket(channel, size);
    }
    bool ReadP2PPacket(void* buffer, uint32_t size, uint32_t* read, CSteamID* source, int channel) override {
        if (!buffer || size == 0)
            return false;
        std::vector<uint8_t> packet;
        CSteamID sender = 0;
        int packetChannel = channel;
        if (!Emu().net.ReadPacket(packet, &sender, &packetChannel))
            return false;
        const uint32_t copy = size < static_cast<uint32_t>(packet.size()) ? size
                                                                          : static_cast<uint32_t>(packet.size());
        std::memcpy(buffer, packet.data(), copy);
        if (read)
            *read = copy;
        if (source)
            *source = sender;
        return true;
    }
    bool AcceptP2PSessionWithUser(CSteamID) override { return true; }
    bool CloseP2PSessionWithUser(CSteamID) override { return true; }
    bool CloseP2PChannelWithUser(CSteamID, int) override { return true; }
    bool GetP2PSessionState(CSteamID peer, P2PSessionState_t* state) override {
        if (!state)
            return false;
        std::memset(state, 0, sizeof(P2PSessionState_t));
        state->m_bConnectionActive = Emu().net.IsConnected(peer) ? 1 : 0;
        state->m_eP2PSessionError = k_EP2PSessionErrorNone;
        return true;
    }
    bool AllowP2PPacketRelay(bool) override { return true; }
    SNetListenSocket_t CreateListenSocket(int, SteamIPAddress_t, uint16_t, bool) override { return 0; }
    SNetSocket_t CreateP2PConnectionSocket(CSteamID, int, int, bool) override { return 0; }
    SNetSocket_t CreateConnectionSocket(SteamIPAddress_t, uint16_t, int) override { return 0; }
    bool DestroySocket(SNetSocket_t, bool) override { return false; }
    bool DestroyListenSocket(SNetListenSocket_t, bool) override { return false; }
    bool SendDataOnSocket(SNetSocket_t, void*, uint32_t, bool) override { return false; }
    bool IsDataAvailableOnSocket(SNetSocket_t, uint32_t*) override { return false; }
    bool RetrieveDataFromSocket(SNetSocket_t, void*, uint32_t, uint32_t*) override { return false; }
    bool IsDataAvailable(SNetListenSocket_t, uint32_t*, bool) override { return false; }
    bool RetrieveData(SNetListenSocket_t, void*, uint32_t, uint32_t*) override { return false; }
    bool GetSocketInfo(SNetSocket_t, CSteamID*, int*, SteamIPAddress_t*, uint16_t*) override {
        return false;
    }
    bool GetListenSocketInfo(SNetListenSocket_t, SteamIPAddress_t*, uint16_t*) override { return false; }
    int GetSocketConnectionType(SNetSocket_t) override { return k_ESNetSocketConnectionTypeNotConnected; }
    int GetMaxPacketSize(SNetSocket_t) override { return 1200; }
};

} // namespace

ISteamMatchmaking* MatchmakingInstance() {
    static SteamMatchmakingImpl impl;
    return &impl;
}

ISteamMatchmakingServers* MatchmakingServersInstance() {
    static SteamMatchmakingServersImpl impl;
    return &impl;
}

ISteamNetworking* NetworkingInstance() {
    static SteamNetworkingImpl impl;
    return &impl;
}

} // namespace sfemu
