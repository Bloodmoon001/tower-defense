#include "json/Serializer.h"
#include <cstdio>
#include <cmath>

namespace json {

    Serializer::Serializer(bool compact) : compact_(compact) {}

    std::string Serializer::ToString(const Value& value, bool compact) {
        Serializer s(compact);
        s.WriteValue(value, 0);
        return std::move(s.out_);
    }

    void Serializer::WriteIndent(int depth) {
        if (compact_) return;
        out_ += '\n';
        for (int i = 0; i < depth; ++i) out_ += "  ";
    }

    void Serializer::WriteString(const std::string& s) {
        out_ += '"';
        for (char c : s) {
            switch (c) {
            case '"':  out_ += "\\\""; break;
            case '\\': out_ += "\\\\"; break;
            case '\b': out_ += "\\b";  break;
            case '\f': out_ += "\\f";  break;
            case '\n': out_ += "\\n";  break;
            case '\r': out_ += "\\r";  break;
            case '\t': out_ += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x",
                        static_cast<unsigned char>(c));
                    out_ += buf;
                }
                else {
                    out_ += c;
                }
            }
        }
        out_ += '"';
    }

    void Serializer::WriteValue(const Value& value, int depth) {
        switch (value.type()) {
        case Value::Type::Null:
            out_ += "null";
            break;

        case Value::Type::Bool:
            out_ += value.asBool() ? "true" : "false";
            break;

        case Value::Type::Number: {
            double d = value.asNumber();
            if (std::isfinite(d) && d == static_cast<double>(static_cast<long long>(d))
                && std::abs(d) < 1e15) {
                out_ += std::to_string(static_cast<long long>(d));
            }
            else {
                char buf[32];
                std::snprintf(buf, sizeof(buf), "%.17g", d);
                out_ += buf;
            }
            break;
        }

        case Value::Type::String:
            WriteString(value.asString());
            break;

        case Value::Type::Array: {
            const auto& arr = value.asArray();
            if (arr.empty()) { out_ += "[]"; break; }

            out_ += '[';
            for (std::size_t i = 0; i < arr.size(); ++i) {
                if (i > 0) out_ += ',';
                WriteIndent(depth + 1);
                WriteValue(arr[i], depth + 1);
            }
            WriteIndent(depth);
            out_ += ']';
            break;
        }

        case Value::Type::Object: {
            const auto& obj = value.asObject();
            if (obj.empty()) { out_ += "{}"; break; }

            out_ += '{';
            bool first = true;
            for (const auto& [key, val] : obj) {
                if (!first) out_ += ',';
                first = false;
                WriteIndent(depth + 1);
                WriteString(key);
                out_ += ':';
                if (!compact_) out_ += ' ';
                WriteValue(val, depth + 1);
            }
            WriteIndent(depth);
            out_ += '}';
            break;
        }
        }
    }

} // namespace json