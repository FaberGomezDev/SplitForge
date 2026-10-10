#include "minijson.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace sfemu {

Json::Json(std::string value) : kind_(Kind::String), str_(std::move(value)) {}
Json::Json(double value) : kind_(Kind::Number), num_(value) {}
Json::Json(bool value) : kind_(Kind::Bool), boolean_(value) {}

const Json* Json::find(const std::string& key) const {
    if (kind_ != Kind::Object)
        return nullptr;
    const auto it = object_.find(key);
    return it == object_.end() ? nullptr : &it->second;
}

Json& Json::at(const std::string& key) {
    if (kind_ != Kind::Object) {
        kind_ = Kind::Object;
        object_.clear();
    }
    return object_[key];
}

void Json::push(Json value) {
    kind_ = Kind::Array;
    array_.push_back(std::move(value));
}

void Json::set(const std::string& key, Json value) {
    kind_ = Kind::Object;
    object_[key] = std::move(value);
}

namespace {

struct Parser {
    const char* p;
    const char* end;
    int* errorPos;

    explicit Parser(const std::string& text, int* err) : p(text.c_str()), end(p + text.size()), errorPos(err) {}

    void SkipWs() {
        while (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'))
            p++;
    }

    bool ParseString(std::string& out) {
        if (p >= end || *p != '"')
            return false;
        p++;
        out.clear();
        while (p < end) {
            const char c = *p++;
            if (c == '"')
                return true;
            if (c == '\\') {
                if (p >= end)
                    return false;
                const char e = *p++;
                switch (e) {
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case '/': out += '/'; break;
                    case 'b': out += '\b'; break;
                    case 'f': out += '\f'; break;
                    case 'n': out += '\n'; break;
                    case 'r': out += '\r'; break;
                    case 't': out += '\t'; break;
                    case 'u': {
                        if (end - p < 4)
                            return false;
                        unsigned code = 0;
                        for (int i = 0; i < 4; i++) {
                            const char h = *p++;
                            code <<= 4;
                            if (h >= '0' && h <= '9') code |= static_cast<unsigned>(h - '0');
                            else if (h >= 'a' && h <= 'f') code |= static_cast<unsigned>(h - 'a' + 10);
                            else if (h >= 'A' && h <= 'F') code |= static_cast<unsigned>(h - 'A' + 10);
                            else return false;
                        }
                        if (code < 0x80) {
                            out += static_cast<char>(code);
                        } else if (code < 0x800) {
                            out += static_cast<char>(0xC0 | (code >> 6));
                            out += static_cast<char>(0x80 | (code & 0x3F));
                        } else {
                            out += static_cast<char>(0xE0 | (code >> 12));
                            out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                            out += static_cast<char>(0x80 | (code & 0x3F));
                        }
                        break;
                    }
                    default: return false;
                }
            } else {
                out += c;
            }
        }
        return false;
    }

    bool ParseValue(Json& out) {
        SkipWs();
        if (p >= end)
            return false;
        const char c = *p;
        if (c == '"') {
            std::string s;
            if (!ParseString(s))
                return false;
            out = Json(std::move(s));
            return true;
        }
        if (c == '{') {
            p++;
            out = Json();
            Json obj;
            SkipWs();
            if (p < end && *p == '}') {
                p++;
                return true;
            }
            for (;;) {
                SkipWs();
                std::string key;
                if (!ParseString(key))
                    return false;
                SkipWs();
                if (p >= end || *p != ':')
                    return false;
                p++;
                Json value;
                if (!ParseValue(value))
                    return false;
                obj.set(std::move(key), std::move(value));
                SkipWs();
                if (p >= end)
                    return false;
                if (*p == ',') {
                    p++;
                    continue;
                }
                if (*p == '}') {
                    p++;
                    out = std::move(obj);
                    return true;
                }
                return false;
            }
        }
        if (c == '[') {
            p++;
            Json arr;
            SkipWs();
            if (p < end && *p == ']') {
                p++;
                out = std::move(arr);
                return true;
            }
            for (;;) {
                Json value;
                if (!ParseValue(value))
                    return false;
                arr.push(std::move(value));
                SkipWs();
                if (p >= end)
                    return false;
                if (*p == ',') {
                    p++;
                    continue;
                }
                if (*p == ']') {
                    p++;
                    out = std::move(arr);
                    return true;
                }
                return false;
            }
        }
        if (std::strncmp(p, "true", 4) == 0) {
            p += 4;
            out = Json(true);
            return true;
        }
        if (std::strncmp(p, "false", 5) == 0) {
            p += 5;
            out = Json(false);
            return true;
        }
        if (std::strncmp(p, "null", 4) == 0) {
            p += 4;
            out = Json();
            return true;
        }
        char* numEnd = nullptr;
        const double number = std::strtod(p, &numEnd);
        if (numEnd == p)
            return false;
        p = numEnd;
        out = Json(number);
        return true;
    }
};

void DumpString(const std::string& s, std::string& out) {
    out += '"';
    for (const unsigned char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    out += '"';
}

void DumpValue(const Json& value, std::string& out) {
    switch (value.kind()) {
        case Json::Kind::Null: out += "null"; break;
        case Json::Kind::Bool: out += value.boolean() ? "true" : "false"; break;
        case Json::Kind::Number: {
            char buf[32];
            const double n = value.num();
            if (std::floor(n) == n && std::fabs(n) < 9.0e15) {
                std::snprintf(buf, sizeof(buf), "%.0f", n);
            } else {
                std::snprintf(buf, sizeof(buf), "%.9g", n);
            }
            out += buf;
            break;
        }
        case Json::Kind::String: DumpString(value.str(), out); break;
        case Json::Kind::Array: {
            out += '[';
            bool first = true;
            for (const auto& item : value.array()) {
                if (!first) out += ',';
                first = false;
                DumpValue(item, out);
            }
            out += ']';
            break;
        }
        case Json::Kind::Object: {
            out += '{';
            bool first = true;
            for (const auto& [key, item] : value.object()) {
                if (!first) out += ',';
                first = false;
                DumpString(key, out);
                out += ':';
                DumpValue(item, out);
            }
            out += '}';
            break;
        }
    }
}

} // namespace

Json Json::parse(const std::string& text, int* errorPos) {
    if (errorPos)
        *errorPos = kNoError;
    Parser parser(text, errorPos);
    // Tolerancia al BOM UTF-8 (algunas herramientas lo escriben al principio del archivo).
    if (text.size() >= 3 && static_cast<unsigned char>(text[0]) == 0xEF &&
        static_cast<unsigned char>(text[1]) == 0xBB && static_cast<unsigned char>(text[2]) == 0xBF)
        parser.p += 3;
    Json value;
    if (!parser.ParseValue(value))
        return Json();
    return value;
}

std::string Json::dump() const {
    std::string out;
    DumpValue(*this, out);
    return out;
}

} // namespace sfemu
