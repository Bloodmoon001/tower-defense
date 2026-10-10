#include "td/Config.h"
#include "json/Parser.h"
#include "json/Value.h"

#include <fstream>
#include <sstream>
#include <iostream>

namespace td {

    namespace {

        // Безопасно достать число из объекта.
        float getFloat(const json::Value& obj, const std::string& key, float def) {
            if (!obj.isObject()) return def;
            auto it = obj.asObject().find(key);
            if (it == obj.asObject().end()) return def;
            if (!it->second.isNumber()) return def;
            return static_cast<float>(it->second.asNumber());
        }

        int getInt(const json::Value& obj, const std::string& key, int def) {
            return static_cast<int>(getFloat(obj, key, static_cast<float>(def)));
        }

    } // namespace

    Config Config::LoadFromFile(const std::string& path) {
        Config cfg;   // все значения по умолчанию

        std::ifstream in(path);
        if (!in.is_open()) {
            std::cerr << "[Config] File not found: " << path
                << " - using defaults.\n";
            return cfg;
        }

        std::stringstream ss;
        ss << in.rdbuf();
        std::string content = ss.str();

        try {
            json::Value root = json::Parser::Parse(content);

            // --- grid ---
            if (root.isObject()) {
                auto obj = root.asObject();
                auto find = [&](const std::string& k) -> const json::Value* {
                    auto it = obj.find(k);
                    return it == obj.end() ? nullptr : &it->second;
                    };

                if (auto* g = find("grid")) {
                    cfg.gridWidth = getInt(*g, "width", cfg.gridWidth);
                    cfg.gridHeight = getInt(*g, "height", cfg.gridHeight);
                    cfg.cellSize = getFloat(*g, "cellSize", cfg.cellSize);
                }

                if (auto* p = find("player")) {
                    cfg.startingGold = getInt(*p, "startingGold", cfg.startingGold);
                    cfg.startingHp = getInt(*p, "startingHp", cfg.startingHp);
                }

                if (auto* e = find("enemies")) {
                    if (auto* n = find("normal")) {
                        // not reached — placeholder
                    }
                    // Пока не усложняем: если структура есть, читаем под ней
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "[Config] JSON parse error: " << e.what()
                << " - using defaults.\n";
        }

        return cfg;
    }

} // namespace td