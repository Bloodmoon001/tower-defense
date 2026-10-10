#pragma once

#include "json/Value.h"
#include <stdexcept>
#include <string>
#include <cstddef>

namespace json {

    // Исключение, которое бросает парсер. Содержит позицию в исходной строке.
    class ParseError : public std::runtime_error {
    public:
        ParseError(const std::string& message, std::size_t position)
            : std::runtime_error(message), position_(position) {}

        std::size_t position() const noexcept { return position_; }

    private:
        std::size_t position_;
    };

    class Parser {
    public:
        // Единственная публичная функция: строка → Value.
        // Бросает ParseError при ошибке.
        static Value Parse(const std::string& source);

    private:
        explicit Parser(const std::string& source);

        // Основной рекурсивный спуск
        Value ParseValue();
        Value ParseNull();
        Value ParseBool();
        Value ParseNumber();
        Value ParseString();
        Value ParseArray();
        Value ParseObject();

        // Вспомогательные
        void  SkipWhitespace();
        bool  AtEnd() const;
        char  Peek() const;
        char  Advance();
        bool  Match(char expected);
        [[noreturn]] void Error(const std::string& message) const;

        const std::string& src_;
        std::size_t pos_ = 0;
    };

} // namespace json