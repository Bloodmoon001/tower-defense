#pragma once

#include "td/Vector2.h"
#include "td/Enemy.h"
#include <memory>

namespace td {

    class Projectile {
    public:
        Projectile(Vector2 start,
            Enemy* target,
            float speed,
            float damage,
            float aoeRadius,
            bool  slowEffect,
            float slowFactor,
            float slowDuration);

        // Move toward the target. Returns true if the projectile is done
        // (either hit, or target is dead / null).
        bool Update(float dt);

        // --- Getters ---
        Vector2 position() const noexcept { return position_; }
        float damage()      const noexcept { return damage_; }
        float aoeRadius()   const noexcept { return aoeRadius_; }
        bool  hasSlowEffect() const noexcept { return slowEffect_; }
        float slowFactor()   const noexcept { return slowFactor_; }
        float slowDuration() const noexcept { return slowDuration_; }
        Enemy* target()      const noexcept { return target_; }

        bool isDone() const noexcept { return done_; }

    private:
        Vector2 position_;
        Enemy* target_ = nullptr;
        float   speed_;
        float   damage_;
        float   aoeRadius_;       // 0 = single target
        bool    slowEffect_;
        float   slowFactor_;
        float   slowDuration_;
        bool    done_ = false;
    };

} // namespace td