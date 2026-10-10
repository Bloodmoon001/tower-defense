#pragma once

#include <variant>
#include <string>
#include <vector>
#include <map>
#include <cstddef>

namespace json {

    class Value {
    public:
        using Array = std::vector<Value>;
        using Object = std::map<std::string, Value>;

        enum class Type {
            Null,
            Bool,
            Number,
            String,
            Array,
            Object
        };

        // --- Конструкторы ---
        Value() noexcept;                                // null
        Value(std::nullptr_t) noexcept;                  // null
        Value(bool b);                                   // bool
        Value(double d);                                 // number
        Value(int i);                                    // number (int -> double)
        Value(const char* s);                            // string
        Value(std::string s);                            // string
        Value(Array a);                                  // array
        Value(Object o);                                 // object

        // --- Специальные члены (компилятор сгенерирует сам) ---
        Value(const Value&) = default;
        Value(Value&&) noexcept = default;
        Value& operator=(const Value&) = default;
        Value& operator=(Value&&) noexcept = default;
        ~Value() = default;

        // --- Проверки типа ---
        Type type()     const noexcept;
        bool isNull()   const noexcept;
        bool isBool()   const noexcept;
        bool isNumber() const noexcept;
        bool isString() const noexcept;
        bool isArray()  const noexcept;
        bool isObject() const noexcept;

        // --- Доступ к значению (бросает std::runtime_error при неверном типе) ---
        bool               asBool()   const;
        double             asNumber() const;
        const std::string& asString() const;

        const Array& asArray()  const;
        Array& asArray();
        const Object& asObject() const;
        Object& asObject();

        // --- Размер для массивов, объектов и строк ---
        std::size_t size() const;

        // --- Сравнение ---
        bool operator==(const Value& other) const;
        bool operator!=(const Value& other) const { return !(*this == other); }

        // --- Человекочитаемое имя типа (для сообщений об ошибках) ---
        static const char* typeName(Type t) noexcept;

    private:
        std::variant<std::nullptr_t, bool, double, std::string, Array, Object> data_;
    };

} // namespace json