#include "td/Tower.h"
#include <algorithm>
#include <limits>

namespace td {

    Tower::Tower(TowerType type, Vector2i cell, float cellSize)
        : type_(type),
        cell_(cell)
    {
        position_ = cell_.toWorld(cellSize);

        // Per-type stats (Tower Defense balancing).
        switch (type_) {
        case TowerType::Arrow:
            range_ = 3.5f * cellSize;
            damage_ = 10.0f;
            fireCooldown_ = 0.4f;
            projectileSpeed_ = 500.0f;
            aoeRadius_ = 0.0f;
            break;
        case TowerType::Cannon:
            range_ = 4.0f * cellSize;
            damage_ = 40.0f;
            fireCooldown_ = 1.5f;
            projectileSpeed_ = 300.0f;
            aoeRadius_ = 1.2f * cellSize;
            break;
        case TowerType::Frost:
            range_ = 3.0f * cellSize;
            damage_ = 5.0f;
            fireCooldown_ = 0.8f;
            projectileSpeed_ = 600.0f;
            aoeRadius_ = 0.0f;
            slowFactor_ = 0.5f;
            slowDuration_ = 2.0f;
            break;
        }
    }

    void Tower::Update(float dt) {
        if (cooldown_ > 0.0f) {
            cooldown_ -= dt;
            if (cooldown_ < 0.0f) cooldown_ = 0.0f;
        }
    }

    Enemy* Tower::AcquireTarget(const std::vector<Enemy>& enemies) const {
        Enemy* best = nullptr;
        float bestProgress = -1.0f;

        for (const auto& e : enemies) {
            if (!e.isAlive() || e.reachedGoal()) continue;

            float distSq = Vector2::distanceSquared(position_, e.position());
            float rangeSq = range_ * range_;
            if (distSq > rangeSq) continue;

            // Prefer enemies closest to the goal (highest progress).
            float prog = e.progress();
            if (prog > bestProgress) {
                bestProgress = prog;
                best = const_cast<Enemy*>(&e);
            }
        }

        return best;
    }

    int Tower::CostFor(TowerType type) noexcept {
        switch (type) {
        case TowerType::Arrow:  return 50;
        case TowerType::Cannon: return 120;
        case TowerType::Frost:  return 80;
        }
        return 0;
    }

} // namespace td