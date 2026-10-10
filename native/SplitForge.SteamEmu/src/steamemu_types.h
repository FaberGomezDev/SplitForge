#pragma once

// Tipos ABI del cliente Steam (Steamworks SDK público). Solo lo necesario para que los
// juegos enlacen contra esta DLL y para que las estructuras de callbacks coincidan byte a byte.
// Nada de esto habla con Steam: es una reimplementación educativa del contrato.

#include <cstdint>

namespace sfemu {

using AppId_t = uint32_t;
using DepotId_t = uint32_t;
using AccountID_t = uint32_t;
using SteamID_t = uint64_t;              // CSteamID
using CSteamID = uint64_t;
using CGameID = uint64_t;
using HSteamUser = int32_t;
using HSteamPipe = int32_t;
using HAuthTicket = uint32_t;
using SteamAPICall_t = uint64_t;
using SteamLeaderboard_t = uint64_t;
using SteamLeaderboardEntries_t = uint64_t;
using UGCHandle_t = uint64_t;
using UGCFileWriteStreamHandle_t = uint64_t;
using PublishedFileId_t = uint64_t;
using PublishedFileUpdateHandle_t = uint64_t;
using UGCQueryHandle_t = uint64_t;
using UGCUpdateHandle_t = uint64_t;
using ScreenshotHandle = uint32_t;
using SNetSocket_t = uint32_t;
using SNetListenSocket_t = uint32_t;
using FriendsGroupID_t = int16_t;
using HTTPRequestHandle = uint32_t;
using HTTPCookieContainerHandle = uint32_t;
using SteamItemDef_t = int32_t;
using SteamItemInstanceID_t = uint64_t;
using SteamInventoryResult_t = int32_t;
using SteamInventoryUpdateHandle_t = uint64_t;
using ControllerHandle_t = uint64_t;
using ControllerActionSetHandle_t = uint64_t;
using ControllerDigitalActionHandle_t = uint64_t;
using ControllerAnalogActionHandle_t = uint64_t;
using HServerListRequest = void*;
using HServerQuery = int;

constexpr SteamAPICall_t k_uAPICallInvalid = 0;
constexpr SteamID_t k_steamIDNil = 0;
constexpr HAuthTicket k_HAuthTicketInvalid = 0;
constexpr uint32_t k_uInvalidHTTPRequestHandle = 0;
constexpr uint32_t k_uInvalidHTTPCookieContainerHandle = 0;
constexpr SteamLeaderboard_t k_SteamLeaderboardInvalid = 0;
constexpr SteamLeaderboardEntries_t k_SteamLeaderboardEntriesInvalid = 0;
constexpr UGCHandle_t k_UGCHandleInvalid = 0xFFFFFFFFFFFFFFFFull;
constexpr PublishedFileId_t k_PublishedFileIdInvalid = 0;
constexpr uint32_t k_unSteamUserDefaultInstance = 1;
constexpr int k_cchMaxRgchFriendsGroupName = 64;
constexpr int k_cchMaxRichPresenceKeys = 30;
constexpr int k_cchMaxRichPresenceKeyLength = 64;
constexpr int k_cchMaxRichPresenceValueLength = 256;
constexpr int k_cchMaxPersonaName = 128;

enum EResult : int32_t {
    k_EResultNone = 0, k_EResultOK = 1, k_EResultFail = 2, k_EResultNoConnection = 3,
    k_EResultInvalidPassword = 5, k_EResultLoggedInElsewhere = 6, k_EResultInvalidProtocolVer = 7,
    k_EResultInvalidParam = 8, k_EResultFileNotFound = 9, k_EResultBusy = 10, k_EResultInvalidState = 11,
    k_EResultInvalidName = 12, k_EResultInvalidEmail = 13, k_EResultDuplicateName = 14,
    k_EResultAccessDenied = 15, k_EResultTimeout = 16, k_EResultBanned = 17,
    k_EResultAccountNotFound = 18, k_EResultInvalidSteamID = 19, k_EResultServiceUnavailable = 20,
    k_EResultNotLoggedOn = 21, k_EResultPending = 22, k_EResultEncryptionFailure = 23,
    k_EResultInsufficientPrivilege = 24, k_EResultLimitExceeded = 25, k_EResultRevoked = 26,
    k_EResultExpired = 27, k_EResultAlreadyRedeemed = 28, k_EResultDuplicateRequest = 29,
    k_EResultAlreadyOwned = 30, k_EResultIPNotFound = 31, k_EResultPersistFailed = 32,
    k_EResultLockingFailed = 33, k_EResultLogonSessionReplaced = 34, k_EResultConnectFailed = 35,
    k_EResultHandshakeFailed = 36, k_EResultIOFailure = 37, k_EResultRemoteDisconnect = 38,
    k_EResultShoppingCartNotFound = 39, k_EResultBlocked = 40, k_EResultIgnored = 41,
    k_EResultNoMatch = 42, k_EResultAccountDisabled = 43, k_EResultServiceReadOnly = 44,
    k_EResultAccountNotFeatured = 45, k_EResultAdministratorOK = 46, k_EResultContentVersion = 47,
    k_EResultTryAnotherCM = 48, k_EResultPasswordRequiredToKickSession = 49,
    k_EResultAlreadyLoggedInElsewhere = 50, k_EResultSuspended = 51, k_EResultCancelled = 52,
    k_EResultDataCorruption = 53, k_EResultDiskFull = 54, k_EResultRemoteCallFailed = 55,
    k_EResultPasswordUnset = 56, k_EResultExternalAccountUnlinked = 57, k_EResultPSNTicketInvalid = 58,
    k_EResultExternalAccountAlreadyLinked = 59, k_EResultRemoteFileConflict = 60,
    k_EResultIllegalPassword = 61, k_EResultSameAsPreviousValue = 62, k_EResultAccountLogonDenied = 63,
    k_EResultCannotUseOldPassword = 64, k_EResultInvalidLoginAuthCode = 65,
    k_EResultAccountLogonDeniedNoMail = 66, k_EResultHardwareNotCapableOfIPT = 67,
    k_EResultIPTInitError = 68, k_EResultParentalControlRestricted = 69, k_EResultFacebookQueryError = 70,
    k_EResultExpiredLoginAuthCode = 71, k_EResultIPLoginRestrictionFailed = 72,
    k_EResultAccountLockedDown = 73, k_EResultAccountLogonDeniedVerifiedEmailRequired = 74,
    k_EResultNoMatchingURL = 75, k_EResultBadResponse = 76, k_EResultRequirePasswordReEntry = 77,
    k_EResultValueOutOfRange = 78, k_EResultUnexpectedError = 79, k_EResultDisabled = 80,
    k_EResultInvalidCEGSubmission = 81, k_EResultRestrictedDevice = 82, k_EResultRegionLocked = 83,
    k_EResultRateLimitExceeded = 84, k_EResultAccountLoginDeniedNeedTwoFactor = 85,
    k_EResultItemDeleted = 86, k_EResultAccountLoginDeniedThrottle = 87,
    k_EResultTwoFactorCodeMismatch = 88, k_EResultTwoFactorActivationCodeMismatch = 89,
    k_EResultAccountAssociatedToMultiplePartners = 90, k_EResultNotModified = 91,
    k_EResultNoMobileDevice = 92, k_EResultTimeNotSynced = 93, k_EResultSmsCodeFailed = 94,
    k_EResultAccountLimitExceeded = 95, k_EResultAccountActivityLimitExceeded = 96,
    k_EResultPhoneActivityLimitExceeded = 97, k_EResultRefundToWallet = 98,
    k_EResultEmailSendFailure = 99, k_EResultNotSettled = 100, k_EResultNeedCaptcha = 101,
    k_EResultGSLTDenied = 102, k_EResultGSOwnershipDenied = 103, k_EResultInvalidItemType = 104,
    k_EResultIPBanned = 105, k_EResultExpiredPaymentMethod = 106, k_EResultInvalidSteamID2 = 107
};

enum EUniverse { k_EUniverseInvalid = 0, k_EUniversePublic = 1, k_EUniverseBeta = 2,
                 k_EUniverseInternal = 3, k_EUniverseDev = 4 };
enum EAccountType { k_EAccountTypeInvalid = 0, k_EAccountTypeIndividual = 1, k_EAccountTypeMultiseat = 2,
                    k_EAccountTypeGameServer = 3, k_EAccountTypeAnonGameServer = 4,
                    k_EAccountTypePending = 5, k_EAccountTypeContentServer = 6, k_EAccountTypeClan = 7,
                    k_EAccountTypeChat = 8, k_EAccountTypeConsoleUser = 9, k_EAccountTypeAnonUser = 10 };

enum EPersonaState { k_EPersonaStateOffline = 0, k_EPersonaStateOnline = 1, k_EPersonaStateBusy = 2,
                     k_EPersonaStateAway = 3, k_EPersonaStateSnooze = 4, k_EPersonaStateLookingToTrade = 5,
                     k_EPersonaStateLookingToPlay = 6, k_EPersonaStateInvisible = 7, k_EPersonaStateMax = 8 };
enum EFriendRelationship { k_EFriendRelationshipNone = 0, k_EFriendRelationshipBlocked = 1,
                           k_EFriendRelationshipRequestRecipient = 2, k_EFriendRelationshipFriend = 3,
                           k_EFriendRelationshipRequestInitiator = 4, k_EFriendRelationshipIgnored = 5,
                           k_EFriendRelationshipIgnoredFriend = 6, k_EFriendRelationshipSuggested_DEPRECATED = 7,
                           k_EFriendRelationshipMax = 8 };
enum EFriendFlags { k_EFriendFlagNone = 0x00, k_EFriendFlagBlocked = 0x01, k_EFriendFlagFriendshipRequested = 0x02,
                    k_EFriendFlagImmediate = 0x04, k_EFriendFlagClanMember = 0x08, k_EFriendFlagOnGameServer = 0x10,
                    k_EFriendFlagRequestingFriendship = 0x80, k_EFriendFlagRequestingInfo = 0x100,
                    k_EFriendFlagIgnored = 0x200, k_EFriendFlagIgnoredFriend = 0x400,
                    k_EFriendFlagChatMember = 0x1000, k_EFriendFlagAll = 0xFFFF };
enum EChatEntryType { k_EChatEntryTypeInvalid = 0, k_EChatEntryTypeChatMsg = 1, k_EChatEntryTypeTyping = 2,
                      k_EChatEntryTypeInviteGame = 3, k_EChatEntryTypeEmote = 4, k_EChatEntryTypeLobbyGameStart = 5,
                      k_EChatEntryTypeWebLink = 6, k_EChatEntryTypeEntered = 7, k_EChatEntryTypeLeft = 8,
                      k_EChatEntryTypeWasKicked = 9, k_EChatEntryTypeWasBanned = 10, k_EChatEntryTypeDisconnected = 11,
                      k_EChatEntryTypeHistoricalChat = 12, k_EChatEntryTypeLinkBlocked = 14 };
enum ESteamAPICallFailure { k_ESteamAPICallFailureNone = -1, k_ESteamAPICallFailureSteamGone = 0,
                            k_ESteamAPICallFailureNetworkFailure = 1, k_ESteamAPICallFailureInvalidHandle = 2,
                            k_ESteamAPICallFailureMismatchedCallback = 3 };

enum ELobbyType { k_ELobbyTypePrivate = 0, k_ELobbyTypeFriendsOnly = 1, k_ELobbyTypePublic = 2,
                  k_ELobbyTypeInvisible = 3, k_ELobbyTypePrivateUnique = 4 };
enum ELobbyComparison { k_ELobbyComparisonEqualToOrLessThan = -2, k_ELobbyComparisonLessThan = -1,
                        k_ELobbyComparisonEqual = 0, k_ELobbyComparisonGreaterThan = 1,
                        k_ELobbyComparisonEqualToOrGreaterThan = 2, k_ELobbyComparisonNotEqual = 3 };
enum ELobbyDistanceFilter { k_ELobbyDistanceFilterClose = 0, k_ELobbyDistanceFilterDefault = 1,
                            k_ELobbyDistanceFilterFar = 2, k_ELobbyDistanceFilterWorldwide = 3 };
enum EChatRoomEnterResponse { k_EChatRoomEnterResponseSuccess = 1, k_EChatRoomEnterResponseDoesntExist = 2,
                              k_EChatRoomEnterResponseNotAllowed = 3, k_EChatRoomEnterResponseFull = 4,
                              k_EChatRoomEnterResponseError = 5, k_EChatRoomEnterResponseBanned = 6,
                              k_EChatRoomEnterResponseLimited = 7, k_EChatRoomEnterResponseClanDisabled = 8,
                              k_EChatRoomEnterResponseCommunityBan = 9, k_EChatRoomEnterResponseMemberBlockedYou = 10,
                              k_EChatRoomEnterResponseYouBlockedMember = 11,
                              k_EChatRoomEnterResponseRatelimitExceeded = 15 };

enum ELeaderboardSortMethod { k_ELeaderboardSortMethodNone = 0, k_ELeaderboardSortMethodAscending = 1,
                              k_ELeaderboardSortMethodDescending = 2 };
enum ELeaderboardDisplayType { k_ELeaderboardDisplayTypeNone = 0, k_ELeaderboardDisplayTypeNumeric = 1,
                               k_ELeaderboardDisplayTypeTimeSeconds = 2, k_ELeaderboardDisplayTypeTimeMilliSeconds = 3 };
enum ELeaderboardDataRequest { k_ELeaderboardDataRequestGlobal = 0, k_ELeaderboardDataRequestGlobalAroundUser = 1,
                               k_ELeaderboardDataRequestFriends = 2, k_ELeaderboardDataRequestUsers = 3 };
enum ELeaderboardUploadScoreMethod { k_ELeaderboardUploadScoreMethodNone = 0, k_ELeaderboardUploadScoreMethodKeepBest = 1,
                                     k_ELeaderboardUploadScoreMethodForceUpdate = 2 };

enum ENotificationPosition { k_EPositionInvalid = -1, k_EPositionTopLeft = 0, k_EPositionTopRight = 1,
                             k_EPositionBottomLeft = 2, k_EPositionBottomRight = 3 };
enum EGamepadTextInputMode { k_EGamepadTextInputModeNormal = 0, k_EGamepadTextInputModePassword = 1 };
enum EGamepadTextInputLineMode { k_EGamepadTextInputLineModeSingleLine = 0, k_EGamepadTextInputLineModeMultipleLines = 1 };
enum ETextFilteringContext { k_ETextFilteringContextUnknown = 0, k_ETextFilteringContextGameContent = 1,
                             k_ETextFilteringContextChat = 2, k_ETextFilteringContextName = 3 };
enum ESteamIPv6ConnectivityProtocol { k_ESteamIPv6ConnectivityProtocol_Invalid = 0,
                                      k_ESteamIPv6ConnectivityProtocol_HTTP = 1,
                                      k_ESteamIPv6ConnectivityProtocol_UDP = 2 };
enum ESteamIPv6ConnectivityState { k_ESteamIPv6ConnectivityState_Unknown = 0, k_ESteamIPv6ConnectivityState_Good = 1,
                                   k_ESteamIPv6ConnectivityState_Bad = 2 };

enum EP2PSend { k_EP2PSendUnreliable = 0, k_EP2PSendUnreliableNoDelay = 1, k_EP2PSendReliable = 2,
                k_EP2PSendReliableWithBuffering = 3 };
enum EP2PSessionError { k_EP2PSessionErrorNone = 0, k_EP2PSessionErrorNoRightsToUser = 2,
                        k_EP2PSessionErrorTimeout = 4 };
enum ESNetSocketState { k_ESNetSocketStateInvalid = 0, k_ESNetSocketStateConnected = 1 };
enum ESNetSocketConnectionType { k_ESNetSocketConnectionTypeNotConnected = 0, k_ESNetSocketConnectionTypeUDP = 1,
                                 k_ESNetSocketConnectionTypeUDPRelay = 2 };

enum ERemoteStoragePlatform { k_ERemoteStoragePlatformNone = 0, k_ERemoteStoragePlatformWindows = 1,
                              k_ERemoteStoragePlatformOSX = 2, k_ERemoteStoragePlatformPS3 = 4,
                              k_ERemoteStoragePlatformLinux = 8, k_ERemoteStoragePlatformSwitch = 16,
                              k_ERemoteStoragePlatformAndroid = 32, k_ERemoteStoragePlatformIOS = 64,
                              k_ERemoteStoragePlatformAll = 0xffffffff };
enum EUGCReadAction { k_EUGCRead_ContinueReadingUntilFinished = 0, k_EUGCRead_ContinueReading = 1,
                      k_EUGCRead_Close = 2 };
enum ERemoteStoragePublishedFileVisibility { k_ERemoteStoragePublishedFileVisibilityPublic = 0,
                                             k_ERemoteStoragePublishedFileVisibilityFriendsOnly = 1,
                                             k_ERemoteStoragePublishedFileVisibilityPrivate = 2,
                                             k_ERemoteStoragePublishedFileVisibilityUnlisted = 3 };
enum EWorkshopFileType { k_EWorkshopFileTypeFirst = 0, k_EWorkshopFileTypeCommunity = 0,
                         k_EWorkshopFileTypeMicrotransaction = 1, k_EWorkshopFileTypeCollection = 2,
                         k_EWorkshopFileTypeArt = 3, k_EWorkshopFileTypeVideo = 4, k_EWorkshopFileTypeScreenshot = 5,
                         k_EWorkshopFileTypeGame = 6, k_EWorkshopFileTypeSoftware = 7, k_EWorkshopFileTypeConcept = 8,
                         k_EWorkshopFileTypeWebGuide = 9, k_EWorkshopFileTypeIntegratedGuide = 10,
                         k_EWorkshopFileTypeMerch = 11, k_EWorkshopFileTypeControllerBinding = 12,
                         k_EWorkshopFileTypeSteamworksAccessInvite = 13, k_EWorkshopFileTypeSteamVideo = 14,
                         k_EWorkshopFileTypeGameManagedItem = 15, k_EWorkshopFileTypeMax = 16 };
enum EWorkshopFileAction { k_EWorkshopFileActionPlayed = 0, k_EWorkshopFileActionCompleted = 1 };
enum EWorkshopEnumerationType { k_EWorkshopEnumerationTypeRankedByVote = 0, k_EWorkshopEnumerationTypeRecent = 1,
                                k_EWorkshopEnumerationTypeTrending = 2, k_EWorkshopEnumerationTypeFavoritesOfFriends = 3,
                                k_EWorkshopEnumerationTypeVotedByFriends = 4, k_EWorkshopEnumerationTypeContentByFriends = 5,
                                k_EWorkshopEnumerationTypeRecentFromFollowedUsers = 6 };
enum EUserUGCList { k_EUserUGCList_Published = 0, k_EUserUGCList_VotedOn = 1, k_EUserUGCList_VotedUp = 2,
                    k_EUserUGCList_VotedDown = 3, k_EUserUGCList_WillVoteLater = 4, k_EUserUGCList_Favorited = 5,
                    k_EUserUGCList_Subscribed = 6, k_EUserUGCList_UsedOrPlayed = 7, k_EUserUGCList_Followed = 8 };
enum EUGCMatchingUGCType { k_EUGCMatchingUGCType_Items = 0, k_EUGCMatchingUGCType_Items_Mtx = 1,
                           k_EUGCMatchingUGCType_Items_ReadyToUse = 2, k_EUGCMatchingUGCType_Collections = 3,
                           k_EUGCMatchingUGCType_Artwork = 4, k_EUGCMatchingUGCType_Videos = 5,
                           k_EUGCMatchingUGCType_Screenshots = 6, k_EUGCMatchingUGCType_AllGuides = 7,
                           k_EUGCMatchingUGCType_WebGuides = 8, k_EUGCMatchingUGCType_IntegratedGuides = 9,
                           k_EUGCMatchingUGCType_UsableInGame = 10, k_EUGCMatchingUGCType_ControllerBindings = 11,
                           k_EUGCMatchingUGCType_GameManagedItems = 12, k_EUGCMatchingUGCType_All = ~0 };
enum EUserUGCListSortOrder { k_EUserUGCListSortOrder_CreationOrderDesc = 0, k_EUserUGCListSortOrder_CreationOrderAsc = 1,
                             k_EUserUGCListSortOrder_TitleAsc = 2, k_EUserUGCListSortOrder_LastUpdatedDesc = 3,
                             k_EUserUGCListSortOrder_SubscriptionDateDesc = 4, k_EUserUGCListSortOrder_VoteScoreDesc = 5,
                             k_EUserUGCListSortOrder_ForModeration = 6 };
enum EUGCQuery { k_EUGCQuery_RankedByVote = 0, k_EUGCQuery_RankedByPublicationDate = 1,
                 k_EUGCQuery_AcceptedForGameRankedByAcceptanceDate = 2, k_EUGCQuery_RankedByTrend = 3,
                 k_EUGCQuery_FavoritedByFriendsRankedByPublicationDate = 4, k_EUGCQuery_CreatedByFriendsRankedByPublicationDate = 5,
                 k_EUGCQuery_RankedByNumTimesReported = 6, k_EUGCQuery_CreatedByFollowedUsersRankedByPublicationDate = 7,
                 k_EUGCQuery_NotYetRated = 8, k_EUGCQuery_RankedByTotalVotesAsc = 9, k_EUGCQuery_RankedByVotesUp = 10,
                 k_EUGCQuery_RankedByTextSearch = 11, k_EUGCQuery_RankedByTotalUniqueSubscriptions = 12,
                 k_EUGCQuery_RankedByPlaytimeTrend = 13, k_EUGCQuery_RankedByTotalPlaytime = 14,
                 k_EUGCQuery_RankedByAveragePlaytimeTrend = 15, k_EUGCQuery_RankedByLifetimeAveragePlaytime = 16,
                 k_EUGCQuery_RankedByPlaytimeSessionsTrend = 17, k_EUGCQuery_RankedByLifetimePlaytimeSessions = 18,
                 k_EUGCQuery_RankedByLastUpdatedDate = 19 };
enum EItemPreviewType { k_EItemPreviewType_Image = 0, k_EItemPreviewType_YouTubeVideo = 1,
                        k_EItemPreviewType_Sketchfab = 2, k_EItemPreviewType_EnvironmentMap_HorizontalCross = 3,
                        k_EItemPreviewType_EnvironmentMap_LatLong = 4, k_EItemPreviewType_ReservedMax = 255 };
enum EItemStatistic { k_EItemStatistic_NumSubscriptions = 0, k_EItemStatistic_NumFavorites = 1,
                      k_EItemStatistic_NumFollowers = 2, k_EItemStatistic_NumUniqueSubscriptions = 3,
                      k_EItemStatistic_NumUniqueFavorites = 4, k_EItemStatistic_NumUniqueFollowers = 5,
                      k_EItemStatistic_NumUniqueWebsiteViews = 6, k_EItemStatistic_ReportScore = 7,
                      k_EItemStatistic_NumSecondsPlayed = 8, k_EItemStatistic_NumPlaytimeSessions = 9,
                      k_EItemStatistic_NumComments = 10, k_EItemStatistic_NumSecondsPlayedDuringTimePeriod = 11,
                      k_EItemStatistic_NumPlaytimeSessionsDuringTimePeriod = 12 };
enum EItemUpdateStatus { k_EItemUpdateStatusInvalid = 0, k_EItemUpdateStatusPreparingConfig = 1,
                         k_EItemUpdateStatusPreparingContent = 2, k_EItemUpdateStatusUploadingContent = 3,
                         k_EItemUpdateStatusUploadingPreviewFile = 4, k_EItemUpdateStatusCommittingChanges = 5 };
enum EVRScreenshotType { k_EVRScreenshotType_None = 0, k_EVRScreenshotType_Mono = 1, k_EVRScreenshotType_Stereo = 2,
                         k_EVRScreenshotType_MonoCubemap = 3, k_EVRScreenshotType_MonoPanorama = 4,
                         k_EVRScreenshotType_StereoPanorama = 5 };
enum EHTTPMethod { k_EHTTPMethodInvalid = 0, k_EHTTPMethodGET = 1, k_EHTTPMethodHEAD = 2, k_EHTTPMethodPOST = 3,
                   k_EHTTPMethodPUT = 4, k_EHTTPMethodDELETE = 5, k_EHTTPMethodOPTIONS = 6, k_EHTTPMethodPATCH = 7 };
enum ESteamInputType { k_ESteamInputType_Unknown = 0, k_ESteamInputType_SteamController = 1,
                       k_ESteamInputType_XBox360Controller = 2, k_ESteamInputType_XBoxOneController = 3,
                       k_ESteamInputType_GenericGamepad = 4, k_ESteamInputType_PS3Controller = 5,
                       k_ESteamInputType_PS4Controller = 6, k_ESteamInputType_PS5Controller = 7,
                       k_ESteamInputType_SteamDeckController = 8, k_ESteamInputType_SteamController2 = 9,
                       k_ESteamInputType_MaximumPossibleValue = 255 };
enum EControllerActionOrigin {
    k_EControllerActionOrigin_None = 0, k_EControllerActionOrigin_A = 1, k_EControllerActionOrigin_B = 2,
    k_EControllerActionOrigin_X = 3, k_EControllerActionOrigin_Y = 4, k_EControllerActionOrigin_LeftBumper = 5,
    k_EControllerActionOrigin_RightBumper = 6, k_EControllerActionOrigin_LeftGrip = 7, k_EControllerActionOrigin_RightGrip = 8,
    k_EControllerActionOrigin_Start = 9, k_EControllerActionOrigin_Back = 10, k_EControllerActionOrigin_LeftPad_Touch = 11,
    k_EControllerActionOrigin_LeftPad_Swipe = 12, k_EControllerActionOrigin_LeftPad_Click = 13,
    k_EControllerActionOrigin_LeftPad_DPadNorth = 14, k_EControllerActionOrigin_LeftPad_DPadSouth = 15,
    k_EControllerActionOrigin_LeftPad_DPadWest = 16, k_EControllerActionOrigin_LeftPad_DPadEast = 17,
    k_EControllerActionOrigin_RightPad_Touch = 18, k_EControllerActionOrigin_RightPad_Swipe = 19,
    k_EControllerActionOrigin_RightPad_Click = 20, k_EControllerActionOrigin_RightPad_DPadNorth = 21,
    k_EControllerActionOrigin_RightPad_DPadSouth = 22, k_EControllerActionOrigin_RightPad_DPadWest = 23,
    k_EControllerActionOrigin_RightPad_DPadEast = 24, k_EControllerActionOrigin_LeftTrigger_Pull = 25,
    k_EControllerActionOrigin_LeftTrigger_Click = 26, k_EControllerActionOrigin_RightTrigger_Pull = 27,
    k_EControllerActionOrigin_RightTrigger_Click = 28, k_EControllerActionOrigin_LeftStick_Move = 29,
    k_EControllerActionOrigin_LeftStick_Click = 30, k_EControllerActionOrigin_LeftStick_DPadNorth = 31,
    k_EControllerActionOrigin_LeftStick_DPadSouth = 32, k_EControllerActionOrigin_LeftStick_DPadWest = 33,
    k_EControllerActionOrigin_LeftStick_DPadEast = 34, k_EControllerActionOrigin_Gyro_Move = 35,
    k_EControllerActionOrigin_Gyro_Pitch = 36, k_EControllerActionOrigin_Gyro_Yaw = 37, k_EControllerActionOrigin_Gyro_Roll = 38,
    k_EControllerActionOrigin_PS4_X = 39, k_EControllerActionOrigin_PS4_Circle = 40, k_EControllerActionOrigin_PS4_Triangle = 41,
    k_EControllerActionOrigin_PS4_Square = 42, k_EControllerActionOrigin_PS4_LeftBumper = 43,
    k_EControllerActionOrigin_PS4_RightBumper = 44, k_EControllerActionOrigin_PS4_Options = 45,
    k_EControllerActionOrigin_PS4_Share = 46, k_EControllerActionOrigin_PS4_LeftPad_Touch = 47,
    k_EControllerActionOrigin_PS4_LeftPad_Swipe = 48, k_EControllerActionOrigin_PS4_LeftPad_Click = 49,
    k_EControllerActionOrigin_PS4_LeftPad_DPadNorth = 50, k_EControllerActionOrigin_PS4_LeftPad_DPadSouth = 51,
    k_EControllerActionOrigin_PS4_LeftPad_DPadWest = 52, k_EControllerActionOrigin_PS4_LeftPad_DPadEast = 53,
    k_EControllerActionOrigin_PS4_RightPad_Touch = 54, k_EControllerActionOrigin_PS4_RightPad_Swipe = 55,
    k_EControllerActionOrigin_PS4_RightPad_Click = 56, k_EControllerActionOrigin_PS4_RightPad_DPadNorth = 57,
    k_EControllerActionOrigin_PS4_RightPad_DPadSouth = 58, k_EControllerActionOrigin_PS4_RightPad_DPadWest = 59,
    k_EControllerActionOrigin_PS4_RightPad_DPadEast = 60, k_EControllerActionOrigin_PS4_LeftTrigger_Pull = 61,
    k_EControllerActionOrigin_PS4_LeftTrigger_Click = 62, k_EControllerActionOrigin_PS4_RightTrigger_Pull = 63,
    k_EControllerActionOrigin_PS4_RightTrigger_Click = 64, k_EControllerActionOrigin_PS4_LeftStick_Move = 65,
    k_EControllerActionOrigin_PS4_LeftStick_Click = 66, k_EControllerActionOrigin_PS4_LeftStick_DPadNorth = 67,
    k_EControllerActionOrigin_PS4_LeftStick_DPadSouth = 68, k_EControllerActionOrigin_PS4_LeftStick_DPadWest = 69,
    k_EControllerActionOrigin_PS4_LeftStick_DPadEast = 70, k_EControllerActionOrigin_PS4_RightStick_Move = 71,
    k_EControllerActionOrigin_PS4_RightStick_Click = 72, k_EControllerActionOrigin_PS4_RightStick_DPadNorth = 73,
    k_EControllerActionOrigin_PS4_RightStick_DPadSouth = 74, k_EControllerActionOrigin_PS4_RightStick_DPadWest = 75,
    k_EControllerActionOrigin_PS4_RightStick_DPadEast = 76, k_EControllerActionOrigin_PS4_DPad_North = 77,
    k_EControllerActionOrigin_PS4_DPad_South = 78, k_EControllerActionOrigin_PS4_DPad_West = 79,
    k_EControllerActionOrigin_PS4_DPad_East = 80, k_EControllerActionOrigin_PS4_Gyro_Move = 81,
    k_EControllerActionOrigin_PS4_Gyro_Pitch = 82, k_EControllerActionOrigin_PS4_Gyro_Yaw = 83,
    k_EControllerActionOrigin_PS4_Gyro_Roll = 84, k_EControllerActionOrigin_XBoxOne_A = 85,
    k_EControllerActionOrigin_Count = 512, k_EControllerActionOrigin_MaximumPossibleValue = 32767 };
enum EXboxOrigin {
    k_EXboxOrigin_A = 0, k_EXboxOrigin_B = 1, k_EXboxOrigin_X = 2, k_EXboxOrigin_Y = 3,
    k_EXboxOrigin_LeftBumper = 4, k_EXboxOrigin_RightBumper = 5, k_EXboxOrigin_Menu = 6, k_EXboxOrigin_View = 7,
    k_EXboxOrigin_LeftTrigger_Pull = 8, k_EXboxOrigin_LeftTrigger_Click = 9, k_EXboxOrigin_RightTrigger_Pull = 10,
    k_EXboxOrigin_RightTrigger_Click = 11, k_EXboxOrigin_LeftStick_Move = 12, k_EXboxOrigin_LeftStick_Click = 13,
    k_EXboxOrigin_LeftStick_DPadNorth = 14, k_EXboxOrigin_LeftStick_DPadSouth = 15, k_EXboxOrigin_LeftStick_DPadWest = 16,
    k_EXboxOrigin_LeftStick_DPadEast = 17, k_EXboxOrigin_RightStick_Move = 18, k_EXboxOrigin_RightStick_Click = 19,
    k_EXboxOrigin_RightStick_DPadNorth = 20, k_EXboxOrigin_RightStick_DPadSouth = 21,
    k_EXboxOrigin_RightStick_DPadWest = 22, k_EXboxOrigin_RightStick_DPadEast = 23, k_EXboxOrigin_DPad_North = 24,
    k_EXboxOrigin_DPad_South = 25, k_EXboxOrigin_DPad_West = 26, k_EXboxOrigin_DPad_East = 27,
    k_EXboxOrigin_LeftGrip_Lower = 28, k_EXboxOrigin_LeftGrip_Upper = 29, k_EXboxOrigin_RightGrip_Lower = 30,
    k_EXboxOrigin_RightGrip_Upper = 31, k_EXboxOrigin_Count = 32 };
enum ESteamControllerPad { k_ESteamControllerPad_Left = 0, k_ESteamControllerPad_Right = 1 };
enum EVoiceResult { k_EVoiceResultOK = 0, k_EVoiceResultNotInitialized = 1, k_EVoiceResultNotRecording = 2,
                    k_EVoiceResultNoData = 3, k_EVoiceResultBufferTooSmall = 4, k_EVoiceResultDataCorrupted = 5,
                    k_EVoiceResultRestricted = 6, k_EVoiceResultUnsupportedCodec = 7, k_EVoiceResultReceiverOutOfDate = 8,
                    k_EVoiceResultReceiverDidNotAnswer = 9 };
enum EBeginAuthSessionResult { k_EBeginAuthSessionResultOK = 0, k_EBeginAuthSessionResultInvalidTicket = 1,
                               k_EBeginAuthSessionResultDuplicateRequest = 2, k_EBeginAuthSessionResultInvalidVersion = 3,
                               k_EBeginAuthSessionResultGameMismatch = 4, k_EBeginAuthSessionResultExpiredTicket = 5 };
enum EAuthSessionResponse { k_EAuthSessionResponseOK = 0, k_EAuthSessionResponseUserNotConnectedToSteam = 1,
                            k_EAuthSessionResponseNoLicenseOrExpired = 2, k_EAuthSessionResponseVACBanned = 3,
                            k_EAuthSessionResponseLoggedInElseWhere = 4, k_EAuthSessionResponseVACCheckTimedOut = 5,
                            k_EAuthSessionResponseAuthTicketCanceled = 6, k_EAuthSessionResponseAuthTicketInvalidAlreadyUsed = 7,
                            k_EAuthSessionResponseAuthTicketInvalid = 8, k_EAuthSessionResponsePublisherIssuedBan = 9,
                            k_EAuthSessionResponseAuthTicketNetworkIdentityFailure = 10 };
enum EUserHasLicenseForAppResult { k_EUserHasLicenseResultHasLicense = 0, k_EUserHasLicenseResultDoesNotHaveLicense = 1,
                                   k_EUserHasLicenseResultNoAuth = 2 };
enum EServerMode { eServerModeInvalid = 0, eServerModeNoAuthentication = 1, eServerModeAuthentication = 2,
                   eServerModeAuthenticationAndSecure = 3 };
enum EMarketNotAllowedReasonFlags { k_EMarketNotAllowedReasonFlags_None = 0, k_EMarketNotAllowedReasonFlags_TemporaryFailure = 1,
                                    k_EMarketNotAllowedReasonFlags_AccountDisabled = 2, k_EMarketNotAllowedReasonFlags_AccountLockedDown = 4,
                                    k_EMarketNotAllowedReasonFlags_AccountLimited = 8, k_EMarketNotAllowedReasonFlags_TradeBanned = 16,
                                    k_EMarketNotAllowedReasonFlags_AccountNotTrusted = 32, k_EMarketNotAllowedReasonFlags_SteamGuardNotEnabled = 64,
                                    k_EMarketNotAllowedReasonFlags_SteamGuardOnlyRecentlyEnabled = 128,
                                    k_EMarketNotAllowedReasonFlags_RecentPasswordReset = 256,
                                    k_EMarketNotAllowedReasonFlags_NewPaymentMethod = 512, k_EMarketNotAllowedReasonFlags_InvalidCookie = 1024,
                                    k_EMarketNotAllowedReasonFlags_UsingNewDevice = 2048, k_EMarketNotAllowedReasonFlags_RecentSelfRefund = 4096,
                                    k_EMarketNotAllowedReasonFlags_NewPaymentMethodCannotBeVerified = 8192,
                                    k_EMarketNotAllowedReasonFlags_NoRecentPurchases = 16384, k_EMarketNotAllowedReasonFlags_AcceptedWalletGift = 32768 };
enum EDurationControlProgress { k_EDurationControlProgress_Full = 0, k_EDurationControlProgress_Half = 1,
                                k_EDurationControlProgress_None = 2, k_EDurationControlExitSoon_3h = 3,
                                k_EDurationControlExitSoon_5h = 4, k_EDurationControlExitSoon_Night = 5 };
enum EDurationControlNotification { k_EDurationControlNotification_None = 0, k_EDurationControlNotification_1Hour = 1,
                                    k_EDurationControlNotification_3Hours = 2, k_EDurationControlNotification_HalfProgress = 3,
                                    k_EDurationControlNotification_ExitSoon_3h = 4, k_EDurationControlNotification_ExitSoon_5h = 5,
                                    k_EDurationControlNotification_ExitSoon_Night = 6 };
enum EDurationControlOnlineState { k_EDurationControlOnlineState_Invalid = 0, k_EDurationControlOnlineState_Offline = 1,
                                   k_EDurationControlOnlineState_Online = 2, k_EDurationControlOnlineState_OnlineHighPri = 3 };
enum EActivateGameOverlayToWebPageMode { k_EActivateGameOverlayToWebPageMode_Default = 0,
                                         k_EActivateGameOverlayToWebPageMode_Modal = 1 };
enum EOverlayToStoreFlag { k_EOverlayToStoreFlag_None = 0, k_EOverlayToStoreFlag_AddToCart = 1,
                           k_EOverlayToStoreFlag_AddToCartAndShow = 2 };

constexpr uint64_t MakeSteamID(uint32_t accountId, uint32_t instance = 1,
                               uint32_t type = k_EAccountTypeIndividual,
                               uint32_t universe = k_EUniversePublic) {
    return (static_cast<uint64_t>(universe) << 56) | (static_cast<uint64_t>(type) << 52) |
           (static_cast<uint64_t>(instance) << 32) | accountId;
}

struct P2PSessionState_t {
    uint8_t m_bConnectionActive;
    uint8_t m_bConnecting;
    uint8_t m_eP2PSessionError;
    uint8_t m_bUsingRelay;
    int32_t m_nBytesQueuedForSend;
    int32_t m_nPacketsQueuedForSend;
    uint32_t m_nRemoteIP;
    uint16_t m_nRemotePort;
};

struct FriendGameInfo_t {
    CGameID m_gameID;
    uint32_t m_unGameIP;
    uint16_t m_usGamePort;
    uint16_t m_usQueryPort;
    CSteamID m_steamIDLobby;
};

struct LeaderboardEntry_t {
    CSteamID m_steamIDUser;
    int32_t m_nGlobalRank;
    int32_t m_nScore;
    int32_t m_cDetails;
    UGCHandle_t m_hUGC;
};

struct SteamParamStringArray_t {
    const char** m_ppStrings;
    int32_t m_nNumStrings;
};

struct SteamUGCDetails_t {
    PublishedFileId_t m_nPublishedFileId;
    EResult m_eResult;
    EWorkshopFileType m_eFileType;
    AppId_t m_nCreatorAppID;
    AppId_t m_nConsumerAppID;
    char m_rgchTitle[129];
    char m_rgchDescription[8000];
    uint64_t m_ulSteamIDOwner;
    uint32_t m_rtimeCreated;
    uint32_t m_rtimeUpdated;
    uint32_t m_rtimeAddedToUserList;
    ERemoteStoragePublishedFileVisibility m_eVisibility;
    bool m_bBanned;
    bool m_bAcceptedForUse;
    bool m_bTagsTruncated;
    char m_rgchTags[1025];
    UGCHandle_t m_hFile;
    UGCHandle_t m_hPreviewFile;
    char m_pchFileName[260];
    int32_t m_nFileSize;
    int32_t m_nPreviewFileSize;
    char m_rgchURL[256];
    uint32_t m_unVotesUp;
    uint32_t m_unVotesDown;
    float m_flScore;
    uint32_t m_unNumChildren;
    uint64_t m_ulTotalFilesSize;
};

struct SteamItemDetails_t {
    SteamItemInstanceID_t m_itemId;
    SteamItemDef_t m_iDefinition;
    uint16_t m_unQuantity;
    uint16_t m_unFlags;
};

struct SteamIPAddress_t {
    union {
        uint32_t m_ipv4;
        uint8_t m_rgubIPv6[16];
    };
    uint16_t m_eType;
};

struct MatchMakingKeyValuePair_t {
    char m_szKey[256];
    char m_szValue[256];
};

struct servernetadr_t {
    uint16_t m_usConnectionPort;
    uint16_t m_usQueryPort;
    uint32_t m_unIP;
};

struct gameserveritem_t {
    servernetadr_t m_NetAdr;
    int m_nPing;
    bool m_bHadSuccessfulResponse;
    bool m_bDoNotRefresh;
    char m_szGameDir[32];
    char m_szMap[32];
    char m_szGameDescription[64];
    uint32_t m_nAppID;
    int m_nPlayers;
    int m_nMaxPlayers;
    int m_nBotPlayers;
    bool m_bPassword;
    bool m_bSecure;
    uint32_t m_ulTimeLastPlayed;
    int m_nServerVersion;
    char m_szServerName[64];
    char m_szGameTags[128];
    CSteamID m_steamID;
};

struct InputDigitalActionData_t {
    bool bState;
    bool bActive;
};
struct InputAnalogActionData_t {
    int32_t eMode;
    float x, y;
    bool bActive;
};
struct InputMotionData_t {
    float rotQuatX, rotQuatY, rotQuatZ, rotQuatW;
    float posAccelX, posAccelY, posAccelZ;
    float rotVelX, rotVelY, rotVelZ;
};

constexpr int k_iCallback_UserStatsReceived = 1101;
constexpr int k_iCallback_UserStatsStored = 1102;
constexpr int k_iCallback_LeaderboardFindResult = 1104;
constexpr int k_iCallback_LeaderboardScoresDownloaded = 1105;
constexpr int k_iCallback_LeaderboardScoreUploaded = 1106;
constexpr int k_iCallback_NumberOfCurrentPlayers = 1107;
constexpr int k_iCallback_GlobalAchievementPercentagesReady = 1110;
constexpr int k_iCallback_GlobalStatsReceived = 1112;
constexpr int k_iCallback_LobbyEnter = 504;
constexpr int k_iCallback_LobbyDataUpdate = 505;
constexpr int k_iCallback_LobbyChatUpdate = 506;
constexpr int k_iCallback_LobbyChatMsg = 507;
constexpr int k_iCallback_LobbyMatchList = 510;
constexpr int k_iCallback_LobbyCreated = 513;
constexpr int k_iCallback_PersonaStateChange = 304;
constexpr int k_iCallback_RemoteStorageFileWriteAsyncComplete = 1303;
constexpr int k_iCallback_RemoteStorageFileReadAsyncComplete = 1304;
constexpr int k_iCallback_RemoteStorageFileShareResult = 1307;

struct UserStatsReceived_t {
    CGameID m_nGameID;
    EResult m_eResult;
    CSteamID m_steamIDUser;
};
struct UserStatsStored_t {
    CGameID m_nGameID;
    EResult m_eResult;
};
struct GlobalStatsReceived_t {
    CGameID m_nGameID;
    EResult m_eResult;
};
struct GlobalAchievementPercentagesReady_t {
    CGameID m_nGameID;
    EResult m_eResult;
};
struct NumberOfCurrentPlayers_t {
    uint8_t m_bSuccess;
    int32_t m_cPlayers;
};
struct LeaderboardFindResult_t {
    SteamLeaderboard_t m_hSteamLeaderboard;
    uint8_t m_bLeaderboardFound;
};
struct LeaderboardScoresDownloaded_t {
    SteamLeaderboard_t m_hSteamLeaderboard;
    SteamLeaderboardEntries_t m_hSteamLeaderboardEntries;
    int m_cEntryCount;
};
struct LeaderboardScoreUploaded_t {
    uint8_t m_bSuccess;
    SteamLeaderboard_t m_hSteamLeaderboard;
    int32_t m_nScore;
    uint8_t m_bScoreChanged;
    int m_nGlobalRankNew;
    int m_nGlobalRankPrevious;
};
struct LobbyMatchList_t {
    uint32_t m_nLobbiesMatching;
};
struct LobbyCreated_t {
    EResult m_eResult;
    CSteamID m_ulSteamIDLobby;
};
struct LobbyEnter_t {
    CSteamID m_ulSteamIDLobby;
    uint32_t m_rgfChatPermissions;
    bool m_bLocked;
    uint32_t m_EChatRoomEnterResponse;
};
struct LobbyDataUpdate_t {
    CSteamID m_ulSteamIDLobby;
    CSteamID m_ulSteamIDMember;
    uint8_t m_bSuccess;
};
struct LobbyChatUpdate_t {
    CSteamID m_ulSteamIDLobby;
    CSteamID m_ulSteamIDUserChanged;
    CSteamID m_ulSteamIDMakingChange;
    uint32_t m_rgfChatMemberStateChange;
};
struct LobbyChatMsg_t {
    CSteamID m_ulSteamIDLobby;
    CSteamID m_ulSteamIDUser;
    uint8_t m_eChatEntryType;
    uint32_t m_iChatID;
};
struct PersonaStateChange_t {
    CSteamID m_ulSteamID;
    int m_nChangeFlags;
};
struct RemoteStorageFileReadAsyncComplete_t {
    SteamAPICall_t m_hFileReadAsync;
    EResult m_eResult;
    uint32_t m_nOffset;
    uint32_t m_cubRead;
};
struct RemoteStorageFileWriteAsyncComplete_t {
    EResult m_eResult;
};
struct RemoteStorageFileShareResult_t {
    EResult m_eResult;
    UGCHandle_t m_hFile;
    char m_rgchFilename[260];
};

struct CallbackMsg_t {
    HSteamUser m_hSteamUser;
    int m_iCallback;
    uint8_t* m_pubParam;
    int m_cubParam;
};

} // namespace sfemu
