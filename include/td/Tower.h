#pragma once

#include "td/Vector2.h"
#include "td/Enemy.h"
#include <vector>

namespace td {

    enum class TowerType : std::uint8_t {
        Arrow = 0,   // fast, single-target, low damage
        Cannon = 1,   // slow, AoE damage
        Frost = 2    // slows enemies in small radius
    };

    class Tower {
    public:
        Tower(TowerType type, Vector2i cell, float cellSize);

        // Update cooldown. Returns true when the tower is ready to fire.
        void Update(float dt);

        // Pick a target from the list (closest-to-goal enemy within range).
        // Returns nullptr if no target is available.
        Enemy* AcquireTarget(const std::vector<Enemy>& enemies) const;

        // --- Getters ---
        TowerType type()  const noexcept { return type_; }
        Vector2i  cell()  const noexcept { return cell_; }
        Vector2   position() const noexcept { return position_; }
        float     range() const noexcept { return range_; }
        float     damage() const noexcept { return damage_; }
        float     fireCooldown() const noexcept { return fireCooldown_; }
        bool      canFire() const noexcept { return cooldown_ <= 0.0f; }

        // Stats for projectiles spawned by this tower.
        float projectileSpeed() const noexcept { return projectileSpeed_; }
        float aoeRadius()      const noexcept { return aoeRadius_; }
        bool  hasSlowEffect()  const noexcept { return type_ == TowerType::Frost; }
        float slowFactor()     const noexcept { return slowFactor_; }
        float slowDuration()   const noexcept { return slowDuration_; }

        // Reset cooldown after firing.
        void OnFired() noexcept { cooldown_ = fireCooldown_; }

        // Upgrade: +damage, +range (for UI/JSON features).
        void Upgrade() noexcept { level_++; damage_ *= 1.3f; range_ += 8.0f; }
        int  level() const noexcept { return level_; }

        // Build cost (used by UI).
        static int CostFor(TowerType type) noexcept;

    private:
        TowerType type_;
        Vector2i  cell_;
        Vector2   position_;
        int       level_ = 1;

        float range_;
        float damage_;
        float fireCooldown_;
        float cooldown_ = 0.0f;

        float projectileSpeed_;
        float aoeRadius_;
        float slowFactor_ = 0.5f;
        float slowDuration_ = 2.0f;
    };

} // namespace td