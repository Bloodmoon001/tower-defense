#include <stdexcept>
#include "json/Value.h"

namespace json {

    // --- Конструкторы ---
    Value::Value() noexcept : data_(nullptr) {}
    Value::Value(std::nullptr_t) noexcept : data_(nullptr) {}
    Value::Value(bool b) : data_(b) {}
    Value::Value(double d) : data_(d) {}
    Value::Value(int i) : data_(static_cast<double>(i)) {}
    Value::Value(const char* s) : data_(std::string(s)) {}
    Value::Value(std::string s) : data_(std::move(s)) {}
    Value::Value(Array a) : data_(std::move(a)) {}
    Value::Value(Object o) : data_(std::move(o)) {}

    // --- Проверки типа ---
    Value::Type Value::type() const noexcept {
        switch (data_.index()) {
        case 0: return Type::Null;
        case 1: return Type::Bool;
        case 2: return Type::Number;
        case 3: return Type::String;
        case 4: return Type::Array;
        case 5: return Type::Object;
        }
        return Type::Null;
    }

    bool Value::isNull()   const noexcept { return std::holds_alternative<std::nullptr_t>(data_); }
    bool Value::isBool()   const noexcept { return std::holds_alternative<bool>(data_); }
    bool Value::isNumber() const noexcept { return std::holds_alternative<double>(data_); }
    bool Value::isString() const noexcept { return std::holds_alternative<std::string>(data_); }
    bool Value::isArray()  const noexcept { return std::holds_alternative<Array>(data_); }
    bool Value::isObject() const noexcept { return std::holds_alternative<Object>(data_); }

    // --- Доступ к значению ---
    bool Value::asBool() const {
        if (!isBool()) throw std::runtime_error("Value is not a bool");
        return std::get<bool>(data_);
    }

    double Value::asNumber() const {
        if (!isNumber()) throw std::runtime_error("Value is not a number");
        return std::get<double>(data_);
    }

    const std::string& Value::asString() const {
        if (!isString()) throw std::runtime_error("Value is not a string");
        return std::get<std::string>(data_);
    }

    const Value::Array& Value::asArray() const {
        if (!isArray()) throw std::runtime_error("Value is not an array");
        return std::get<Array>(data_);
    }

    Value::Array& Value::asArray() {
        if (!isArray()) throw std::runtime_error("Value is not an array");
        return std::get<Array>(data_);
    }

    const Value::Object& Value::asObject() const {
        if (!isObject()) throw std::runtime_error("Value is not an object");
        return std::get<Object>(data_);
    }

    Value::Object& Value::asObject() {
        if (!isObject()) throw std::runtime_error("Value is not an object");
        return std::get<Object>(data_);
    }

    // --- Размер ---
    std::size_t Value::size() const {
        if (isArray())  return std::get<Array>(data_).size();
        if (isObject()) return std::get<Object>(data_).size();
        if (isString()) return std::get<std::string>(data_).size();
        return 0;
    }

    // --- Сравнение ---
    bool Value::operator==(const Value& other) const {
        return data_ == other.data_;
    }

    // --- Имена типов ---
    const char* Value::typeName(Type t) noexcept {
        switch (t) {
        case Type::Null:   return "null";
        case Type::Bool:   return "bool";
        case Type::Number: return "number";
        case Type::String: return "string";
        case Type::Array:  return "array";
        case Type::Object: return "object";
        }
        return "unknown";
    }

} // namespace json