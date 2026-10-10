#pragma once

// Mini-JSON: suficiente para leer y escribir la configuración y el archivo de stats
// del emulador. Sin dependencias externas, tolerante a errores (devuelve vacío/por defecto).

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace sfemu {

class Json {
public:
    enum class Kind { Null, Bool, Number, String, Array, Object };
    static constexpr int kNoError = 0;

    Json() = default;
    explicit Json(std::string value);
    explicit Json(double value);
    explicit Json(bool value);
    Json(const Json&) = default;
    Json(Json&&) noexcept = default;
    Json& operator=(const Json&) = default;
    Json& operator=(Json&&) noexcept = default;

    Kind kind() const { return kind_; }
    bool isNull() const { return kind_ == Kind::Null; }
    const std::string& str() const { return str_; }
    double num() const { return num_; }
    bool boolean() const { return boolean_; }
    const std::vector<Json>& array() const { return array_; }
    const std::map<std::string, Json>& object() const { return object_; }

    const Json* find(const std::string& key) const;
    Json& at(const std::string& key);

    void push(Json value);
    void set(const std::string& key, Json value);

    static Json parse(const std::string& text, int* errorPos = nullptr);
    std::string dump() const;

private:
    Kind kind_ = Kind::Null;
    std::string str_;
    double num_ = 0;
    bool boolean_ = false;
    std::vector<Json> array_;
    std::map<std::string, Json> object_;
};

} // namespace sfemu
