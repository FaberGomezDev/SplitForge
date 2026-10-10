#include "impls.h"
#include "interfaces.h"
#include "state.h"

#include <windows.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>

namespace sfemu {

namespace {

std::string RemotePath(const char* name) {
    if (!name || !SafeRelativeName(name))
        return {};
    return JoinPath(Emu().remoteDir, name);
}

class SteamRemoteStorageImpl final : public ISteamRemoteStorage {
public:
    bool FileWrite(const char* name, const void* data, int32_t size) override {
        const std::string path = RemotePath(name);
        if (path.empty() || !data || size < 0)
            return false;
        std::ofstream file(Utf8ToWide(path), std::ios::binary | std::ios::trunc);
        if (!file)
            return false;
        file.write(static_cast<const char*>(data), size);
        return file.good();
    }
    int32_t FileRead(const char* name, void* buffer, int32_t size) override {
        const std::string path = RemotePath(name);
        if (path.empty() || !buffer || size < 0)
            return 0;
        std::ifstream file(Utf8ToWide(path), std::ios::binary);
        if (!file)
            return 0;
        file.seekg(0, std::ios::end);
        const std::streamoff total = file.tellg();
        if (total <= 0)
            return 0;
        file.seekg(0, std::ios::beg);
        const std::streamsize want = total < size ? total : size;
        file.read(static_cast<char*>(buffer), want);
        return static_cast<int32_t>(file.gcount());
    }
    SteamAPICall_t FileWriteAsync(const char* name, const void* data, uint32_t size) override {
        const bool ok = FileWrite(name, data, static_cast<int32_t>(size));
        EmuState& emu = Emu();
        RemoteStorageFileWriteAsyncComplete_t result{};
        result.m_eResult = ok ? k_EResultOK : k_EResultIOFailure;
        return emu.QueueCallResult(k_iCallback_RemoteStorageFileWriteAsyncComplete, &result,
                                   sizeof(result));
    }
    SteamAPICall_t FileReadAsync(const char* name, uint32_t offset, uint32_t size) override {
        const std::string path = RemotePath(name);
        EmuState& emu = Emu();
        const SteamAPICall_t call = emu.NewApiCall();
        RemoteStorageFileReadAsyncComplete_t result{};
        std::memset(&result, 0, sizeof(result));
        result.m_nOffset = offset;
        std::string data;
        if (path.empty() || !ReadWholeFile(path, data)) {
            result.m_eResult = k_EResultFileNotFound;
            emu.QueueCallResultAt(call, k_iCallback_RemoteStorageFileReadAsyncComplete, &result,
                                  sizeof(result));
            return call;
        }
        if (offset < data.size()) {
            const size_t available = data.size() - offset;
            result.m_cubRead = static_cast<uint32_t>(available < size ? available : size);
            emu.asyncReadBuffers[call] = data.substr(offset, result.m_cubRead);
        }
        result.m_eResult = k_EResultOK;
        emu.QueueCallResultAt(call, k_iCallback_RemoteStorageFileReadAsyncComplete, &result,
                              sizeof(result));
        return call;
    }
    bool FileReadAsyncComplete(SteamAPICall_t call, void* buffer, uint32_t size) override {
        EmuState& emu = Emu();
        const auto it = emu.asyncReadBuffers.find(call);
        if (it == emu.asyncReadBuffers.end() || !buffer)
            return false;
        const uint32_t copy = size < static_cast<uint32_t>(it->second.size())
                                  ? size
                                  : static_cast<uint32_t>(it->second.size());
        std::memcpy(buffer, it->second.data(), copy);
        emu.asyncReadBuffers.erase(it);
        return true;
    }
    bool FileForget(const char* name) override {
        const std::string path = RemotePath(name);
        return !path.empty() && DeleteFileW(Utf8ToWide(path).c_str()) != 0;
    }
    bool FileDelete(const char* name) override {
        const std::string path = RemotePath(name);
        return !path.empty() && DeleteFileW(Utf8ToWide(path).c_str()) != 0;
    }
    SteamAPICall_t FileShare(const char* name) override {
        EmuState& emu = Emu();
        RemoteStorageFileShareResult_t result{};
        result.m_eResult = k_EResultOK;
        result.m_hFile = 1;
        if (name)
            std::strncpy(result.m_rgchFilename, name, sizeof(result.m_rgchFilename) - 1);
        return emu.QueueCallResult(k_iCallback_RemoteStorageFileShareResult, &result, sizeof(result));
    }
    bool SetSyncPlatforms(const char* name, uint32_t platforms) override {
        if (!name)
            return false;
        Emu().syncPlatforms[name] = platforms;
        return true;
    }
    UGCFileWriteStreamHandle_t FileWriteStreamOpen(const char* name) override {
        const std::string path = RemotePath(name);
        if (path.empty())
            return k_UGCHandleInvalid;
        EmuState& emu = Emu();
        const UGCFileWriteStreamHandle_t handle = emu.streamCounter++;
        WriteStream stream;
        stream.path = path;
        stream.file = std::make_shared<std::ofstream>(Utf8ToWide(path),
                                                      std::ios::binary | std::ios::trunc);
        if (!stream.file || !*stream.file)
            return k_UGCHandleInvalid;
        emu.writeStreams[handle] = std::move(stream);
        return handle;
    }
    bool FileWriteStreamWriteChunk(UGCFileWriteStreamHandle_t handle, const void* data,
                                   int32_t size) override {
        EmuState& emu = Emu();
        const auto it = emu.writeStreams.find(handle);
        if (it == emu.writeStreams.end() || !data || size < 0 || !it->second.file)
            return false;
        it->second.file->write(static_cast<const char*>(data), size);
        return it->second.file->good();
    }
    bool FileWriteStreamClose(UGCFileWriteStreamHandle_t handle) override {
        EmuState& emu = Emu();
        const auto it = emu.writeStreams.find(handle);
        if (it == emu.writeStreams.end() || !it->second.file)
            return false;
        it->second.file->flush();
        const bool ok = it->second.file->good();
        emu.writeStreams.erase(it);
        return ok;
    }
    bool FileWriteStreamCancel(UGCFileWriteStreamHandle_t handle) override {
        EmuState& emu = Emu();
        const auto it = emu.writeStreams.find(handle);
        if (it == emu.writeStreams.end())
            return false;
        const std::string path = it->second.path;
        emu.writeStreams.erase(it);
        if (!path.empty())
            DeleteFileW(Utf8ToWide(path).c_str());
        return true;
    }
    bool FileExists(const char* name) override {
        const std::string path = RemotePath(name);
        if (path.empty())
            return false;
        const DWORD attributes = GetFileAttributesW(Utf8ToWide(path).c_str());
        return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }
    bool FilePersisted(const char* name) override { return FileExists(name); }
    int32_t GetFileSize(const char* name) override {
        const std::string path = RemotePath(name);
        if (path.empty())
            return 0;
        std::ifstream file(Utf8ToWide(path), std::ios::binary | std::ios::ate);
        if (!file)
            return 0;
        return static_cast<int32_t>(file.tellg());
    }
    int64_t GetFileTimestamp(const char* name) override {
        const std::string path = RemotePath(name);
        if (path.empty())
            return 0;
        WIN32_FILE_ATTRIBUTE_DATA data{};
        if (!GetFileAttributesExW(Utf8ToWide(path).c_str(), GetFileExInfoStandard, &data))
            return 0;
        return static_cast<int64_t>((static_cast<uint64_t>(data.ftLastWriteTime.dwHighDateTime) << 32) |
                                    data.ftLastWriteTime.dwLowDateTime);
    }
    uint32_t GetSyncPlatforms(const char* name) override {
        EmuState& emu = Emu();
        const auto it = emu.syncPlatforms.find(name ? name : "");
        return it == emu.syncPlatforms.end() ? k_ERemoteStoragePlatformAll : it->second;
    }
    int32_t GetFileCount() override {
        return static_cast<int32_t>(ListRemoteFiles().size());
    }
    bool GetFileNameAndSize(int index, char* name, int32_t* size, uint32_t* timestamp) override {
        const auto files = ListRemoteFiles();
        if (index < 0 || index >= static_cast<int>(files.size()))
            return false;
        const auto& item = files[static_cast<size_t>(index)];
        if (name)
            std::strncpy(name, item.name.c_str(), 259);
        if (size)
            *size = item.size;
        if (timestamp)
            *timestamp = item.timestamp;
        return true;
    }
    bool GetQuota(uint64_t* total, uint64_t* available) override {
        EmuState& emu = Emu();
        if (total)
            *total = emu.cloudTotal;
        if (available)
            *available = emu.cloudTotal - emu.cloudUsed;
        return true;
    }
    bool IsCloudEnabledForAccount() override { return Emu().cloudEnabled; }
    bool IsCloudEnabledForApp() override { return Emu().cloudEnabled; }
    void SetCloudEnabledForApp(bool enabled) override { Emu().cloudEnabled = enabled; }
    SteamAPICall_t UGCDownload(UGCHandle_t, uint32_t) override { return 0; }
    SteamAPICall_t UGCDownloadToLocation(UGCHandle_t, const char*, uint32_t) override { return 0; }
    bool GetUGCDownloadProgress(UGCHandle_t, int32_t*, int32_t*) override { return false; }
    bool GetUGCDetails(UGCHandle_t, AppId_t*, char**, int32_t*, CSteamID*) override { return false; }
    int32_t UGCRead(UGCHandle_t, void*, int32_t, uint32_t, int) override { return 0; }
    int32_t GetCachedUGCCount() override { return 0; }
    UGCHandle_t GetCachedUGCHandle(int32_t) override { return k_UGCHandleInvalid; }
    SteamAPICall_t PublishWorkshopFile(const char*, const char*, AppId_t, const char*, const char*, int,
                                       SteamParamStringArray_t*, int) override {
        return 0;
    }
    PublishedFileUpdateHandle_t CreatePublishedFileUpdateRequest(PublishedFileId_t) override { return 0; }
    bool UpdatePublishedFileFile(PublishedFileUpdateHandle_t, const char*) override { return false; }
    bool UpdatePublishedFilePreviewFile(PublishedFileUpdateHandle_t, const char*) override { return false; }
    bool UpdatePublishedFileTitle(PublishedFileUpdateHandle_t, const char*) override { return false; }
    bool UpdatePublishedFileDescription(PublishedFileUpdateHandle_t, const char*) override { return false; }
    bool UpdatePublishedFileVisibility(PublishedFileUpdateHandle_t, int) override { return false; }
    bool UpdatePublishedFileTags(PublishedFileUpdateHandle_t, SteamParamStringArray_t*) override { return false; }
    SteamAPICall_t CommitPublishedFileUpdate(PublishedFileUpdateHandle_t) override { return 0; }
    SteamAPICall_t GetPublishedFileDetails(PublishedFileId_t, uint32_t) override { return 0; }
    SteamAPICall_t DeletePublishedFile(PublishedFileId_t) override { return 0; }
    SteamAPICall_t EnumerateUserPublishedFiles(uint32_t) override { return 0; }
    SteamAPICall_t SubscribePublishedFile(PublishedFileId_t) override { return 0; }
    SteamAPICall_t EnumerateUserSubscribedFiles(uint32_t) override { return 0; }
    SteamAPICall_t UnsubscribePublishedFile(PublishedFileId_t) override { return 0; }
    bool UpdatePublishedFileSetChangeDescription(PublishedFileUpdateHandle_t, const char*) override {
        return false;
    }
    SteamAPICall_t GetPublishedItemVoteDetails(PublishedFileId_t) override { return 0; }
    SteamAPICall_t UpdateUserPublishedItemVote(PublishedFileId_t, bool) override { return 0; }
    SteamAPICall_t GetUserPublishedItemVoteDetails(PublishedFileId_t) override { return 0; }
    SteamAPICall_t EnumerateUserSharedWorkshopFiles(CSteamID, uint32_t, SteamParamStringArray_t*,
                                                    SteamParamStringArray_t*) override {
        return 0;
    }
    SteamAPICall_t PublishVideo(int, const char*, const char*, const char*, const char*, AppId_t,
                                const char*, const char*, int, SteamParamStringArray_t*) override {
        return 0;
    }
    SteamAPICall_t SetUserPublishedFileAction(PublishedFileId_t, int) override { return 0; }
    SteamAPICall_t EnumeratePublishedFilesByUserAction(int, uint32_t) override { return 0; }
    SteamAPICall_t EnumeratePublishedWorkshopFiles(int, uint32_t, uint32_t, uint32_t,
                                                   SteamParamStringArray_t*, SteamParamStringArray_t*) override {
        return 0;
    }
    SteamAPICall_t UGCDownloadToLocationEx(UGCHandle_t, const char*, uint32_t, const char*) override {
        return 0;
    }
    int32_t GetLocalFileChangeCount() override { return 0; }
    int32_t BeginFileWriteBatch() override { return 0; }
    bool EndFileWriteBatch() override { return true; }

private:
    struct RemoteFile {
        std::string name;
        int32_t size = 0;
        uint32_t timestamp = 0;
    };
    static std::vector<RemoteFile> ListRemoteFiles() {
        std::vector<RemoteFile> files;
        WIN32_FIND_DATAW data{};
        const std::string pattern = JoinPath(Emu().remoteDir, "*");
        HANDLE handle = FindFirstFileW(Utf8ToWide(pattern).c_str(), &data);
        if (handle == INVALID_HANDLE_VALUE)
            return files;
        do {
            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;
            RemoteFile file;
            file.name = WideToUtf8(data.cFileName);
            file.size = static_cast<int32_t>(
                (static_cast<uint64_t>(data.nFileSizeHigh) << 32) | data.nFileSizeLow);
            file.timestamp = static_cast<uint32_t>(
                ((static_cast<uint64_t>(data.ftLastWriteTime.dwHighDateTime) << 32) |
                 data.ftLastWriteTime.dwLowDateTime) /
                10000000);
            files.push_back(std::move(file));
        } while (FindNextFileW(handle, &data) != 0);
        FindClose(handle);
        return files;
    }
};

class SteamUGCImpl final : public ISteamUGC {
public:
    UGCQueryHandle_t CreateQueryUserUGCRequest(AccountID_t, int, int, int, AppId_t, AppId_t,
                                               uint32_t) override {
        return 0;
    }
    UGCQueryHandle_t CreateQueryAllUGCRequest(int, int, AppId_t, AppId_t, uint32_t) override { return 0; }
    UGCQueryHandle_t CreateQueryAllUGCRequest(int, int, AppId_t, AppId_t, AppId_t, uint32_t) override {
        return 0;
    }
    UGCQueryHandle_t CreateQueryUGCDetailsRequest(PublishedFileId_t*, uint32_t) override { return 0; }
    SteamAPICall_t SendQueryUGCRequest(UGCQueryHandle_t) override { return 0; }
    bool GetQueryUGCResult(UGCQueryHandle_t, uint32_t, SteamUGCDetails_t*) override { return false; }
    bool GetQueryUGCPreviewURL(UGCQueryHandle_t, uint32_t, char*, uint32_t) override { return false; }
    bool GetQueryUGCMetadataURL(UGCQueryHandle_t, uint32_t, char*, uint32_t) override { return false; }
    bool GetQueryUGCChildren(UGCQueryHandle_t, uint32_t, PublishedFileId_t*, uint32_t) override {
        return false;
    }
    bool GetQueryUGCStatistic(UGCQueryHandle_t, uint32_t, int, uint64_t*) override { return false; }
    uint32_t GetQueryUGCNumAdditionalPreviews(UGCQueryHandle_t, uint32_t) override { return 0; }
    bool GetQueryUGCAdditionalPreview(UGCQueryHandle_t, uint32_t, uint32_t, char*, uint32_t, char*,
                                      uint32_t, int*) override {
        return false;
    }
    uint32_t GetQueryUGCNumKeyValueTags(UGCQueryHandle_t, uint32_t) override { return 0; }
    bool GetQueryUGCKeyValueTag(UGCQueryHandle_t, uint32_t, uint32_t, char*, uint32_t, char*,
                                uint32_t) override {
        return false;
    }
    uint32_t GetQueryUGCContentDescriptors(UGCQueryHandle_t, uint32_t, uint32_t*, uint32_t) override {
        return 0;
    }
    bool ReleaseQueryUGCRequest(UGCQueryHandle_t) override { return true; }
    bool AddRequiredTag(UGCQueryHandle_t, const char*) override { return false; }
    bool AddRequiredTagGroup(UGCQueryHandle_t, const SteamParamStringArray_t*) override { return false; }
    bool AddExcludedTag(UGCQueryHandle_t, const char*) override { return false; }
    bool SetReturnOnlyIDs(UGCQueryHandle_t, bool) override { return false; }
    bool SetReturnKeyValueTags(UGCQueryHandle_t, bool) override { return false; }
    bool SetReturnLongDescription(UGCQueryHandle_t, bool) override { return false; }
    bool SetReturnMetadata(UGCQueryHandle_t, bool) override { return false; }
    bool SetReturnChildren(UGCQueryHandle_t, bool) override { return false; }
    bool SetReturnAdditionalPreviews(UGCQueryHandle_t, bool) override { return false; }
    bool SetReturnTotalOnly(UGCQueryHandle_t, bool) override { return false; }
    bool SetReturnPlaytimeStats(UGCQueryHandle_t, uint32_t) override { return false; }
    bool SetLanguage(UGCQueryHandle_t, const char*) override { return false; }
    bool SetAllowCachedResponse(UGCQueryHandle_t, uint32_t) override { return false; }
    bool SetCloudFileNameFilter(UGCQueryHandle_t, const char*) override { return false; }
    bool SetMatchAnyTag(UGCQueryHandle_t, bool) override { return false; }
    bool SetSearchText(UGCQueryHandle_t, const char*) override { return false; }
    bool SetRankedByTrendDays(UGCQueryHandle_t, uint32_t) override { return false; }
    bool SetTimeCreatedFileId(UGCQueryHandle_t, PublishedFileId_t) override { return false; }
    bool SetTimeUpdatedFileId(UGCQueryHandle_t, PublishedFileId_t) override { return false; }
    bool AddRequiredKeyValueTag(UGCQueryHandle_t, const char*, const char*) override { return false; }
    SteamAPICall_t RequestUGCDetails(PublishedFileId_t*, uint32_t, uint32_t) override { return 0; }
    SteamAPICall_t CreateItem(AppId_t, int) override { return 0; }
    UGCUpdateHandle_t StartItemUpdate(AppId_t, PublishedFileId_t) override { return 0; }
    bool SetItemTitle(UGCUpdateHandle_t, const char*) override { return false; }
    bool SetItemDescription(UGCUpdateHandle_t, const char*) override { return false; }
    bool SetItemUpdateLanguage(UGCUpdateHandle_t, const char*) override { return false; }
    bool SetItemMetadata(UGCUpdateHandle_t, const char*) override { return false; }
    bool SetItemVisibility(UGCUpdateHandle_t, int) override { return false; }
    bool SetItemTags(UGCUpdateHandle_t, const SteamParamStringArray_t*) override { return false; }
    bool SetItemContent(UGCUpdateHandle_t, const char*) override { return false; }
    bool SetItemPreview(UGCUpdateHandle_t, const char*) override { return false; }
    bool SetAllowLegacyUpload(UGCUpdateHandle_t, bool) override { return false; }
    bool RemoveAllItemKeyValueTags(UGCUpdateHandle_t) override { return false; }
    bool RemoveItemKeyValueTags(UGCUpdateHandle_t, const char*) override { return false; }
    bool AddItemKeyValueTag(UGCUpdateHandle_t, const char*, const char*) override { return false; }
    bool AddItemPreviewFile(UGCUpdateHandle_t, const char*, int) override { return false; }
    bool AddItemPreviewVideo(UGCUpdateHandle_t, const char*) override { return false; }
    bool UpdateItemPreviewFile(UGCUpdateHandle_t, uint32_t, const char*) override { return false; }
    bool UpdateItemPreviewVideo(UGCUpdateHandle_t, uint32_t, const char*) override { return false; }
    bool RemoveItemPreview(UGCUpdateHandle_t, uint32_t) override { return false; }
    SteamAPICall_t SubmitItemUpdate(UGCUpdateHandle_t, const char*) override { return 0; }
    int GetItemUpdateProgress(UGCUpdateHandle_t, uint64_t*, uint64_t*) override {
        return k_EItemUpdateStatusInvalid;
    }
    SteamAPICall_t SetUserItemVote(PublishedFileId_t, bool) override { return 0; }
    SteamAPICall_t GetUserItemVote(PublishedFileId_t) override { return 0; }
    SteamAPICall_t AddItemToFavorites(AppId_t, PublishedFileId_t) override { return 0; }
    SteamAPICall_t RemoveItemFromFavorites(AppId_t, PublishedFileId_t) override { return 0; }
    SteamAPICall_t SubscribeItem(PublishedFileId_t) override { return 0; }
    SteamAPICall_t UnsubscribeItem(PublishedFileId_t) override { return 0; }
    uint32_t GetNumSubscribedItems() override { return 0; }
    uint32_t GetSubscribedItems(PublishedFileId_t*, uint32_t) override { return 0; }
    uint32_t GetItemState(PublishedFileId_t) override { return 0; }
    bool GetItemInstallInfo(PublishedFileId_t, uint64_t*, char*, uint32_t, uint32_t*) override {
        return false;
    }
    bool GetItemDownloadInfo(PublishedFileId_t, uint64_t*, uint64_t*) override { return false; }
    bool DeleteItem(PublishedFileId_t) override { return false; }
    bool SetItemTaxonomyTags(UGCUpdateHandle_t, const SteamParamStringArray_t*) override { return false; }
    bool GetItemTaxonomyTags(UGCUpdateHandle_t, uint32_t, uint32_t, char*, uint32_t) override {
        return false;
    }
};

class SteamInventoryImpl final : public ISteamInventory {
public:
    EResult GetResultStatus(SteamInventoryResult_t) override { return k_EResultFail; }
    bool GetResultItems(SteamInventoryResult_t, SteamItemDetails_t*, uint32_t) override { return false; }
    bool GetResultItemProperty(SteamInventoryResult_t, uint32_t, const char*, char*, uint32_t*) override {
        return false;
    }
    uint32_t GetResultTimestamp(SteamInventoryResult_t) override { return 0; }
    bool CheckResultSteamID(SteamInventoryResult_t, CSteamID) override { return false; }
    void DestroyResult(SteamInventoryResult_t) override {}
    bool GetAllItems(SteamInventoryResult_t* result) override {
        if (result)
            *result = 1;
        return true;
    }
    bool GetItemsByID(SteamInventoryResult_t*, const SteamItemInstanceID_t*, uint32_t) override {
        return false;
    }
    bool SerializeResult(SteamInventoryResult_t, void*, uint32_t*) override { return false; }
    bool DeserializeResult(SteamInventoryResult_t*, const void*, uint32_t, bool) override { return false; }
    bool GenerateItems(SteamInventoryResult_t*, const SteamItemDef_t*, const uint32_t*, uint32_t) override {
        return false;
    }
    bool GrantPromoItems(SteamInventoryResult_t* result) override {
        if (result)
            *result = 1;
        return true;
    }
    bool AddPromoItem(SteamInventoryResult_t*, SteamItemDef_t) override { return false; }
    bool AddPromoItems(SteamInventoryResult_t*, const SteamItemDef_t*, uint32_t) override { return false; }
    bool ConsumeItem(SteamInventoryResult_t*, SteamItemInstanceID_t, uint32_t) override { return false; }
    bool ExchangeItems(SteamInventoryResult_t*, const SteamItemDef_t*, const uint32_t*, uint32_t,
                       const SteamItemInstanceID_t*, const uint32_t*, uint32_t) override {
        return false;
    }
    bool TransferItemQuantity(SteamInventoryResult_t*, SteamItemInstanceID_t, uint32_t,
                              SteamItemInstanceID_t) override {
        return false;
    }
    void SendItemDropHeartbeat() override {}
    bool TriggerItemDrop(SteamInventoryResult_t*, SteamItemDef_t) override { return false; }
    bool TradeItems(SteamInventoryResult_t*, CSteamID, const SteamItemInstanceID_t*, const uint32_t*,
                    uint32_t, const SteamItemInstanceID_t*, const uint32_t*, uint32_t) override {
        return false;
    }
    bool LoadItemDefinitions() override { return true; }
    bool GetItemDefinitionIDs(SteamItemDef_t*, uint32_t* count) override {
        if (count)
            *count = 0;
        return true;
    }
    bool GetItemDefinitionProperty(SteamItemDef_t, const char*, char*, uint32_t*) override {
        return false;
    }
    SteamAPICall_t RequestEligiblePromoItemDefinitionsIDs(CSteamID) override { return 0; }
    bool GetEligiblePromoItemDefinitionIDs(CSteamID, SteamItemDef_t*, uint32_t* count) override {
        if (count)
            *count = 0;
        return false;
    }
    SteamAPICall_t StartPurchase(const SteamItemDef_t*, const uint32_t*, uint32_t) override {
        return 0;
    }
    SteamAPICall_t RequestPrices() override { return 0; }
    uint32_t GetNumItemsWithPrices() override { return 0; }
    bool GetItemsWithPrices(SteamItemDef_t*, uint64_t*, uint64_t*, uint32_t) override { return false; }
    bool GetItemPrice(SteamItemDef_t, uint64_t*, uint64_t*) override { return false; }
    SteamInventoryUpdateHandle_t StartUpdateProperties() override { return 0; }
    bool RemoveProperty(SteamInventoryUpdateHandle_t, SteamItemInstanceID_t, const char*) override {
        return false;
    }
    bool SetProperty(SteamInventoryUpdateHandle_t, SteamItemInstanceID_t, const char*, bool) override {
        return false;
    }
    bool SetProperty(SteamInventoryUpdateHandle_t, SteamItemInstanceID_t, const char*, int64_t) override {
        return false;
    }
    bool SetProperty(SteamInventoryUpdateHandle_t, SteamItemInstanceID_t, const char*, float) override {
        return false;
    }
    bool SetProperty(SteamInventoryUpdateHandle_t, SteamItemInstanceID_t, const char*,
                     const char*) override {
        return false;
    }
    bool SubmitUpdateProperties(SteamInventoryUpdateHandle_t, SteamInventoryResult_t*) override {
        return false;
    }
    bool InspectItem(SteamInventoryResult_t*, const char*) override { return false; }
};

class SteamHTTPImpl final : public ISteamHTTP {
public:
    HTTPRequestHandle CreateHTTPRequest(int, const char*) override { return k_uInvalidHTTPRequestHandle; }
    bool SetHTTPRequestContextValue(HTTPRequestHandle, uint64_t) override { return false; }
    bool SetHTTPRequestNetworkActivityTimeout(HTTPRequestHandle, uint32_t) override { return false; }
    bool SetHTTPRequestHeaderValue(HTTPRequestHandle, const char*, const char*) override { return false; }
    bool SetHTTPRequestGetOrPostParameter(HTTPRequestHandle, const char*, const char*) override {
        return false;
    }
    bool SendHTTPRequest(HTTPRequestHandle, SteamAPICall_t* call) override {
        if (call)
            *call = k_uAPICallInvalid;
        return false;
    }
    bool SendHTTPRequestAndStreamResponse(HTTPRequestHandle, SteamAPICall_t* call) override {
        if (call)
            *call = k_uAPICallInvalid;
        return false;
    }
    bool DeferHTTPRequest(HTTPRequestHandle) override { return false; }
    bool PrioritizeHTTPRequest(HTTPRequestHandle) override { return false; }
    bool GetHTTPResponseHeaderSize(HTTPRequestHandle, const char*, uint32_t*) override { return false; }
    bool GetHTTPResponseHeaderValue(HTTPRequestHandle, const char*, uint8_t*, uint32_t) override {
        return false;
    }
    bool GetHTTPResponseBodySize(HTTPRequestHandle, uint32_t*) override { return false; }
    bool GetHTTPResponseBodyData(HTTPRequestHandle, uint8_t*, uint32_t) override { return false; }
    bool GetHTTPStreamingResponseBodyData(HTTPRequestHandle, uint32_t, uint8_t*, uint32_t) override {
        return false;
    }
    bool ReleaseHTTPRequest(HTTPRequestHandle) override { return false; }
    bool GetHTTPDownloadProgressPct(HTTPRequestHandle, float*) override { return false; }
    bool SetHTTPRequestRawPostBody(HTTPRequestHandle, const char*, uint8_t*, uint32_t) override {
        return false;
    }
    HTTPCookieContainerHandle CreateCookieContainer(bool) override {
        return k_uInvalidHTTPCookieContainerHandle;
    }
    bool ReleaseCookieContainer(HTTPCookieContainerHandle) override { return false; }
    bool SetCookie(HTTPCookieContainerHandle, const char*, const char*, const char*) override {
        return false;
    }
    bool SetHTTPRequestCookieContainer(HTTPRequestHandle, HTTPCookieContainerHandle) override {
        return false;
    }
    bool SetHTTPRequestUserAgentInfo(HTTPRequestHandle, const char*) override { return false; }
    bool SetHTTPRequestRequiresVerifiedCertificate(HTTPRequestHandle, bool) override { return false; }
    bool SetHTTPRequestAbsoluteTimeoutMS(HTTPRequestHandle, uint32_t) override { return false; }
    bool GetHTTPRequestWasTimedOut(HTTPRequestHandle, bool*) override { return false; }
};

class SteamScreenshotsImpl final : public ISteamScreenshots {
public:
    ScreenshotHandle WriteScreenshot(void*, uint32_t, int, int) override { return 1; }
    ScreenshotHandle AddScreenshotToLibrary(const char*, const char*, int, int) override { return 1; }
    void TriggerScreenshot() override {}
    void HookScreenshots(bool) override {}
    bool SetLocation(ScreenshotHandle, const char*) override { return true; }
    bool TagUser(ScreenshotHandle, CSteamID) override { return true; }
    bool TagPublishedFile(ScreenshotHandle, PublishedFileId_t) override { return true; }
    bool IsScreenshotsHooked() override { return false; }
    ScreenshotHandle AddVRScreenshotToLibrary(int, const char*, const char*) override { return 1; }
};

} // namespace

ISteamRemoteStorage* RemoteStorageInstance() {
    static SteamRemoteStorageImpl impl;
    return &impl;
}

ISteamUGC* UgcInstance() {
    static SteamUGCImpl impl;
    return &impl;
}

ISteamInventory* InventoryInstance() {
    static SteamInventoryImpl impl;
    return &impl;
}

ISteamHTTP* HttpInstance() {
    static SteamHTTPImpl impl;
    return &impl;
}

ISteamScreenshots* ScreenshotsInstance() {
    static SteamScreenshotsImpl impl;
    return &impl;
}

} // namespace sfemu
