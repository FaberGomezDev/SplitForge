#pragma once

// Transporte P2P local: un socket UDP por instancia unido a un grupo multicast.
// La presencia anuncia (steamId, appId) y los paquetes P2P viajan por el grupo,
// así dos instancias del mismo PC se descubren sin servidor intermedio.
// Alcance: misma máquina (y LAN si el grupo se enruta). No es cifrado ni fiable.

#include "steamemu_types.h"

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#include <atomic>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace sfemu {

struct Packet {
    std::vector<uint8_t> data;
    CSteamID sender = 0;
    int channel = 0;
};

class NetPeer {
public:
    ~NetPeer();
    bool Start(const std::string& groupAddress, uint16_t port, CSteamID steamId, AppId_t appId,
               uint32_t discoverySeconds);
    void Stop();

    bool Send(CSteamID destination, const uint8_t* data, size_t size, int channel);
    bool HasPacket(int channel, uint32_t* size);
    bool ReadPacket(std::vector<uint8_t>& out, CSteamID* sender, int* channel);
    bool IsConnected(CSteamID peer) const;
    void AnnounceNow();

private:
    void ReceiveLoop();
    void AnnounceLoop();

    SOCKET socket_ = INVALID_SOCKET;
    sockaddr_in group_{};
    std::thread receiveThread_;
    std::thread announceThread_;
    std::atomic<bool> running_{false};
    CSteamID self_ = 0;
    AppId_t appId_ = 0;
    uint32_t discoverySeconds_ = 2;
    uint16_t port_ = 0;

    mutable std::mutex mutex_;
    std::vector<Packet> packets_;
    std::map<CSteamID, uint64_t> peers_;
};

} // namespace sfemu
