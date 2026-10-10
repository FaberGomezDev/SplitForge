#include "net.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <atomic>
#include <cstring>

namespace sfemu {

namespace {

constexpr uint64_t kMagicPresence = 0x003130554D454653ull; // "SFEMU1\0\0"
constexpr uint64_t kMagicData = 0x003230554D454653ull;     // "SFEMU2\0\0"
constexpr uint64_t kPeerTtlMs = 10000;

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
        if (now >= next) {
            AnnounceNow();
            next = now + interval;
        }
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
