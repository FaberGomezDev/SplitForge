#include "state.h"

#include "minijson.h"

#include <windows.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <random>

namespace sfemu {

namespace {

bool MakeDir(const std::string& path) {
    return CreateDirectoryW(Utf8ToWide(path).c_str(), nullptr) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
}

bool ParseSteamId(const std::string& text, uint64_t& out) {
    if (text.empty() || text.size() > 20)
        return false;
    uint64_t value = 0;
    for (const char c : text) {
        if (c < '0' || c > '9')
            return false;
        if (value > (UINT64_MAX - 9) / 10)
            return false;
        value = value * 10 + static_cast<uint64_t>(c - '0');
    }
    if (value == 0)
        return false;
    out = value;
    return true;
}

} // namespace

std::wstring Utf8ToWide(const std::string& text) {
    if (text.empty())
        return {};
    const int size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (size <= 0)
        return {};
    std::wstring out(static_cast<size_t>(size) - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, out.data(), size);
    return out;
}

std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty())
        return {};
    const int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0)
        return {};
    std::string out(static_cast<size_t>(size) - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, out.data(), size, nullptr, nullptr);
    return out;
}

EmuState& Emu() {
    static EmuState state;
    return state;
}

const Config& Cfg() {
    return Emu().config;
}

void EmuState::Log(const char* format, ...) {
    wchar_t local[MAX_PATH] = {};
    if (GetEnvironmentVariableW(L"LOCALAPPDATA", local, MAX_PATH) == 0)
        return;
    char path[MAX_PATH * 2] = {};
    const std::string localUtf8 = WideToUtf8(local);
    std::snprintf(path, sizeof(path), "%s\\SplitForge\\steamemu.log", localUtf8.c_str());
    MakeDir(localUtf8 + "\\SplitForge");
    FILE* f = nullptr;
    if (fopen_s(&f, path, "a") != 0 || !f)
        return;
    va_list args;
    va_start(args, format);
    const time_t now = time(nullptr);
    std::fprintf(f, "[%lld] ", static_cast<long long>(now));
    std::vfprintf(f, format, args);
    std::fprintf(f, "\n");
    va_end(args);
    std::fclose(f);
}

std::string JoinPath(const std::string& a, const std::string& b) {
    if (a.empty())
        return b;
    if (b.empty())
        return a;
    if (a.back() == '\\' || a.back() == '/')
        return a + b;
    return a + "\\" + b;
}

bool SafeRelativeName(const std::string& name) {
    if (name.empty() || name.size() > 259)
        return false;
    if (name.front() == '\\' || name.front() == '/' || name.find(':') != std::string::npos)
        return false;
    if (name == "." || name == ".." || name.find("..\\") != std::string::npos || name.find("../") != std::string::npos)
        return false;
    return true;
}

bool ReadWholeFile(const std::string& path, std::string& out) {
    std::ifstream file(Utf8ToWide(path), std::ios::binary);
    if (!file)
        return false;
    out.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    return true;
}

bool WriteWholeFile(const std::string& path, const std::string& data) {
    std::ofstream file(Utf8ToWide(path), std::ios::binary | std::ios::trunc);
    if (!file)
        return false;
    file.write(data.data(), static_cast<std::streamsize>(data.size()));
    return file.good();
}

std::string DefaultStorageDir(const Config& config) {
    wchar_t local[MAX_PATH] = {};
    if (GetEnvironmentVariableW(L"LOCALAPPDATA", local, MAX_PATH) == 0)
        return {};
    char dir[MAX_PATH * 2] = {};
    std::snprintf(dir, sizeof(dir), "%s\\SplitForge\\steamemu\\%u", WideToUtf8(local).c_str(), config.appId);
    return dir;
}

bool LoadConfig(Config& out, const std::string& path) {
    std::string text;
    if (!ReadWholeFile(path, text))
        return false;
    const Json root = Json::parse(text);
    if (root.kind() != Json::Kind::Object)
        return false;
    if (const Json* v = root.find("app_id")) {
        if (v->kind() == Json::Kind::Number)
            out.appId = static_cast<AppId_t>(v->num());
    }
    if (const Json* v = root.find("steam_id")) {
        uint64_t id = 0;
        if (v->kind() == Json::Kind::String && ParseSteamId(v->str(), id))
            out.steamId = id;
    }
    if (const Json* v = root.find("persona_name")) {
        if (v->kind() == Json::Kind::String && !v->str().empty())
            out.personaName = v->str();
    }
    if (const Json* v = root.find("language")) {
        if (v->kind() == Json::Kind::String && !v->str().empty())
            out.language = v->str();
    }
    if (const Json* v = root.find("ip_country")) {
        if (v->kind() == Json::Kind::String && !v->str().empty())
            out.ipCountry = v->str();
    }
    if (const Json* v = root.find("storage_dir")) {
        if (v->kind() == Json::Kind::String && !v->str().empty())
            out.storageDir = v->str();
    }
    if (const Json* v = root.find("dlc")) {
        if (v->kind() == Json::Kind::Array) {
            out.dlc.clear();
            for (const auto& item : v->array()) {
                if (item.kind() == Json::Kind::Number)
                    out.dlc.push_back(static_cast<AppId_t>(item.num()));
            }
        }
    }
    if (const Json* v = root.find("net_port")) {
        if (v->kind() == Json::Kind::Number)
            out.netPort = static_cast<uint16_t>(v->num());
    }
    if (const Json* v = root.find("net_discovery_seconds")) {
        if (v->kind() == Json::Kind::Number)
            out.discoverySeconds = static_cast<uint32_t>(v->num());
    }
    if (const Json* v = root.find("net_multicast_address")) {
        if (v->kind() == Json::Kind::String && !v->str().empty())
            out.multicastAddress = v->str();
    }
    out.configPath = path;
    return true;
}

bool EmuState::EnsureDirs() {
    if (config.storageDir.empty())
        config.storageDir = DefaultStorageDir(config);
    userDataDir = JoinPath(config.storageDir, "userdata");
    remoteDir = JoinPath(config.storageDir, "remote");
    if (!MakeDir(config.storageDir) || !MakeDir(userDataDir) || !MakeDir(remoteDir))
        return false;
    return true;
}

SteamAPICall_t EmuState::NewApiCall() {
    const SteamAPICall_t call = nextApiCall++;
    if (call == 0)
        return nextApiCall++;
    return call;
}

void EmuState::QueueCallback(int iCallback, const void* data, size_t size) {
    CallbackEvent event;
    event.iCallback = iCallback;
    event.data.assign(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + size);
    events.push_back(std::move(event));
}

SteamAPICall_t EmuState::QueueCallResult(int iCallback, const void* data, size_t size) {
    const SteamAPICall_t call = NewApiCall();
    QueueCallResultAt(call, iCallback, data, size);
    return call;
}

void EmuState::QueueCallResultAt(SteamAPICall_t call, int iCallback, const void* data, size_t size) {
    CallbackEvent event;
    event.apiCall = call;
    event.iCallback = iCallback;
    event.data.assign(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + size);
    // Copia antes de mover el evento a la cola (tras el move, event.data queda vacío).
    PendingResult result;
    result.iCallback = iCallback;
    result.data = event.data;
    apiResults[call] = std::move(result);
    events.push_back(std::move(event));
}

bool EmuState::FinishApiResult(SteamAPICall_t call, int iCallback, const void* data, size_t size, bool fail) {
    if (!fail)
        return true;
    auto it = apiResults.find(call);
    if (it == apiResults.end())
        return false;
    PendingResult result;
    result.iCallback = iCallback;
    result.data.assign(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + size);
    it->second = std::move(result);
    return true;
}

struct CallbackBaseCompat {
    void** vtbl;
    int m_nCallbackFlags;
    int m_iCallback;
};

using RunParamFn = void(__cdecl*)(void* self, void* pvParam);
using RunResultFn = void(__cdecl*)(void* self, void* pvParam, bool bIOFailure, uint64_t hSteamAPICall);

namespace {
void InvokeRun(void* object, CallbackEvent& event, bool isCallResult, bool failed) {
    auto* base = static_cast<CallbackBaseCompat*>(object);
    if (!base || !base->vtbl)
        return;
    if (!(base->m_nCallbackFlags & 0x01))
        return;
    if (isCallResult) {
        auto run = reinterpret_cast<RunResultFn>(base->vtbl[1]);
        run(object, event.data.data(), failed, event.apiCall);
    } else {
        if (base->m_iCallback != event.iCallback)
            return;
        auto run = reinterpret_cast<RunParamFn>(base->vtbl[0]);
        run(object, event.data.data());
    }
    event.consumed = true;
}
} // namespace

void EmuState::PruneConsumedEvents() {
    while (!events.empty() && events.front().consumed)
        events.pop_front();
}

void EmuState::DispatchRunCallbacks() {
    PruneConsumedEvents();
    // Por índice: los manejadores pueden encolar eventos nuevos mientras se recorre la cola.
    for (size_t i = 0; i < events.size(); ++i) {
        CallbackEvent& event = events[i];
        if (event.consumed)
            continue;
        if (event.apiCall != 0) {
            for (const auto& reg : callResults) {
                if (reg.apiCall == event.apiCall && reg.object) {
                    InvokeRun(reg.object, event, true, false);
                    if (event.consumed)
                        break;
                }
            }
            if (!event.consumed) {
                for (const auto& reg : globalCallResults) {
                    if (reg.apiCall == event.apiCall && reg.object) {
                        InvokeRun(reg.object, event, true, false);
                        if (event.consumed)
                            break;
                    }
                }
            }
        } else {
            for (const auto& reg : callbacks) {
                if (reg.iCallback == event.iCallback && reg.object) {
                    InvokeRun(reg.object, event, false, false);
                    if (event.consumed)
                        break;
                }
            }
            if (!event.consumed) {
                for (const auto& reg : globalCallbacks) {
                    if (reg.iCallback == event.iCallback && reg.object) {
                        InvokeRun(reg.object, event, false, false);
                        if (event.consumed)
                            break;
                    }
                }
            }
        }
    }
}

bool EmuState::PopManual(CallbackMsg_t* out) {
    if (!lastManualValid)
        PruneConsumedEvents();
    for (auto& event : events) {
        if (event.consumed)
            continue;
        lastManual.m_hSteamUser = 1;
        if (event.apiCall != 0) {
            // Call result: se anuncia con SteamAPICallCompleted_t; el juego pide los datos después
            // con SteamAPI_ManualDispatch_GetAPICallResult (apiResults los conserva hasta entonces).
            lastCompleted.m_hAsyncCall = event.apiCall;
            lastCompleted.m_iCallback = event.iCallback;
            lastCompleted.m_cubParam = static_cast<uint32_t>(event.data.size());
            lastManual.m_iCallback = k_iCallback_SteamAPICallCompleted;
            lastManual.m_pubParam = reinterpret_cast<uint8_t*>(&lastCompleted);
            lastManual.m_cubParam = static_cast<int>(sizeof(lastCompleted));
        } else {
            lastManual.m_iCallback = event.iCallback;
            lastManual.m_pubParam = event.data.data();
            lastManual.m_cubParam = static_cast<int>(event.data.size());
        }
        lastManualValid = true;
        event.consumed = true;
        if (out)
            *out = lastManual;
        return true;
    }
    return false;
}

void EmuState::FreeLastManual() {
    lastManualValid = false;
}

bool EmuState::GetApiResult(SteamAPICall_t call, void* out, size_t size, int expected, bool* failed) {
    const auto it = apiResults.find(call);
    if (it == apiResults.end())
        return false;
    if (failed)
        *failed = false;
    if (expected != it->second.iCallback)
        return false;
    const size_t copy = size < it->second.data.size() ? size : it->second.data.size();
    if (copy && out)
        std::memcpy(out, it->second.data.data(), copy);
    apiResults.erase(it);
    return true;
}

Lobby& EmuState::GetOrCreateLobby(CSteamID id) {
    auto it = lobbies.find(id);
    if (it == lobbies.end()) {
        Lobby lobby;
        lobby.id = id;
        lobby.owner = config.steamId;
        lobby.type = k_ELobbyTypePrivate;
        lobby.members.push_back(config.steamId);
        lobby.memberData[config.steamId] = {};
        it = lobbies.emplace(id, std::move(lobby)).first;
    }
    return it->second;
}

CSteamID EmuState::NewLobbyId() {
    return 0x1860000000000000ull + nextLobbyId++;
}

double EmuState::GetStat(const std::string& name) const {
    const auto it = stats.find(name);
    return it == stats.end() ? 0.0 : it->second;
}

void EmuState::SetStat(const std::string& name, double value) {
    stats[name] = value;
}

bool EmuState::GetAchievement(const std::string& name) const {
    const auto it = achievements.find(name);
    return it != achievements.end() && it->second.first;
}

bool EmuState::SetAchievement(const std::string& name) {
    if (GetAchievement(name))
        return false;
    achievements[name] = {true, static_cast<uint32_t>(time(nullptr))};
    return true;
}

bool EmuState::ClearAchievement(const std::string& name) {
    return achievements.erase(name) > 0;
}

uint32_t EmuState::GetAchievementUnlockTime(const std::string& name) const {
    const auto it = achievements.find(name);
    return it == achievements.end() ? 0 : it->second.second;
}

void EmuState::SaveStats() {
    Json root;
    Json statsJson;
    for (const auto& [name, value] : stats)
        statsJson.set(name, Json(value));
    root.set("stats", std::move(statsJson));
    Json achievementsJson;
    for (const auto& [name, value] : achievements) {
        Json item;
        item.set("unlocked", Json(value.first));
        item.set("time", Json(static_cast<double>(value.second)));
        achievementsJson.set(name, std::move(item));
    }
    root.set("achievements", std::move(achievementsJson));
    WriteWholeFile(JoinPath(config.storageDir, "stats.json"), root.dump());
}

void EmuState::LoadStats() {
    if (statsLoaded)
        return;
    statsLoaded = true;
    std::string text;
    if (!ReadWholeFile(JoinPath(config.storageDir, "stats.json"), text))
        return;
    const Json root = Json::parse(text);
    if (root.kind() != Json::Kind::Object)
        return;
    if (const Json* v = root.find("stats")) {
        if (v->kind() == Json::Kind::Object) {
            for (const auto& [name, item] : v->object()) {
                if (item.kind() == Json::Kind::Number)
                    stats[name] = item.num();
            }
        }
    }
    if (const Json* v = root.find("achievements")) {
        if (v->kind() == Json::Kind::Object) {
            for (const auto& [name, item] : v->object()) {
                if (item.kind() == Json::Kind::Object) {
                    bool unlocked = false;
                    uint32_t when = 0;
                    if (const Json* u = item.find("unlocked"))
                        unlocked = u->kind() == Json::Kind::Bool && u->boolean();
                    if (const Json* t = item.find("time"))
                        when = t->kind() == Json::Kind::Number ? static_cast<uint32_t>(t->num()) : 0;
                    achievements[name] = {unlocked, when};
                }
            }
        }
    }
}

void MakeTicket(uint8_t* out, size_t size) {
    static std::mt19937_64 rng(static_cast<uint64_t>(GetTickCount64()) ^
                               static_cast<uint64_t>(reinterpret_cast<uintptr_t>(out)));
    size_t filled = 0;
    while (filled < size) {
        const uint64_t value = rng();
        const size_t chunk = (size - filled) < sizeof(value) ? (size - filled) : sizeof(value);
        std::memcpy(out + filled, &value, chunk);
        filled += chunk;
    }
}

} // namespace sfemu
