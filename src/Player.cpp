#include "td/Player.h"

namespace td {

    Player::Player(int startingGold, int startingHp) noexcept
        : gold_(startingGold),
        hp_(startingHp)
    {
    }

} // namespace td