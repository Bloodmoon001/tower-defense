#pragma once

#include "td/Grid.h"
#include "td/Vector2.h"
#include <vector>

namespace td {

    class Pathfinder {
    public:
        // Find shortest path from `start` to `goal` on the grid using A*.
        // Returns a list of cells from start to goal inclusive.
        // Returns an empty vector if no path exists.
        static std::vector<Vector2i> FindPath(const Grid& grid,
            Vector2i start,
            Vector2i goal);

        // Same as FindPath but uses the grid's spawn/goal cells.
        static std::vector<Vector2i> FindPathToGoal(const Grid& grid,
            Vector2i start);

    private:
        // Manhattan distance heuristic (grid is 4-directional).
        static float Heuristic(Vector2i a, Vector2i b);
    };

} // namespace td