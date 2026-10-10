#include "td/Projectile.h"
#include <cmath>

namespace td {

    Projectile::Projectile(Vector2 start,
        Enemy* target,
        float speed,
        float damage,
        float aoeRadius,
        bool  slowEffect,
        float slowFactor,
        float slowDuration)
        : position_(start),
        target_(target),
        speed_(speed),
        damage_(damage),
        aoeRadius_(aoeRadius),
        slowEffect_(slowEffect),
        slowFactor_(slowFactor),
        slowDuration_(slowDuration)
    {
    }

    bool Projectile::Update(float dt) {
        if (done_) return true;

        // Target died mid-flight → projectile is spent.
        if (target_ == nullptr || !target_->isAlive()) {
            done_ = true;
            return true;
        }

        Vector2 toTarget = target_->position() - position_;
        float dist = toTarget.length();
        float step = speed_ * dt;

        if (dist <= step) {
            // Reached target this frame.
            position_ = target_->position();
            done_ = true;
            return true;
        }

        position_ += toTarget.normalized() * step;
        return false;
    }

} // namespace td