#pragma once

#include "json/Value.h"
#include <string>

namespace json {

    class Serializer {
    public:
        static std::string ToString(const Value& value, bool compact = true);

    private:
        explicit Serializer(bool compact);

        void WriteValue(const Value& value, int depth);
        void WriteString(const std::string& s);
        void WriteIndent(int depth);

        void AppendChar(char c) { out_ += c; }
        void AppendStr(const std::string& s) { out_ += s; }

        std::string out_;
        bool compact_;
    };

} // namespace json