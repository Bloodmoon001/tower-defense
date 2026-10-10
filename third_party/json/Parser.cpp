#include "json/Parser.h"
#include <cctype>
#include <cstdlib>

namespace json {

    // ============================================================
    // Вспомогательные методы
    // ============================================================

    Parser::Parser(const std::string& source) : src_(source) {}

    bool Parser::AtEnd() const {
        return pos_ >= src_.size();
    }

    char Parser::Peek() const {
        return AtEnd() ? '\0' : src_[pos_];
    }

    char Parser::Advance() {
        if (AtEnd()) Error("Unexpected end of input");
        return src_[pos_++];
    }

    bool Parser::Match(char expected) {
        if (Peek() == expected) { ++pos_; return true; }
        return false;
    }

    void Parser::SkipWhitespace() {
        while (!AtEnd()) {
            char c = Peek();
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') ++pos_;
            else break;
        }
    }

    [[noreturn]] void Parser::Error(const std::string& message) const {
        throw ParseError(message + " at position " + std::to_string(pos_), pos_);
    }

    // ============================================================
    // Точка входа
    // ============================================================

    Value Parser::Parse(const std::string& source) {
        Parser p(source);
        p.SkipWhitespace();
        Value v = p.ParseValue();
        p.SkipWhitespace();
        if (!p.AtEnd()) {
            p.Error("Unexpected trailing characters");
        }
        return v;
    }

    // ============================================================
    // ParseValue
    // ============================================================

    Value Parser::ParseValue() {
        SkipWhitespace();
        if (AtEnd()) Error("Unexpected end of input");

        char c = Peek();
        switch (c) {
        case 'n': return ParseNull();
        case 't':
        case 'f': return ParseBool();
        case '"': return ParseString();
        case '[': return ParseArray();
        case '{': return ParseObject();
        default:
            if (c == '-' || (c >= '0' && c <= '9')) return ParseNumber();
            Error(std::string("Unexpected character '") + c + "'");
        }
    }

    // ============================================================
    // Примитивы
    // ============================================================

    Value Parser::ParseNull() {
        if (src_.compare(pos_, 4, "null") == 0) {
            pos_ += 4;
            return Value(nullptr);
        }
        Error("Expected 'null'");
    }

    Value Parser::ParseBool() {
        if (src_.compare(pos_, 4, "true") == 0) {
            pos_ += 4;
            return Value(true);
        }
        if (src_.compare(pos_, 5, "false") == 0) {
            pos_ += 5;
            return Value(false);
        }
        Error("Expected 'true' or 'false'");
    }

    Value Parser::ParseNumber() {
        std::size_t start = pos_;

        if (Peek() == '-') ++pos_;

        if (AtEnd()) Error("Incomplete number");
        if (Peek() == '0') {
            ++pos_;
        }
        else if (Peek() >= '1' && Peek() <= '9') {
            while (!AtEnd() && std::isdigit(static_cast<unsigned char>(Peek()))) ++pos_;
        }
        else {
            Error("Invalid number: expected digit");
        }

        if (Peek() == '.') {
            ++pos_;
            if (AtEnd() || !std::isdigit(static_cast<unsigned char>(Peek())))
                Error("Invalid number: expected digit after '.'");
            while (!AtEnd() && std::isdigit(static_cast<unsigned char>(Peek()))) ++pos_;
        }

        if (Peek() == 'e' || Peek() == 'E') {
            ++pos_;
            if (Peek() == '+' || Peek() == '-') ++pos_;
            if (AtEnd() || !std::isdigit(static_cast<unsigned char>(Peek())))
                Error("Invalid number: expected digit in exponent");
            while (!AtEnd() && std::isdigit(static_cast<unsigned char>(Peek()))) ++pos_;
        }

        std::string numStr = src_.substr(start, pos_ - start);
        char* end = nullptr;
        double value = std::strtod(numStr.c_str(), &end);
        if (end == nullptr || *end != '\0') {
            pos_ = start;
            Error("Invalid number: " + numStr);
        }
        return Value(value);
    }

    // ============================================================
    // ParseString
    // ============================================================

    Value Parser::ParseString() {
        if (!Match('"')) Error("Expected '\"'");

        std::string result;

        while (true) {
            if (AtEnd()) Error("Unterminated string");

            char c = Advance();

            if (c == '"') break;

            if (c == '\\') {
                if (AtEnd()) Error("Unterminated escape sequence");
                char esc = Advance();
                switch (esc) {
                case '"':  result += '"';  break;
                case '\\': result += '\\'; break;
                case '/':  result += '/';  break;
                case 'b':  result += '\b'; break;
                case 'f':  result += '\f'; break;
                case 'n':  result += '\n'; break;
                case 'r':  result += '\r'; break;
                case 't':  result += '\t'; break;
                case 'u': {
                    if (pos_ + 4 > src_.size()) Error("Invalid \\u escape");
                    unsigned int codepoint = 0;
                    for (int i = 0; i < 4; ++i) {
                        char h = src_[pos_++];
                        codepoint <<= 4;
                        if (h >= '0' && h <= '9')      codepoint |= (h - '0');
                        else if (h >= 'a' && h <= 'f') codepoint |= (h - 'a' + 10);
                        else if (h >= 'A' && h <= 'F') codepoint |= (h - 'A' + 10);
                        else Error("Invalid hex digit in \\u escape");
                    }
                    if (codepoint < 0x80) {
                        result += static_cast<char>(codepoint);
                    }
                    else if (codepoint < 0x800) {
                        result += static_cast<char>(0xC0 | (codepoint >> 6));
                        result += static_cast<char>(0x80 | (codepoint & 0x3F));
                    }
                    else {
                        result += static_cast<char>(0xE0 | (codepoint >> 12));
                        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                        result += static_cast<char>(0x80 | (codepoint & 0x3F));
                    }
                    break;
                }
                default:
                    Error(std::string("Invalid escape character '\\") + esc + "'");
                }
            }
            else if (static_cast<unsigned char>(c) < 0x20) {
                Error("Unescaped control character in string");
            }
            else {
                result += c;
            }
        }

        return Value(std::move(result));
    }

    // ============================================================
    // ParseArray
    // ============================================================

    Value Parser::ParseArray() {
        if (!Match('[')) Error("Expected '['");

        Value::Array arr;
        SkipWhitespace();

        if (Match(']')) return Value(std::move(arr));

        while (true) {
            SkipWhitespace();
            arr.push_back(ParseValue());
            SkipWhitespace();

            if (Match(']')) break;
            if (!Match(',')) Error("Expected ',' or ']' in array");
        }

        return Value(std::move(arr));
    }

    // ============================================================
    // ParseObject
    // ============================================================

    Value Parser::ParseObject() {
        if (!Match('{')) Error("Expected '{'");

        Value::Object obj;
        SkipWhitespace();

        if (Match('}')) return Value(std::move(obj));

        while (true) {
            SkipWhitespace();

            if (Peek() != '"') Error("Expected string key in object");
            Value keyValue = ParseString();
            std::string key = keyValue.asString();

            SkipWhitespace();
            if (!Match(':')) Error("Expected ':' after object key");

            SkipWhitespace();
            Value value = ParseValue();

            obj[key] = std::move(value);

            SkipWhitespace();
            if (Match('}')) break;
            if (!Match(',')) Error("Expected ',' or '}' in object");
        }

        return Value(std::move(obj));
    }

} // namespace json