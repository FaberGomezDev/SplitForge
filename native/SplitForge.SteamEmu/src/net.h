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

// Partida alojada por otra instancia (su servidor de juego está abierto).
struct RemoteServer {
    CSteamID owner = 0;
    uint32_t session = 0;   // Cambia cada vez que el anfitrión abre su servidor.
    uint16_t port = 0;
    std::string address;    // "127.0.0.1" si el anfitrión está en este mismo PC.
    uint64_t firstSeen = 0;
    uint64_t lastSeen = 0;
};

// Lobby anunciado por su dueño: lo que otra instancia necesita para listarlo y entrar.
struct RemoteLobby {
    CSteamID id = 0;
    CSteamID owner = 0;
    int type = 0;
    int memberLimit = 0;
    uint32_t gameServerIp = 0;
    uint16_t gameServerPort = 0;
    CSteamID gameServerSteamId = 0;
    std::map<std::string, std::string> data;
    uint64_t lastSeen = 0;
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

    // Descubrimiento: la partida y los lobbies de este proceso se anuncian junto con la presencia.
    void SetLocalServer(uint16_t port, bool active);
    std::vector<RemoteServer> Servers() const;
    void SetLocalLobby(const RemoteLobby& lobby);
    void RemoveLocalLobby(CSteamID id);
    std::vector<RemoteLobby> Lobbies() const;

private:
    void ReceiveLoop();
    void AnnounceLoop();
    void AnnounceDiscovery();
    void SendLobby(const RemoteLobby& lobby, bool active);

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

    uint64_t machine_ = 0;            // Para saber si quien anuncia está en este mismo PC.
    std::atomic<bool> dirty_{false};  // Hay cambios que anunciar sin esperar al siguiente intervalo.
    uint16_t serverPort_ = 0;
    bool serverActive_ = false;
    uint32_t serverSession_ = 0;
    std::map<CSteamID, RemoteLobby> localLobbies_;
    std::map<CSteamID, RemoteServer> servers_;
    std::map<CSteamID, RemoteLobby> lobbies_;
};

} // namespace sfemu
