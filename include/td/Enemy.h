#pragma once

#include "td/Vector2.h"
#include <vector>
#include <string>
#include <cstdint>

namespace td {

    enum class EnemyType : std::uint8_t {
        Normal = 0,
        Fast = 1,
        Tank = 2
    };

    class Enemy {
    public:
        Enemy(EnemyType type,
            std::vector<Vector2i> path,
            float cellSize,
            float hp,
            float speed,
            int   reward,
            int   damageToPlayer);

        void Update(float dt);
        bool TakeDamage(float amount);

        Vector2 position() const noexcept { return position_; }
        EnemyType type() const noexcept { return type_; }
        float hp()     const noexcept { return hp_; }
        float maxHp()  const noexcept { return maxHp_; }
        float speed()  const noexcept { return speed_; }
        int   reward() const noexcept { return reward_; }
        int   damageToPlayer() const noexcept { return damage_; }

        bool  isAlive() const noexcept { return hp_ > 0.0f; }
        bool  reachedGoal() const noexcept { return reachedGoal_; }

        void ApplySlow(float factor, float duration);
        float currentSpeedMultiplier() const noexcept;
        float progress() const noexcept;

    private:
        EnemyType type_;
        std::vector<Vector2i> path_;
        std::size_t pathIndex_ = 0;
        Vector2 position_;
        float cellSize_;

        float hp_;
        float maxHp_;
        float speed_;
        int   reward_;
        int   damage_;

        bool  reachedGoal_ = false;

        float slowFactor_ = 1.0f;
        float slowRemaining_ = 0.0f;
    };

} // namespace td