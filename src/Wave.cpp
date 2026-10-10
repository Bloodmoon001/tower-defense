#include "td/Wave.h"

namespace td {

    WaveManager::WaveManager() {
        // Прогрессия волн — 8 штук, сложность растёт.
        waves_ = {
            // wave 1: 5 обычных медленных
            { 5,  EnemyType::Normal, 0.8f, 1.0f },
            // wave 2: 8 обычных
            { 8,  EnemyType::Normal, 0.7f, 1.2f },
            // wave 3: добавляем быстрых
            { 6,  EnemyType::Normal, 0.6f, 1.4f },
            // wave 4: 5 быстрых
            { 5,  EnemyType::Fast,   0.4f, 1.5f },
            // wave 5: микс
            { 10, EnemyType::Normal, 0.6f, 1.8f },
            // wave 6: первый танк
            { 4,  EnemyType::Tank,   1.5f, 2.0f },
            // wave 7: смешанная
            { 8,  EnemyType::Fast,   0.4f, 2.2f },
            // wave 8: финальный микс
            { 12, EnemyType::Normal, 0.5f, 2.5f },
        };
    }

    bool WaveManager::StartNextWave() {
        if (allWavesDone()) return false;
        inProgress_ = true;
        spawnedThisWave_ = 0;
        spawnTimer_ = 0.0f;
        return true;
    }

    std::vector<EnemyType> WaveManager::Tick(float dt) {
        std::vector<EnemyType> readyToSpawn;

        if (!inProgress_) return readyToSpawn;

        const WaveDefinition& w = waves_[static_cast<std::size_t>(currentWave_)];

        spawnTimer_ -= dt;
        while (spawnTimer_ <= 0.0f && spawnedThisWave_ < w.count) {
            readyToSpawn.push_back(w.type);
            ++spawnedThisWave_;
            spawnTimer_ += w.spawnDelay;
        }

        // Волна закончилась — все враги выпущены.
        if (spawnedThisWave_ >= w.count) {
            inProgress_ = false;
            ++currentWave_;
        }

        return readyToSpawn;
    }

    float WaveManager::currentHpMultiplier() const noexcept {
        if (currentWave_ >= static_cast<int>(waves_.size())) return 1.0f;
        return waves_[static_cast<std::size_t>(currentWave_)].hpMultiplier;
    }

    int WaveManager::remainingInWave() const noexcept {
        if (!inProgress_ || currentWave_ >= static_cast<int>(waves_.size())) return 0;
        return waves_[static_cast<std::size_t>(currentWave_)].count - spawnedThisWave_;
    }

} // namespace td