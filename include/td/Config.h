#pragma once

#include <string>

namespace td {

    // Конфигурация игры, загружаемая из JSON.
    // Все параметры баланса можно менять без перекомпиляции.
    struct Config {
        // --- Grid ---
        int   gridWidth = 20;
        int   gridHeight = 15;
        float cellSize = 40.0f;

        // --- Player ---
        int startingGold = 200;
        int startingHp = 20;

        // --- Enemy defaults ---
        float enemyNormalHp = 40.0f;
        float enemyNormalSpeed = 60.0f;
        int   enemyNormalReward = 10;
        int   enemyNormalDamage = 1;

        float enemyFastHp = 25.0f;
        float enemyFastSpeed = 130.0f;
        int   enemyFastReward = 12;
        int   enemyFastDamage = 1;

        float enemyTankHp = 150.0f;
        float enemyTankSpeed = 35.0f;
        int   enemyTankReward = 30;
        int   enemyTankDamage = 3;

        // Загрузить из JSON-файла. Если файл недоступен — вернуть значения по умолчанию.
        static Config LoadFromFile(const std::string& path);

        // Значения по умолчанию (не читает файл).
        static Config Default() { return {}; }
    };

} // namespace td