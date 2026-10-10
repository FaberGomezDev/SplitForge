// Funciones auxiliares de la API plana para SteamNetworkingIPAddr y SteamNetworkingIdentity.
// Son métodos de estructuras (no de interfaces): Steamworks.NET las importa y las llama sobre su
// propia copia de la estructura, así que la disposición en memoria debe ser la del SDK
// (steamnetworkingtypes.h, pack(1)): IPAddr = 16 bytes IPv6 + puerto; Identity = tipo, tamaño y
// unión de 128 bytes.

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace {

#pragma pack(push, 1)
struct NetIPAddr {
    uint8_t ipv6[16]; // IPv4 como ::ffff:a.b.c.d (orden de red)
    uint16_t port;    // orden del host
};

struct NetIdentity {
    int32_t type;
    int32_t size;
    union {
        uint64_t steamId64;
        uint64_t psnId;
        uint64_t stadiaId;
        char genericString[32];
        char xboxPairwiseId[33];
        uint8_t genericBytes[32];
        NetIPAddr ip;
        uint32_t reserved[32];
    };
};
#pragma pack(pop)
static_assert(sizeof(NetIPAddr) == 18, "ABI de SteamNetworkingIPAddr");
static_assert(sizeof(NetIdentity) == 136, "ABI de SteamNetworkingIdentity");

// ESteamNetworkingIdentityType y ESteamNetworkingFakeIPType del SDK.
constexpr int32_t kIdentityInvalid = 0;
constexpr int32_t kIdentityIPAddress = 1;
constexpr int32_t kIdentityGenericString = 2;
constexpr int32_t kIdentityGenericBytes = 3;
constexpr int32_t kIdentitySteamID = 16;
constexpr int32_t kIdentityXboxPairwiseID = 17;
constexpr int32_t kIdentitySonyPSN = 18;
constexpr int32_t kIdentityGoogleStadia = 19;
constexpr int kFakeIPInvalid = 0;
constexpr int kFakeIPNotFake = 1;

constexpr uint8_t kIPv4Prefix[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff};

bool IsIPv4(const NetIPAddr* a) {
    return std::memcmp(a->ipv6, kIPv4Prefix, sizeof(kIPv4Prefix)) == 0;
}

uint32_t GetIPv4(const NetIPAddr* a) {
    if (!IsIPv4(a))
        return 0;
    return (uint32_t{a->ipv6[12]} << 24) | (uint32_t{a->ipv6[13]} << 16) | (uint32_t{a->ipv6[14]} << 8) |
           uint32_t{a->ipv6[15]};
}

void SetIPv4(NetIPAddr* a, uint32_t ip, uint16_t port) {
    std::memcpy(a->ipv6, kIPv4Prefix, sizeof(kIPv4Prefix));
    a->ipv6[12] = static_cast<uint8_t>(ip >> 24);
    a->ipv6[13] = static_cast<uint8_t>(ip >> 16);
    a->ipv6[14] = static_cast<uint8_t>(ip >> 8);
    a->ipv6[15] = static_cast<uint8_t>(ip);
    a->port = port;
}

bool IsLocalHost(const NetIPAddr* a) {
    if (IsIPv4(a))
        return a->ipv6[12] == 127;
    static constexpr uint8_t loopback[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    return std::memcmp(a->ipv6, loopback, sizeof(loopback)) == 0;
}

void IPToString(const NetIPAddr* a, char* buffer, uint32_t size, bool withPort) {
    if (!buffer || size == 0)
        return;
    if (IsIPv4(a)) {
        if (withPort)
            std::snprintf(buffer, size, "%u.%u.%u.%u:%u", a->ipv6[12], a->ipv6[13], a->ipv6[14], a->ipv6[15], a->port);
        else
            std::snprintf(buffer, size, "%u.%u.%u.%u", a->ipv6[12], a->ipv6[13], a->ipv6[14], a->ipv6[15]);
        return;
    }
    char text[48] = {};
    int length = 0;
    for (int i = 0; i < 16; i += 2)
        length += std::snprintf(text + length, sizeof(text) - static_cast<size_t>(length), i ? ":%x" : "%x",
                                (a->ipv6[i] << 8) | a->ipv6[i + 1]);
    if (withPort)
        std::snprintf(buffer, size, "[%s]:%u", text, a->port);
    else
        std::snprintf(buffer, size, "%s", text);
}

// Solo IPv4 "a.b.c.d" o "a.b.c.d:puerto" (lo que usan los juegos en LAN).
bool ParseIP(NetIPAddr* a, const char* text) {
    if (!text)
        return false;
    unsigned b0 = 0, b1 = 0, b2 = 0, b3 = 0, port = 0;
    char extra = 0;
    const int read = std::sscanf(text, "%u.%u.%u.%u:%u%c", &b0, &b1, &b2, &b3, &port, &extra);
    if ((read != 4 && read != 5) || b0 > 255 || b1 > 255 || b2 > 255 || b3 > 255 || port > 65535)
        return false;
    SetIPv4(a, (b0 << 24) | (b1 << 16) | (b2 << 8) | b3, static_cast<uint16_t>(port));
    return true;
}

} // namespace

extern "C" {

// --- SteamNetworkingIPAddr ---

void SteamAPI_SteamNetworkingIPAddr_Clear(NetIPAddr* self) {
    if (self)
        std::memset(self, 0, sizeof(*self));
}

bool SteamAPI_SteamNetworkingIPAddr_IsIPv6AllZeros(NetIPAddr* self) {
    static constexpr uint8_t zeros[16] = {};
    return self && std::memcmp(self->ipv6, zeros, sizeof(zeros)) == 0;
}

void SteamAPI_SteamNetworkingIPAddr_SetIPv6(NetIPAddr* self, const uint8_t* ipv6, uint16_t port) {
    if (!self || !ipv6)
        return;
    std::memcpy(self->ipv6, ipv6, sizeof(self->ipv6));
    self->port = port;
}

void SteamAPI_SteamNetworkingIPAddr_SetIPv4(NetIPAddr* self, uint32_t ip, uint16_t port) {
    if (self)
        SetIPv4(self, ip, port);
}

bool SteamAPI_SteamNetworkingIPAddr_IsIPv4(NetIPAddr* self) {
    return self && IsIPv4(self);
}

uint32_t SteamAPI_SteamNetworkingIPAddr_GetIPv4(NetIPAddr* self) {
    return self ? GetIPv4(self) : 0;
}

void SteamAPI_SteamNetworkingIPAddr_SetIPv6LocalHost(NetIPAddr* self, uint16_t port) {
    if (!self)
        return;
    std::memset(self->ipv6, 0, sizeof(self->ipv6));
    self->ipv6[15] = 1;
    self->port = port;
}

bool SteamAPI_SteamNetworkingIPAddr_IsLocalHost(NetIPAddr* self) {
    return self && IsLocalHost(self);
}

void SteamAPI_SteamNetworkingIPAddr_ToString(NetIPAddr* self, char* buffer, uint32_t size, bool withPort) {
    if (self)
        IPToString(self, buffer, size, withPort);
    else if (buffer && size)
        buffer[0] = 0;
}

bool SteamAPI_SteamNetworkingIPAddr_ParseString(NetIPAddr* self, const char* text) {
    return self && ParseIP(self, text);
}

bool SteamAPI_SteamNetworkingIPAddr_IsEqualTo(NetIPAddr* self, const NetIPAddr* other) {
    return self && other && std::memcmp(self->ipv6, other->ipv6, sizeof(self->ipv6)) == 0 && self->port == other->port;
}

int SteamAPI_SteamNetworkingIPAddr_GetFakeIPType(NetIPAddr* self) {
    return self && IsIPv4(self) ? kFakeIPNotFake : kFakeIPInvalid;
}

bool SteamAPI_SteamNetworkingIPAddr_IsFakeIP(NetIPAddr*) {
    return false;
}

// --- SteamNetworkingIdentity ---

void SteamAPI_SteamNetworkingIdentity_Clear(NetIdentity* self) {
    if (self)
        std::memset(self, 0, sizeof(*self));
}

bool SteamAPI_SteamNetworkingIdentity_IsInvalid(NetIdentity* self) {
    return !self || self->type == kIdentityInvalid;
}

void SteamAPI_SteamNetworkingIdentity_SetSteamID64(NetIdentity* self, uint64_t steamId) {
    if (!self)
        return;
    SteamAPI_SteamNetworkingIdentity_Clear(self);
    self->type = kIdentitySteamID;
    self->size = sizeof(uint64_t);
    self->steamId64 = steamId;
}

void SteamAPI_SteamNetworkingIdentity_SetSteamID(NetIdentity* self, uint64_t steamId) {
    SteamAPI_SteamNetworkingIdentity_SetSteamID64(self, steamId);
}

uint64_t SteamAPI_SteamNetworkingIdentity_GetSteamID64(NetIdentity* self) {
    return self && self->type == kIdentitySteamID ? self->steamId64 : 0;
}

uint64_t SteamAPI_SteamNetworkingIdentity_GetSteamID(NetIdentity* self) {
    return SteamAPI_SteamNetworkingIdentity_GetSteamID64(self);
}

bool SteamAPI_SteamNetworkingIdentity_SetXboxPairwiseID(NetIdentity* self, const char* id) {
    if (!self || !id)
        return false;
    const size_t length = std::strlen(id);
    if (length >= sizeof(self->xboxPairwiseId))
        return false;
    SteamAPI_SteamNetworkingIdentity_Clear(self);
    self->type = kIdentityXboxPairwiseID;
    self->size = static_cast<int32_t>(length + 1);
    std::memcpy(self->xboxPairwiseId, id, length + 1);
    return true;
}

const char* SteamAPI_SteamNetworkingIdentity_GetXboxPairwiseID(NetIdentity* self) {
    return self && self->type == kIdentityXboxPairwiseID ? self->xboxPairwiseId : nullptr;
}

void SteamAPI_SteamNetworkingIdentity_SetPSNID(NetIdentity* self, uint64_t id) {
    if (!self)
        return;
    SteamAPI_SteamNetworkingIdentity_Clear(self);
    self->type = kIdentitySonyPSN;
    self->size = sizeof(uint64_t);
    self->psnId = id;
}

uint64_t SteamAPI_SteamNetworkingIdentity_GetPSNID(NetIdentity* self) {
    return self && self->type == kIdentitySonyPSN ? self->psnId : 0;
}

void SteamAPI_SteamNetworkingIdentity_SetStadiaID(NetIdentity* self, uint64_t id) {
    if (!self)
        return;
    SteamAPI_SteamNetworkingIdentity_Clear(self);
    self->type = kIdentityGoogleStadia;
    self->size = sizeof(uint64_t);
    self->stadiaId = id;
}

uint64_t SteamAPI_SteamNetworkingIdentity_GetStadiaID(NetIdentity* self) {
    return self && self->type == kIdentityGoogleStadia ? self->stadiaId : 0;
}

// Steamworks.NET la declara con retorno IntPtr; el SDK, void. Devolver la identidad es inocuo.
NetIdentity* SteamAPI_SteamNetworkingIdentity_SetIPAddr(NetIdentity* self, const NetIPAddr* address) {
    if (!self || !address)
        return self;
    SteamAPI_SteamNetworkingIdentity_Clear(self);
    self->type = kIdentityIPAddress;
    self->size = sizeof(NetIPAddr);
    self->ip = *address;
    return self;
}

const NetIPAddr* SteamAPI_SteamNetworkingIdentity_GetIPAddr(NetIdentity* self) {
    return self && self->type == kIdentityIPAddress ? &self->ip : nullptr;
}

void SteamAPI_SteamNetworkingIdentity_SetIPv4Addr(NetIdentity* self, uint32_t ip, uint16_t port) {
    if (!self)
        return;
    SteamAPI_SteamNetworkingIdentity_Clear(self);
    self->type = kIdentityIPAddress;
    self->size = sizeof(NetIPAddr);
    SetIPv4(&self->ip, ip, port);
}

uint32_t SteamAPI_SteamNetworkingIdentity_GetIPv4(NetIdentity* self) {
    return self && self->type == kIdentityIPAddress ? GetIPv4(&self->ip) : 0;
}

int SteamAPI_SteamNetworkingIdentity_GetFakeIPType(NetIdentity* self) {
    return self && self->type == kIdentityIPAddress && IsIPv4(&self->ip) ? kFakeIPNotFake : kFakeIPInvalid;
}

bool SteamAPI_SteamNetworkingIdentity_IsFakeIP(NetIdentity*) {
    return false;
}

void SteamAPI_SteamNetworkingIdentity_SetLocalHost(NetIdentity* self) {
    if (!self)
        return;
    SteamAPI_SteamNetworkingIdentity_Clear(self);
    self->type = kIdentityIPAddress;
    self->size = sizeof(NetIPAddr);
    SteamAPI_SteamNetworkingIPAddr_SetIPv6LocalHost(&self->ip, 0);
}

bool SteamAPI_SteamNetworkingIdentity_IsLocalHost(NetIdentity* self) {
    return self && self->type == kIdentityIPAddress && IsLocalHost(&self->ip);
}

bool SteamAPI_SteamNetworkingIdentity_SetGenericString(NetIdentity* self, const char* text) {
    if (!self || !text)
        return false;
    const size_t length = std::strlen(text);
    if (length >= sizeof(self->genericString))
        return false;
    SteamAPI_SteamNetworkingIdentity_Clear(self);
    self->type = kIdentityGenericString;
    self->size = static_cast<int32_t>(length + 1);
    std::memcpy(self->genericString, text, length + 1);
    return true;
}

const char* SteamAPI_SteamNetworkingIdentity_GetGenericString(NetIdentity* self) {
    return self && self->type == kIdentityGenericString ? self->genericString : nullptr;
}

bool SteamAPI_SteamNetworkingIdentity_SetGenericBytes(NetIdentity* self, const void* data, uint32_t length) {
    if (!self || (!data && length) || length > sizeof(self->genericBytes))
        return false;
    SteamAPI_SteamNetworkingIdentity_Clear(self);
    self->type = kIdentityGenericBytes;
    self->size = static_cast<int32_t>(length);
    if (length)
        std::memcpy(self->genericBytes, data, length);
    return true;
}

const uint8_t* SteamAPI_SteamNetworkingIdentity_GetGenericBytes(NetIdentity* self, int* length) {
    if (!self || self->type != kIdentityGenericBytes)
        return nullptr;
    if (length)
        *length = self->size;
    return self->genericBytes;
}

bool SteamAPI_SteamNetworkingIdentity_IsEqualTo(NetIdentity* self, const NetIdentity* other) {
    if (!self || !other || self->type != other->type || self->size != other->size)
        return false;
    if (self->size < 0 || self->size > static_cast<int32_t>(sizeof(self->reserved)))
        return false;
    return std::memcmp(self->reserved, other->reserved, static_cast<size_t>(self->size)) == 0;
}

void SteamAPI_SteamNetworkingIdentity_ToString(NetIdentity* self, char* buffer, uint32_t size) {
    if (!buffer || size == 0)
        return;
    buffer[0] = 0;
    if (!self)
        return;
    char ip[64] = {};
    switch (self->type) {
    case kIdentitySteamID:
        std::snprintf(buffer, size, "steamid:%llu", static_cast<unsigned long long>(self->steamId64));
        break;
    case kIdentityIPAddress:
        IPToString(&self->ip, ip, sizeof(ip), true);
        std::snprintf(buffer, size, "ip:%s", ip);
        break;
    case kIdentityGenericString:
        std::snprintf(buffer, size, "str:%s", self->genericString);
        break;
    case kIdentityInvalid:
        std::snprintf(buffer, size, "invalid");
        break;
    default:
        std::snprintf(buffer, size, "unknown:%d", self->type);
        break;
    }
}

bool SteamAPI_SteamNetworkingIdentity_ParseString(NetIdentity* self, const char* text) {
    if (!self || !text)
        return false;
    unsigned long long steamId = 0;
    char extra = 0;
    if (std::sscanf(text, "steamid:%llu%c", &steamId, &extra) == 1) {
        SteamAPI_SteamNetworkingIdentity_SetSteamID64(self, steamId);
        return true;
    }
    if (std::strncmp(text, "ip:", 3) == 0) {
        NetIPAddr address{};
        if (!ParseIP(&address, text + 3))
            return false;
        SteamAPI_SteamNetworkingIdentity_SetIPAddr(self, &address);
        return true;
    }
    if (std::strncmp(text, "str:", 4) == 0)
        return SteamAPI_SteamNetworkingIdentity_SetGenericString(self, text + 4);
    return false;
}

// El emulador nunca entrega mensajes de SteamNetworkingSockets/Messages (sus colas están vacías),
// así que no hay nada que liberar.
void SteamAPI_SteamNetworkingMessage_t_Release(void*) {}

// Variantes que Steamworks.NET importa desde ISteamNetworkingUtils (con self de la interfaz).
void SteamAPI_ISteamNetworkingUtils_SteamNetworkingIPAddr_ToString(void*, NetIPAddr* address, char* buffer,
                                                                   uint32_t size, bool withPort) {
    SteamAPI_SteamNetworkingIPAddr_ToString(address, buffer, size, withPort);
}

bool SteamAPI_ISteamNetworkingUtils_SteamNetworkingIPAddr_ParseString(void*, NetIPAddr* address, const char* text) {
    return SteamAPI_SteamNetworkingIPAddr_ParseString(address, text);
}

int SteamAPI_ISteamNetworkingUtils_SteamNetworkingIPAddr_GetFakeIPType(void*, NetIPAddr* address) {
    return SteamAPI_SteamNetworkingIPAddr_GetFakeIPType(address);
}

void SteamAPI_ISteamNetworkingUtils_SteamNetworkingIdentity_ToString(void*, NetIdentity* identity, char* buffer,
                                                                     uint32_t size) {
    SteamAPI_SteamNetworkingIdentity_ToString(identity, buffer, size);
}

bool SteamAPI_ISteamNetworkingUtils_SteamNetworkingIdentity_ParseString(void*, NetIdentity* identity,
                                                                       const char* text) {
    return SteamAPI_SteamNetworkingIdentity_ParseString(identity, text);
}

} // extern "C"
