#pragma once

#include <cstddef>

namespace td {

    class Player {
    public:
        Player(int startingGold, int startingHp) noexcept;

        // --- Gold ---
        int  gold() const noexcept { return gold_; }
        bool canAfford(int cost) const noexcept { return gold_ >= cost; }
        void spend(int cost) noexcept { gold_ -= cost; }
        void earn(int reward) noexcept { gold_ += reward; }

        // --- HP (lives) ---
        int  hp() const noexcept { return hp_; }
        bool isAlive() const noexcept { return hp_ > 0; }
        void takeDamage(int amount) noexcept {
            hp_ -= amount;
            if (hp_ < 0) hp_ = 0;
        }

        // --- Score ---
        int  score() const noexcept { return score_; }
        void addScore(int points) noexcept { score_ += points; }

    private:
        int gold_;
        int hp_;
        int score_ = 0;
    };

} // namespace td