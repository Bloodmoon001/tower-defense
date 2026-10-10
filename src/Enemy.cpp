#include "td/Enemy.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace td {

    Enemy::Enemy(EnemyType type,
        std::vector<Vector2i> path,
        float cellSize,
        float hp,
        float speed,
        int   reward,
        int   damageToPlayer)
        : type_(type),
        path_(std::move(path)),
        cellSize_(cellSize),
        hp_(hp),
        maxHp_(hp),
        speed_(speed),
        reward_(reward),
        damage_(damageToPlayer)
    {
        if (path_.empty()) {
            throw std::invalid_argument("Enemy path cannot be empty");
        }
        position_ = path_.front().toWorld(cellSize_);
    }

    void Enemy::Update(float dt) {
        if (!isAlive() || reachedGoal_) return;

        // Update slow effect timer
        if (slowRemaining_ > 0.0f) {
            slowRemaining_ -= dt;
            if (slowRemaining_ <= 0.0f) {
                slowFactor_ = 1.0f;
            }
        }

        float effectiveSpeed = speed_ * currentSpeedMultiplier();
        float remainingMove = effectiveSpeed * dt;

        while (remainingMove > 0.0f && !reachedGoal_) {
            if (pathIndex_ + 1 >= path_.size()) {
                reachedGoal_ = true;
                break;
            }

            Vector2 target = path_[pathIndex_ + 1].toWorld(cellSize_);
            Vector2 toTarget = target - position_;
            float distToTarget = toTarget.length();

            if (distToTarget <= remainingMove) {
                position_ = target;
                remainingMove -= distToTarget;
                ++pathIndex_;

                // Если достигли последней клетки пути — финиш.
                if (pathIndex_ + 1 >= path_.size()) {
                    reachedGoal_ = true;
                    break;
                }
            }
            else {
                Vector2 direction = toTarget.normalized();
                position_ += direction * remainingMove;
                remainingMove = 0.0f;
            }
        }
    }

    bool Enemy::TakeDamage(float amount) {
        if (!isAlive()) return false;
        hp_ -= amount;
        return hp_ <= 0.0f;
    }

    void Enemy::ApplySlow(float factor, float duration) {
        if (factor < slowFactor_ || slowRemaining_ <= 0.0f) {
            slowFactor_ = factor;
        }
        slowRemaining_ = std::max(slowRemaining_, duration);
    }

    float Enemy::currentSpeedMultiplier() const noexcept {
        return slowRemaining_ > 0.0f ? slowFactor_ : 1.0f;
    }

    float Enemy::progress() const noexcept {
        if (path_.size() <= 1) return 1.0f;
        return static_cast<float>(pathIndex_) /
            static_cast<float>(path_.size() - 1);
    }

} // namespace td