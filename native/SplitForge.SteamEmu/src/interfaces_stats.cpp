#include "impls.h"
#include "interfaces.h"
#include "state.h"

#include <cstring>
#include <ctime>

namespace sfemu {

namespace {

CGameID GameId() {
    return static_cast<CGameID>(Emu().config.appId);
}

class SteamUserStatsImpl final : public ISteamUserStats {
public:
    bool RequestCurrentStats() override {
        EmuState& emu = Emu();
        emu.LoadStats();
        UserStatsReceived_t result{};
        result.m_nGameID = GameId();
        result.m_eResult = k_EResultOK;
        result.m_steamIDUser = emu.config.steamId;
        emu.QueueCallback(k_iCallback_UserStatsReceived, &result, sizeof(result));
        return true;
    }
    bool GetStat(const char* name, int32_t* value) override {
        if (!name || !value)
            return false;
        *value = static_cast<int32_t>(Emu().GetStat(name));
        return true;
    }
    bool GetStat(const char* name, float* value) override {
        if (!name || !value)
            return false;
        *value = static_cast<float>(Emu().GetStat(name));
        return true;
    }
    bool SetStat(const char* name, int32_t value) override {
        if (!name)
            return false;
        Emu().SetStat(name, static_cast<double>(value));
        return true;
    }
    bool SetStat(const char* name, float value) override {
        if (!name)
            return false;
        Emu().SetStat(name, static_cast<double>(value));
        return true;
    }
    bool UpdateAvgRateStat(const char* name, float value, double sessionLength) override {
        if (!name)
            return false;
        EmuState& emu = Emu();
        const double current = emu.GetStat(name);
        const double sessions = emu.GetStat(std::string(name) + "_sessions") + 1.0;
        const double total = current * sessions + value * sessionLength;
        emu.SetStat(name, total / (sessions * sessionLength));
        emu.SetStat(std::string(name) + "_sessions", sessions);
        return true;
    }
    bool GetAchievement(const char* name, bool* achieved) override {
        if (!name || !achieved)
            return false;
        *achieved = Emu().GetAchievement(name);
        return true;
    }
    bool SetAchievement(const char* name) override {
        if (!name)
            return false;
        return Emu().SetAchievement(name);
    }
    bool ClearAchievement(const char* name) override {
        if (!name)
            return false;
        return Emu().ClearAchievement(name);
    }
    bool GetAchievementAndUnlockTime(const char* name, bool* achieved, uint32_t* unlockTime) override {
        if (!name || !achieved || !unlockTime)
            return false;
        EmuState& emu = Emu();
        *achieved = emu.GetAchievement(name);
        *unlockTime = emu.GetAchievementUnlockTime(name);
        return true;
    }
    bool StoreStats() override {
        EmuState& emu = Emu();
        emu.SaveStats();
        UserStatsStored_t result{};
        result.m_nGameID = GameId();
        result.m_eResult = k_EResultOK;
        emu.QueueCallback(k_iCallback_UserStatsStored, &result, sizeof(result));
        return true;
    }
    int GetAchievementIcon(const char*) override { return 0; }
    const char* GetAchievementDisplayAttribute(const char*, const char*) override { return ""; }
    bool IndicateAchievementProgress(const char*, uint32_t, uint32_t) override { return false; }
    uint32_t GetNumAchievements() override { return 0; }
    const char* GetAchievementName(uint32_t) override { return ""; }
    SteamAPICall_t RequestUserStats(CSteamID) override {
        EmuState& emu = Emu();
        UserStatsReceived_t result{};
        result.m_nGameID = GameId();
        result.m_eResult = k_EResultOK;
        result.m_steamIDUser = emu.config.steamId;
        return emu.QueueCallResult(k_iCallback_UserStatsReceived, &result, sizeof(result));
    }
    bool GetUserStat(CSteamID, const char* name, int32_t* value) override {
        if (!name || !value)
            return false;
        *value = static_cast<int32_t>(Emu().GetStat(name));
        return true;
    }
    bool GetUserStat(CSteamID, const char* name, float* value) override {
        if (!name || !value)
            return false;
        *value = static_cast<float>(Emu().GetStat(name));
        return true;
    }
    bool GetUserAchievement(CSteamID, const char* name, bool* achieved) override {
        if (!name || !achieved)
            return false;
        *achieved = Emu().GetAchievement(name);
        return true;
    }
    bool GetUserAchievementAndUnlockTime(CSteamID, const char* name, bool* achieved,
                                         uint32_t* unlockTime) override {
        if (!name || !achieved || !unlockTime)
            return false;
        EmuState& emu = Emu();
        *achieved = emu.GetAchievement(name);
        *unlockTime = emu.GetAchievementUnlockTime(name);
        return true;
    }
    bool ResetAllStats(bool achievementsToo) override {
        EmuState& emu = Emu();
        emu.stats.clear();
        if (achievementsToo)
            emu.achievements.clear();
        emu.SaveStats();
        return true;
    }
    SteamAPICall_t FindOrCreateLeaderboard(const char*, int, int) override {
        EmuState& emu = Emu();
        LeaderboardFindResult_t result{};
        result.m_hSteamLeaderboard = 1;
        result.m_bLeaderboardFound = 1;
        return emu.QueueCallResult(k_iCallback_LeaderboardFindResult, &result, sizeof(result));
    }
    SteamAPICall_t FindLeaderboard(const char*) override {
        return FindOrCreateLeaderboard(nullptr, 0, 0);
    }
    const char* GetLeaderboardName(SteamLeaderboard_t) override { return "Local"; }
    int GetLeaderboardEntryCount(SteamLeaderboard_t) override { return 0; }
    int GetLeaderboardSortMethod(SteamLeaderboard_t) override { return k_ELeaderboardSortMethodDescending; }
    int GetLeaderboardDisplayType(SteamLeaderboard_t) override { return k_ELeaderboardDisplayTypeNumeric; }
    SteamAPICall_t DownloadLeaderboardEntries(SteamLeaderboard_t board, int, int, int) override {
        EmuState& emu = Emu();
        LeaderboardScoresDownloaded_t result{};
        result.m_hSteamLeaderboard = board;
        result.m_hSteamLeaderboardEntries = 1;
        result.m_cEntryCount = 0;
        return emu.QueueCallResult(k_iCallback_LeaderboardScoresDownloaded, &result, sizeof(result));
    }
    SteamAPICall_t DownloadLeaderboardEntriesForUsers(SteamLeaderboard_t board, CSteamID*, int) override {
        return DownloadLeaderboardEntries(board, 0, 0, 0);
    }
    bool GetDownloadedLeaderboardEntry(SteamLeaderboardEntries_t, int, LeaderboardEntry_t*, int32_t*,
                                       int) override {
        return false;
    }
    SteamAPICall_t UploadLeaderboardScore(SteamLeaderboard_t board, int, int32_t score, const int32_t*,
                                          int) override {
        EmuState& emu = Emu();
        LeaderboardScoreUploaded_t result{};
        result.m_bSuccess = 1;
        result.m_hSteamLeaderboard = board;
        result.m_nScore = score;
        result.m_bScoreChanged = 1;
        return emu.QueueCallResult(k_iCallback_LeaderboardScoreUploaded, &result, sizeof(result));
    }
    SteamAPICall_t AttachLeaderboardUGC(SteamLeaderboard_t, UGCHandle_t) override { return 0; }
    SteamAPICall_t GetNumberOfCurrentPlayers() override {
        EmuState& emu = Emu();
        NumberOfCurrentPlayers_t result{};
        result.m_bSuccess = 1;
        result.m_cPlayers = 1;
        return emu.QueueCallResult(k_iCallback_NumberOfCurrentPlayers, &result, sizeof(result));
    }
    SteamAPICall_t RequestGlobalAchievementPercentages() override {
        EmuState& emu = Emu();
        GlobalAchievementPercentagesReady_t result{};
        result.m_nGameID = GameId();
        result.m_eResult = k_EResultOK;
        return emu.QueueCallResult(k_iCallback_GlobalAchievementPercentagesReady, &result, sizeof(result));
    }
    int GetMostAchievedAchievementInfo(char*, uint32_t, float*, bool*) override { return -1; }
    int GetNextMostAchievedAchievementInfo(int, char*, uint32_t, float*, bool*) override { return -1; }
    bool GetAchievementAchievedPercent(const char*, float*) override { return false; }
    SteamAPICall_t RequestGlobalStats(int) override {
        EmuState& emu = Emu();
        GlobalStatsReceived_t result{};
        result.m_nGameID = GameId();
        result.m_eResult = k_EResultOK;
        return emu.QueueCallResult(k_iCallback_GlobalStatsReceived, &result, sizeof(result));
    }
    bool GetGlobalStat(const char*, int64_t* value) override {
        if (value)
            *value = 0;
        return false;
    }
    bool GetGlobalStat(const char*, double* value) override {
        if (value)
            *value = 0;
        return false;
    }
    int32_t GetGlobalStatHistory(const char*, int64_t*, uint32_t) override { return 0; }
    int32_t GetGlobalStatHistory(const char*, double*, uint32_t) override { return 0; }
};

class SteamGameServerStatsImpl final : public ISteamGameServerStats {
public:
    SteamAPICall_t RequestUserStats(CSteamID steamId) override {
        EmuState& emu = Emu();
        UserStatsReceived_t result{};
        result.m_nGameID = GameId();
        result.m_eResult = k_EResultOK;
        result.m_steamIDUser = steamId;
        return emu.QueueCallResult(k_iCallback_UserStatsReceived, &result, sizeof(result));
    }
    bool GetUserStat(CSteamID, const char* name, int32_t* value) override {
        if (!name || !value)
            return false;
        *value = static_cast<int32_t>(Emu().GetStat(name));
        return true;
    }
    bool GetUserStat(CSteamID, const char* name, float* value) override {
        if (!name || !value)
            return false;
        *value = static_cast<float>(Emu().GetStat(name));
        return true;
    }
    bool GetUserAchievement(CSteamID, const char* name, bool* achieved) override {
        if (!name || !achieved)
            return false;
        *achieved = Emu().GetAchievement(name);
        return true;
    }
    bool SetUserStat(CSteamID, const char* name, int32_t value) override {
        if (!name)
            return false;
        Emu().SetStat(name, static_cast<double>(value));
        return true;
    }
    bool SetUserStat(CSteamID, const char* name, float value) override {
        if (!name)
            return false;
        Emu().SetStat(name, static_cast<double>(value));
        return true;
    }
    bool UpdateUserAvgRateStat(CSteamID, const char*, float, double) override { return false; }
    bool SetUserAchievement(CSteamID, const char* name) override {
        if (!name)
            return false;
        Emu().SetAchievement(name);
        return true;
    }
    bool ClearUserAchievement(CSteamID, const char* name) override {
        if (!name)
            return false;
        Emu().ClearAchievement(name);
        return true;
    }
    SteamAPICall_t StoreUserStats(CSteamID) override {
        EmuState& emu = Emu();
        emu.SaveStats();
        return 0;
    }
};

} // namespace

ISteamUserStats* UserStatsInstance() {
    static SteamUserStatsImpl impl;
    return &impl;
}

ISteamGameServerStats* GameServerStatsInstance() {
    static SteamGameServerStatsImpl impl;
    return &impl;
}

} // namespace sfemu
