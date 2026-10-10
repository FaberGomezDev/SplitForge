#include "impls.h"
#include "interfaces.h"
#include "state.h"

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
    SteamAPICall_t RequestLobbyList() override {
        EmuState& emu = Emu();
        LobbyMatchList_t result{};
        result.m_nLobbiesMatching = 0;
        return emu.QueueCallResult(k_iCallback_LobbyMatchList, &result, sizeof(result));
    }
    void AddRequestLobbyListStringFilter(const char*, const char*, int) override {}
    void AddRequestLobbyListNumericalFilter(const char*, int, int) override {}
    void AddRequestLobbyListNearValueFilter(const char*, int) override {}
    void AddRequestLobbyListFilterSlotsAvailable(int) override {}
    void AddRequestLobbyListDistanceFilter(int) override {}
    void AddRequestLobbyListResultCountFilter(int) override {}
    void AddRequestLobbyListCompatibleMembersFilter(CSteamID) override {}
    CSteamID GetLobbyByIndex(int) override { return k_steamIDNil; }
    SteamAPICall_t CreateLobby(int type, int maxMembers) override {
        EmuState& emu = Emu();
        Lobby& lobby = emu.GetOrCreateLobby(emu.NewLobbyId());
        lobby.type = type;
        lobby.memberLimit = maxMembers > 0 ? maxMembers : 4;
        LobbyCreated_t created{};
        created.m_eResult = k_EResultOK;
        created.m_ulSteamIDLobby = lobby.id;
        const SteamAPICall_t call = emu.QueueCallResult(k_iCallback_LobbyCreated, &created, sizeof(created));
        LobbyEnter_t entered{};
        entered.m_ulSteamIDLobby = lobby.id;
        entered.m_rgfChatPermissions = 0xFFFFFFFF;
        entered.m_bLocked = false;
        entered.m_EChatRoomEnterResponse = k_EChatRoomEnterResponseSuccess;
        emu.QueueCallback(k_iCallback_LobbyEnter, &entered, sizeof(entered));
        return call;
    }
    SteamAPICall_t JoinLobby(CSteamID lobbyId) override {
        EmuState& emu = Emu();
        Lobby& lobby = emu.GetOrCreateLobby(lobbyId);
        LobbyEnter_t entered{};
        entered.m_ulSteamIDLobby = lobby.id;
        entered.m_rgfChatPermissions = 0xFFFFFFFF;
        entered.m_bLocked = false;
        entered.m_EChatRoomEnterResponse = k_EChatRoomEnterResponseSuccess;
        return emu.QueueCallResult(k_iCallback_LobbyEnter, &entered, sizeof(entered));
    }
    void LeaveLobby(CSteamID lobbyId) override {
        Emu().lobbies.erase(lobbyId);
    }
    bool InviteUserToLobby(CSteamID, CSteamID) override { return false; }
    int GetNumLobbyMembers(CSteamID lobbyId) override {
        EmuState& emu = Emu();
        if (!emu.lobbies.count(lobbyId))
            return 0;
        return static_cast<int>(emu.lobbies[lobbyId].members.size());
    }
    CSteamID GetLobbyMemberByIndex(CSteamID lobbyId, int index) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        if (it == emu.lobbies.end() || index < 0 || index >= static_cast<int>(it->second.members.size()))
            return k_steamIDNil;
        return it->second.members[static_cast<size_t>(index)];
    }
    const char* GetLobbyData(CSteamID lobbyId, const char* key) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        if (it == emu.lobbies.end() || !key)
            return "";
        const auto data = it->second.data.find(key);
        return data == it->second.data.end() ? "" : data->second.c_str();
    }
    bool SetLobbyData(CSteamID lobbyId, const char* key, const char* value) override {
        if (!key)
            return false;
        EmuState& emu = Emu();
        if (!emu.lobbies.count(lobbyId))
            return false;
        Lobby& lobby = emu.lobbies[lobbyId];
        lobby.data[key] = value ? value : "";
        LobbyDataUpdate_t update{};
        update.m_ulSteamIDLobby = lobbyId;
        update.m_ulSteamIDMember = k_steamIDNil;
        update.m_bSuccess = 1;
        emu.QueueCallback(k_iCallback_LobbyDataUpdate, &update, sizeof(update));
        return true;
    }
    int GetLobbyDataCount(CSteamID lobbyId) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        return it == emu.lobbies.end() ? 0 : static_cast<int>(it->second.data.size());
    }
    bool GetLobbyDataByIndex(CSteamID lobbyId, int index, char* key, int keySize, char* value,
                             int valueSize) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        if (it == emu.lobbies.end() || index < 0)
            return false;
        int i = 0;
        for (const auto& [k, v] : it->second.data) {
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
        return it->second.data.erase(key) > 0;
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
    bool RequestLobbyData(CSteamID) override { return true; }
    void SetLobbyGameServer(CSteamID lobbyId, uint32_t ip, uint16_t port, CSteamID serverId) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        if (it == emu.lobbies.end())
            return;
        it->second.gameServerIp = ip;
        it->second.gameServerPort = port;
        it->second.gameServerSteamId = serverId;
    }
    bool GetLobbyGameServer(CSteamID lobbyId, uint32_t* ip, uint16_t* port, CSteamID* serverId) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        if (it == emu.lobbies.end())
            return false;
        if (ip)
            *ip = it->second.gameServerIp;
        if (port)
            *port = it->second.gameServerPort;
        if (serverId)
            *serverId = it->second.gameServerSteamId;
        return it->second.gameServerIp != 0 || it->second.gameServerSteamId != 0;
    }
    bool SetLobbyMemberLimit(CSteamID lobbyId, int limit) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        if (it == emu.lobbies.end())
            return false;
        it->second.memberLimit = limit;
        return true;
    }
    int GetLobbyMemberLimit(CSteamID lobbyId) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        return it == emu.lobbies.end() ? 0 : it->second.memberLimit;
    }
    bool SetLobbyType(CSteamID lobbyId, int type) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        if (it == emu.lobbies.end())
            return false;
        it->second.type = type;
        return true;
    }
    bool SetLobbyJoinable(CSteamID, bool) override { return true; }
    CSteamID GetLobbyOwner(CSteamID lobbyId) override {
        EmuState& emu = Emu();
        const auto it = emu.lobbies.find(lobbyId);
        return it == emu.lobbies.end() ? k_steamIDNil : it->second.owner;
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
    bool IsDataAvailableOnSocket(SNetSocket_t, uint32_t*, bool) override { return false; }
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
