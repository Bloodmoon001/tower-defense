#include "td/Pathfinder.h"

#include <algorithm>
#include <limits>
#include <queue>
#include <vector>

namespace td {

    namespace {
        // Min-heap entry: (f_score, encoded_position).
        struct Node {
            float f;
            int   pos;
            bool operator>(const Node& other) const { return f > other.f; }
        };

        constexpr int DX[4] = { 1, -1, 0,  0 };
        constexpr int DY[4] = { 0,  0, 1, -1 };
    }

    float Pathfinder::Heuristic(Vector2i a, Vector2i b) {
        return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
    }

    std::vector<Vector2i> Pathfinder::FindPath(const Grid& grid,
        Vector2i start,
        Vector2i goal) {
        // Sanity checks.
        if (!grid.inBounds(start) || !grid.inBounds(goal)) return {};
        if (!grid.isWalkable(start) || !grid.isWalkable(goal)) return {};
        if (start == goal) return { start };

        const int w = grid.width();
        const int h = grid.height();
        const int total = w * h;

        // Distance from start to each cell (infinity by default).
        std::vector<float> gScore(static_cast<std::size_t>(total),
            std::numeric_limits<float>::infinity());
        // came_from[cell] = previous cell index, or -1 for none.
        std::vector<int> cameFrom(static_cast<std::size_t>(total), -1);
        // Already fully processed cells.
        std::vector<bool> closed(static_cast<std::size_t>(total), false);

        const int startEnc = grid.encode(start);
        const int goalEnc = grid.encode(goal);

        gScore[static_cast<std::size_t>(startEnc)] = 0.0f;

        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
        open.push({ Heuristic(start, goal), startEnc });

        while (!open.empty()) {
            Node current = open.top();
            open.pop();

            if (closed[static_cast<std::size_t>(current.pos)]) continue;
            closed[static_cast<std::size_t>(current.pos)] = true;

            // Reached the goal → reconstruct path.
            if (current.pos == goalEnc) {
                std::vector<Vector2i> path;
                int node = goalEnc;
                while (node != -1) {
                    path.push_back(grid.decode(node));
                    node = cameFrom[static_cast<std::size_t>(node)];
                }
                std::reverse(path.begin(), path.end());
                return path;
            }

            Vector2i cell = grid.decode(current.pos);

            for (int dir = 0; dir < 4; ++dir) {
                Vector2i next{ cell.x + DX[dir], cell.y + DY[dir] };
                if (!grid.inBounds(next))       continue;
                if (!grid.isWalkable(next))     continue;

                int nextEnc = grid.encode(next);
                if (closed[static_cast<std::size_t>(nextEnc)]) continue;

                float tentativeG = gScore[static_cast<std::size_t>(current.pos)] + 1.0f;

                if (tentativeG < gScore[static_cast<std::size_t>(nextEnc)]) {
                    gScore[static_cast<std::size_t>(nextEnc)] = tentativeG;
                    cameFrom[static_cast<std::size_t>(nextEnc)] = current.pos;
                    float f = tentativeG + Heuristic(next, goal);
                    open.push({ f, nextEnc });
                }
            }
        }

        return {};  // no path found
    }

    std::vector<Vector2i> Pathfinder::FindPathToGoal(const Grid& grid,
        Vector2i start) {
        return FindPath(grid, start, grid.goal());
    }

} // namespace td