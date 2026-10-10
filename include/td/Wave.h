#pragma once

#include "td/Enemy.h"
#include <vector>
#include <cstddef>

namespace td {

    // Описание одной волны: сколько врагов какого типа и с какими параметрами.
    struct WaveDefinition {
        int   count = 0;      // сколько врагов выпустить
        EnemyType type = EnemyType::Normal;
        float spawnDelay = 1.0f;   // секунды между спавнами
        float hpMultiplier = 1.0f;  // базовое HP множится на это
    };

    // Менеджер волн. Отвечает за то, когда и какого врага создать.
    class WaveManager {
    public:
        WaveManager();

        // Запустить следующую волну. Возвращает false, если волны закончились.
        bool StartNextWave();

        // Обновить таймеры. Возвращает список врагов, готовых к спавну в этом кадре.
        // (Создание идёт наружу — это ответственность Game.)
        std::vector<EnemyType> Tick(float dt);

        // --- Getters ---
        int  currentWave() const noexcept { return currentWave_; }
        int  totalWaves()  const noexcept { return static_cast<int>(waves_.size()); }
        bool allWavesDone() const noexcept { return currentWave_ >= totalWaves(); }
        bool waveInProgress() const noexcept { return inProgress_; }

        // HP множитель для текущей волны (для передачи в Enemy).
        float currentHpMultiplier() const noexcept;

        // Сколько врагов осталось выпустить в текущей волне.
        int remainingInWave() const noexcept;

    private:
        std::vector<WaveDefinition> waves_;
        int   currentWave_ = 0;       // индекс текущей волны (0 = ещё не начали)
        int   spawnedThisWave_ = 0;
        float spawnTimer_ = 0.0f;
        bool  inProgress_ = false;
    };

} // namespace td