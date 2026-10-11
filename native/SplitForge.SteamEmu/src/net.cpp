#include "net.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <atomic>
#include <cstring>

namespace sfemu {

namespace {

constexpr uint64_t kMagicPresence = 0x003130554D454653ull; // "SFEMU1\0\0"
constexpr uint64_t kMagicData = 0x003230554D454653ull;     // "SFEMU2\0\0"
constexpr uint64_t kMagicServer = 0x003330554D454653ull;   // "SFEMU03": partida alojada
constexpr uint64_t kMagicLobby = 0x003430554D454653ull;    // "SFEMU04": lobby y sus datos
constexpr uint64_t kPeerTtlMs = 10000;
constexpr size_t kServerFrame = 35;
constexpr size_t kLobbyHeader = 55;
constexpr size_t kMaxLobbyFrame = 60000;

uint16_t Read16(const uint8_t* p) {
    uint16_t v = 0;
    std::memcpy(&v, p, sizeof(v));
    return v;
}

void Write16(uint8_t* p, uint16_t v) { std::memcpy(p, &v, sizeof(v)); }

// Identifica este PC (hash del nombre del equipo): quien anuncia desde aquí se alcanza por 127.0.0.1.
uint64_t MachineToken() {
    char name[MAX_COMPUTERNAME_LENGTH + 1] = {};
    DWORD size = sizeof(name);
    if (!GetComputerNameA(name, &size))
        size = 0;
    uint64_t hash = 1469598103934665603ull;
    for (DWORD i = 0; i < size; ++i) {
        hash ^= static_cast<uint8_t>(name[i]);
        hash *= 1099511628211ull;
    }
    return hash;
}

uint64_t Read64(const uint8_t* p) {
    uint64_t v = 0;
    std::memcpy(&v, p, sizeof(v));
    return v;
}

uint32_t Read32(const uint8_t* p) {
    uint32_t v = 0;
    std::memcpy(&v, p, sizeof(v));
    return v;
}

void Write64(uint8_t* p, uint64_t v) { std::memcpy(p, &v, sizeof(v)); }
void Write32(uint8_t* p, uint32_t v) { std::memcpy(p, &v, sizeof(v)); }

} // namespace

NetPeer::~NetPeer() { Stop(); }

bool NetPeer::Start(const std::string& groupAddress, uint16_t port, CSteamID steamId, AppId_t appId,
                    uint32_t discoverySeconds) {
    Stop();
    self_ = steamId;
    appId_ = appId;
    port_ = port;
    discoverySeconds_ = discoverySeconds ? discoverySeconds : 2;
    machine_ = MachineToken();

    WSADATA data{};
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
        return false;

    socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_ == INVALID_SOCKET)
        return false;

    BOOL reuse = TRUE;
    setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    sockaddr_in bindAddr{};
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindAddr.sin_port = htons(port);
    if (bind(socket_, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr)) != 0)
        return false;

    ip_mreq mreq{};
    if (inet_pton(AF_INET, groupAddress.c_str(), &mreq.imr_multiaddr) != 1)
        return false;
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(socket_, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<const char*>(&mreq), sizeof(mreq)) != 0)
        return false;

    group_.sin_family = AF_INET;
    group_.sin_addr = mreq.imr_multiaddr;
    group_.sin_port = htons(port);

    u_long nonBlocking = 1;
    ioctlsocket(socket_, FIONBIO, &nonBlocking);

    running_.store(true);
    receiveThread_ = std::thread([this] { ReceiveLoop(); });
    announceThread_ = std::thread([this] { AnnounceLoop(); });
    AnnounceNow();
    return true;
}

void NetPeer::Stop() {
    if (!running_.exchange(false))
        return;
    if (socket_ != INVALID_SOCKET) {
        closesocket(socket_);
        socket_ = INVALID_SOCKET;
    }
    if (receiveThread_.joinable())
        receiveThread_.join();
    if (announceThread_.joinable())
        announceThread_.join();
    WSACleanup();
    std::lock_guard<std::mutex> lock(mutex_);
    packets_.clear();
    peers_.clear();
    servers_.clear();
    lobbies_.clear();
    localLobbies_.clear();
    serverActive_ = false;
}

void NetPeer::SetLocalServer(uint16_t port, bool active) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (serverActive_ == active && serverPort_ == port)
        return;
    if (active)
        serverSession_ = static_cast<uint32_t>(GetTickCount64()) | 1u; // Nunca 0: es «sin partida».
    serverActive_ = active;
    serverPort_ = port;
    dirty_.store(true);
}

std::vector<RemoteServer> NetPeer::Servers() const {
    const uint64_t now = GetTickCount64();
    std::vector<RemoteServer> result;
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [owner, server] : servers_) {
        if (now - server.lastSeen < kPeerTtlMs)
            result.push_back(server);
    }
    return result;
}

void NetPeer::SetLocalLobby(const RemoteLobby& lobby) {
    std::lock_guard<std::mutex> lock(mutex_);
    localLobbies_[lobby.id] = lobby;
    dirty_.store(true);
}

void NetPeer::RemoveLocalLobby(CSteamID id) {
    RemoteLobby gone;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto it = localLobbies_.find(id);
        if (it == localLobbies_.end())
            return;
        gone = std::move(it->second);
        localLobbies_.erase(it);
    }
    gone.data.clear();
    SendLobby(gone, false); // Los demás lo retiran ya, sin esperar a que caduque.
}

std::vector<RemoteLobby> NetPeer::Lobbies() const {
    const uint64_t now = GetTickCount64();
    std::vector<RemoteLobby> result;
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [id, lobby] : lobbies_) {
        if (now - lobby.lastSeen < kPeerTtlMs)
            result.push_back(lobby);
    }
    return result;
}

void NetPeer::SendLobby(const RemoteLobby& lobby, bool active) {
    if (!running_.load() || socket_ == INVALID_SOCKET)
        return;
    std::vector<uint8_t> frame(kLobbyHeader);
    Write64(frame.data(), kMagicLobby);
    Write64(frame.data() + 8, self_);
    Write32(frame.data() + 16, appId_);
    Write64(frame.data() + 20, lobby.id);
    Write32(frame.data() + 28, static_cast<uint32_t>(lobby.type));
    Write32(frame.data() + 32, static_cast<uint32_t>(lobby.memberLimit));
    Write32(frame.data() + 36, lobby.gameServerIp);
    Write16(frame.data() + 40, lobby.gameServerPort);
    Write64(frame.data() + 42, lobby.gameServerSteamId);
    frame[50] = active ? 1 : 0;
    uint32_t count = 0;
    for (const auto& [key, value] : lobby.data) {
        if (key.size() > 0xFFFF || value.size() > 0xFFFF || frame.size() + 4 + key.size() + value.size() > kMaxLobbyFrame)
            continue; // No cabe en un datagrama: ese dato no viaja.
        const size_t at = frame.size();
        frame.resize(at + 4 + key.size() + value.size());
        Write16(frame.data() + at, static_cast<uint16_t>(key.size()));
        std::memcpy(frame.data() + at + 2, key.data(), key.size());
        Write16(frame.data() + at + 2 + key.size(), static_cast<uint16_t>(value.size()));
        std::memcpy(frame.data() + at + 4 + key.size(), value.data(), value.size());
        ++count;
    }
    Write32(frame.data() + 51, count);
    sendto(socket_, reinterpret_cast<const char*>(frame.data()), static_cast<int>(frame.size()), 0,
           reinterpret_cast<const sockaddr*>(&group_), sizeof(group_));
}

// Anuncia la partida alojada (o que se cerró) y los lobbies visibles de este proceso.
void NetPeer::AnnounceDiscovery() {
    if (!running_.load() || socket_ == INVALID_SOCKET)
        return;
    std::vector<RemoteLobby> lobbies;
    uint16_t port = 0;
    uint32_t session = 0;
    bool active = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        port = serverPort_;
        session = serverSession_;
        active = serverActive_;
        for (const auto& [id, lobby] : localLobbies_)
            lobbies.push_back(lobby);
    }
    if (active || session != 0) {
        uint8_t frame[kServerFrame] = {};
        Write64(frame, kMagicServer);
        Write64(frame + 8, self_);
        Write32(frame + 16, appId_);
        Write32(frame + 20, session);
        Write16(frame + 24, port);
        frame[26] = active ? 1 : 0;
        Write64(frame + 27, machine_);
        sendto(socket_, reinterpret_cast<const char*>(frame), sizeof(frame), 0,
               reinterpret_cast<const sockaddr*>(&group_), sizeof(group_));
    }
    for (const RemoteLobby& lobby : lobbies)
        SendLobby(lobby, true);
}

bool NetPeer::Send(CSteamID destination, const uint8_t* data, size_t size, int channel) {
    if (!running_.load() || socket_ == INVALID_SOCKET)
        return false;
    std::vector<uint8_t> frame;
    frame.resize(8 + 8 + 8 + 1 + 4 + size);
    Write64(frame.data(), kMagicData);
    Write64(frame.data() + 8, destination);
    Write64(frame.data() + 16, self_);
    frame[24] = static_cast<uint8_t>(channel);
    Write32(frame.data() + 25, static_cast<uint32_t>(size));
    if (size)
        std::memcpy(frame.data() + 29, data, size);
    const int sent = sendto(socket_, reinterpret_cast<const char*>(frame.data()),
                            static_cast<int>(frame.size()), 0,
                            reinterpret_cast<const sockaddr*>(&group_), sizeof(group_));
    return sent == static_cast<int>(frame.size());
}

bool NetPeer::HasPacket(int channel, uint32_t* size) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& packet : packets_) {
        if (channel >= 0 && packet.channel != channel)
            continue;
        if (size)
            *size = static_cast<uint32_t>(packet.data.size());
        return true;
    }
    return false;
}

bool NetPeer::ReadPacket(std::vector<uint8_t>& out, CSteamID* sender, int* channel) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = packets_.begin(); it != packets_.end(); ++it) {
        if (channel && it->channel != *channel)
            continue;
        out = std::move(it->data);
        if (sender)
            *sender = it->sender;
        if (channel)
            *channel = it->channel;
        packets_.erase(it);
        return true;
    }
    return false;
}

bool NetPeer::IsConnected(CSteamID peer) const {
    if (peer == self_)
        return true;
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = peers_.find(peer);
    if (it == peers_.end())
        return false;
    return (GetTickCount64() - it->second) < kPeerTtlMs;
}

void NetPeer::AnnounceNow() {
    if (!running_.load() || socket_ == INVALID_SOCKET)
        return;
    uint8_t frame[24] = {};
    Write64(frame, kMagicPresence);
    Write64(frame + 8, self_);
    Write32(frame + 16, appId_);
    Write32(frame + 20, 0);
    sendto(socket_, reinterpret_cast<const char*>(frame), sizeof(frame), 0,
           reinterpret_cast<const sockaddr*>(&group_), sizeof(group_));
}

void NetPeer::AnnounceLoop() {
    const uint64_t interval = static_cast<uint64_t>(discoverySeconds_) * 1000;
    uint64_t next = GetTickCount64();
    while (running_.load()) {
        const uint64_t now = GetTickCount64();
        const bool due = now >= next;
        const bool dirty = dirty_.exchange(false);
        if (due) {
            AnnounceNow();
            next = now + interval;
        }
        if (due || dirty)
            AnnounceDiscovery();
        Sleep(100);
    }
}

void NetPeer::ReceiveLoop() {
    std::vector<uint8_t> buffer(65536);
    while (running_.load()) {
        sockaddr_in from{};
        int fromLen = sizeof(from);
        const int received = recvfrom(socket_, reinterpret_cast<char*>(buffer.data()),
                                      static_cast<int>(buffer.size()), 0,
                                      reinterpret_cast<sockaddr*>(&from), &fromLen);
        if (received < 0) {
            const int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK || error == WSAECONNRESET || error == WSAENETRESET) {
                Sleep(10);
                continue;
            }
            Sleep(50);
            continue;
        }
        if (received < 16)
            continue;
        const uint64_t magic = Read64(buffer.data());
        if (magic == kMagicPresence && received >= 20) {
            const CSteamID peer = Read64(buffer.data() + 8);
            if (peer == self_)
                continue;
            std::lock_guard<std::mutex> lock(mutex_);
            peers_[peer] = GetTickCount64();
            continue;
        }
        if (magic == kMagicServer && static_cast<size_t>(received) >= kServerFrame) {
            const CSteamID owner = Read64(buffer.data() + 8);
            if (owner == self_ || Read32(buffer.data() + 16) != appId_)
                continue;
            const uint32_t session = Read32(buffer.data() + 20);
            const uint64_t now = GetTickCount64();
            std::lock_guard<std::mutex> lock(mutex_);
            if (buffer[26] == 0) {
                servers_.erase(owner);
                continue;
            }
            RemoteServer& server = servers_[owner];
            if (server.session != session)
                server.firstSeen = now;
            server.owner = owner;
            server.session = session;
            server.port = Read16(buffer.data() + 24);
            char text[INET_ADDRSTRLEN] = {};
            server.address = Read64(buffer.data() + 27) == machine_ || !inet_ntop(AF_INET, &from.sin_addr, text, sizeof(text))
                                 ? "127.0.0.1" : text;
            server.lastSeen = now;
            continue;
        }
        if (magic == kMagicLobby && static_cast<size_t>(received) >= kLobbyHeader) {
            const CSteamID owner = Read64(buffer.data() + 8);
            if (owner == self_ || Read32(buffer.data() + 16) != appId_)
                continue;
            RemoteLobby lobby;
            lobby.owner = owner;
            lobby.id = Read64(buffer.data() + 20);
            lobby.type = static_cast<int>(Read32(buffer.data() + 28));
            lobby.memberLimit = static_cast<int>(Read32(buffer.data() + 32));
            lobby.gameServerIp = Read32(buffer.data() + 36);
            lobby.gameServerPort = Read16(buffer.data() + 40);
            lobby.gameServerSteamId = Read64(buffer.data() + 42);
            const bool active = buffer[50] != 0;
            const uint32_t count = Read32(buffer.data() + 51);
            size_t at = kLobbyHeader;
            const size_t end = static_cast<size_t>(received);
            bool valid = true;
            for (uint32_t i = 0; i < count && valid; ++i) {
                if (at + 2 > end) { valid = false; break; }
                const size_t keySize = Read16(buffer.data() + at);
                if (at + 4 + keySize > end) { valid = false; break; }
                const size_t valueSize = Read16(buffer.data() + at + 2 + keySize);
                if (at + 4 + keySize + valueSize > end) { valid = false; break; }
                lobby.data[std::string(reinterpret_cast<const char*>(buffer.data() + at + 2), keySize)] =
                    std::string(reinterpret_cast<const char*>(buffer.data() + at + 4 + keySize), valueSize);
                at += 4 + keySize + valueSize;
            }
            if (!valid)
                continue;
            lobby.lastSeen = GetTickCount64();
            std::lock_guard<std::mutex> lock(mutex_);
            if (active)
                lobbies_[lobby.id] = std::move(lobby);
            else
                lobbies_.erase(lobby.id);
            continue;
        }
        if (magic == kMagicData && received >= 29) {
            const CSteamID destination = Read64(buffer.data() + 8);
            const CSteamID source = Read64(buffer.data() + 16);
            if (destination != 0 && destination != self_)
                continue;
            const int channel = buffer[24];
            const uint32_t size = Read32(buffer.data() + 25);
            if (size > static_cast<uint32_t>(received) - 29 || size > 4096)
                continue;
            Packet packet;
            packet.sender = source;
            packet.channel = channel;
            packet.data.assign(buffer.data() + 29, buffer.data() + 29 + size);
            std::lock_guard<std::mutex> lock(mutex_);
            packets_.push_back(std::move(packet));
            peers_[source] = GetTickCount64();
        }
    }
}

} // namespace sfemu
