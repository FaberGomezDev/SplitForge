#pragma once

// Estado global del emulador: configuración, identidad, cola de callbacks, lobbies y
// persistencia (stats/achievements). Un solo proceso = una identidad Steam falsa.

#include "interfaces.h"
#include "net.h"

#include <cstdint>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace sfemu {

struct Config {
    AppId_t appId = 480;
    uint64_t steamId = MakeSteamID(1);
    std::string personaName = "SplitForgePlayer";
    std::string language = "english";
    std::string ipCountry = "ES";
    std::string storageDir;
    std::vector<AppId_t> dlc;
    uint16_t netPort = 27117;
    uint32_t discoverySeconds = 2;
    std::string multicastAddress = "239.255.69.69";
    std::string configPath;
};

struct CallbackEvent {
    SteamAPICall_t apiCall = 0;
    int iCallback = 0;
    std::vector<uint8_t> data;
    bool consumed = false;
};

struct PendingResult {
    int iCallback = 0;
    std::vector<uint8_t> data;
};

struct RegisteredCallback {
    void* object = nullptr;
    int iCallback = 0;
};

struct RegisteredCallResult {
    void* object = nullptr;
    SteamAPICall_t apiCall = 0;
};

struct ChatMessage {
    CSteamID sender = 0;
    int type = 0;
    std::vector<uint8_t> data;
    uint32_t id = 0;
};

struct Lobby {
    CSteamID id = 0;
    CSteamID owner = 0;
    int type = 0;
    int memberLimit = 0;
    std::map<std::string, std::string> data;
    std::map<CSteamID, std::map<std::string, std::string>> memberData;
    std::vector<CSteamID> members;
    std::vector<ChatMessage> chat;
    uint32_t nextChatId = 1;
    uint32_t gameServerIp = 0;
    uint16_t gameServerPort = 0;
    CSteamID gameServerSteamId = 0;
};

struct WriteStream {
    std::shared_ptr<std::ofstream> file;
    std::string path;
};

class EmuState {
public:
    std::mutex mutex;

    bool initialized = false;
    bool gameServerInitialized = false;
    Config config;
    std::string userDataDir;
    std::string remoteDir;
    uint64_t initTicks = 0;

    bool statsLoaded = false;
    std::map<std::string, double> stats;
    std::map<std::string, std::pair<bool, uint32_t>> achievements;
    uint64_t cloudTotal = 1024ull * 1024 * 1024 * 10;
    uint64_t cloudUsed = 0;
    bool cloudEnabled = true;
    std::map<std::string, uint32_t> syncPlatforms;
    std::map<std::string, std::string> richPresence;

    std::vector<CallbackEvent> events;
    std::vector<RegisteredCallback> callbacks;
    std::vector<RegisteredCallResult> callResults;
    std::vector<RegisteredCallback> globalCallbacks;
    std::vector<RegisteredCallResult> globalCallResults;
    std::map<SteamAPICall_t, PendingResult> apiResults;
    SteamAPICall_t nextApiCall = 1;
    uint32_t ipcCallCount = 0;
    void* warningHook = nullptr;
    CallbackMsg_t lastManual{};
    bool lastManualValid = false;
    UGCFileWriteStreamHandle_t streamCounter = 1;

    std::map<SteamAPICall_t, std::string> asyncReadBuffers;
    std::map<UGCFileWriteStreamHandle_t, WriteStream> writeStreams;

    std::map<CSteamID, Lobby> lobbies;
    uint64_t nextLobbyId = 1;

    std::map<uint32_t, uint64_t> authTickets;
    uint32_t nextAuthTicket = 1;
    uint32_t gameServerLastPacketCounter = 0;

    NetPeer net;

    void Log(const char* format, ...);
    bool EnsureDirs();
    SteamAPICall_t NewApiCall();
    void QueueCallback(int iCallback, const void* data, size_t size);
    SteamAPICall_t QueueCallResult(int iCallback, const void* data, size_t size);
    void QueueCallResultAt(SteamAPICall_t call, int iCallback, const void* data, size_t size);
    void DispatchRunCallbacks();
    bool PopManual(CallbackMsg_t* out);
    void FreeLastManual();
    bool GetApiResult(SteamAPICall_t call, void* out, size_t size, int expected, bool* failed);
    bool FinishApiResult(SteamAPICall_t call, int iCallback, const void* data, size_t size, bool fail);
    Lobby& GetOrCreateLobby(CSteamID id);
    CSteamID NewLobbyId();
    void SaveStats();
    void LoadStats();
    double GetStat(const std::string& name) const;
    void SetStat(const std::string& name, double value);
    bool SetAchievement(const std::string& name);
    bool ClearAchievement(const std::string& name);
    bool GetAchievement(const std::string& name) const;
    uint32_t GetAchievementUnlockTime(const std::string& name) const;
};

EmuState& Emu();
const Config& Cfg();

bool LoadConfig(Config& out, const std::string& path);
std::string DefaultStorageDir(const Config& config);
void MakeTicket(uint8_t* out, size_t size);
std::string JoinPath(const std::string& a, const std::string& b);
bool SafeRelativeName(const std::string& name);
bool ReadWholeFile(const std::string& path, std::string& out);
bool WriteWholeFile(const std::string& path, const std::string& data);
std::string WideToUtf8(const std::wstring& wide);
std::wstring Utf8ToWide(const std::string& text);

} // namespace sfemu
