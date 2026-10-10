#pragma once

#include "td/Vector2.h"
#include <vector>
#include <string>
#include <optional>

namespace td {

    // Cell type on the grid.
    enum class CellType : std::uint8_t {
        Empty = 0,   // walkable, can build towers
        Blocked = 1,   // wall / obstacle, not walkable
        Spawn = 2,   // walkable, enemy entry point
        Goal = 3    // walkable, enemy exit point (base)
    };

    class Grid {
    public:
        Grid(int width, int height);

        // Build a grid from ASCII art:
        //   '.' = empty, '#' = blocked, 'S' = spawn, 'G' = goal
        // First 'S' and first 'G' encountered become the spawn/goal cells.
        static Grid FromString(const std::string& ascii);

        // Build an empty grid with a default path from left to right
        // (blocked columns are placed to force a winding path).
        static Grid CreateDefault(int width, int height);

        int width()  const noexcept { return width_; }
        int height() const noexcept { return height_; }

        bool inBounds(Vector2i p) const noexcept {
            return p.x >= 0 && p.y >= 0 && p.x < width_ && p.y < height_;
        }

        CellType getCell(Vector2i p) const;
        void     setCell(Vector2i p, CellType type);

        bool isWalkable(Vector2i p) const {
            return inBounds(p) && getCell(p) != CellType::Blocked;
        }

        bool isBuildable(Vector2i p) const {
            return inBounds(p) && getCell(p) == CellType::Empty;
        }

        Vector2i spawn() const noexcept { return spawn_; }
        Vector2i goal()  const noexcept { return goal_; }

        // Convert cell index (0..width*height-1) to coordinates.
        Vector2i decode(int index) const noexcept {
            return { index % width_, index / width_ };
        }

        int encode(Vector2i p) const noexcept {
            return p.y * width_ + p.x;
        }

        // Debug helper: render grid back to ASCII.
        std::string toString() const;

    private:
        int width_;
        int height_;
        std::vector<CellType> cells_;
        Vector2i spawn_{ 0, 0 };
        Vector2i goal_{ 0, 0 };
    };

} // namespace td