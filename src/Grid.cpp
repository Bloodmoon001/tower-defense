#include "td/Grid.h"
#include <stdexcept>
#include <sstream>

namespace td {

    Grid::Grid(int width, int height)
        : width_(width),
        height_(height),
        cells_(static_cast<std::size_t>(width)* static_cast<std::size_t>(height),
            CellType::Empty)
    {
        if (width <= 0 || height <= 0) {
            throw std::invalid_argument("Grid dimensions must be positive");
        }
    }

    CellType Grid::getCell(Vector2i p) const {
        if (!inBounds(p)) {
            throw std::out_of_range("Grid::getCell: out of bounds " + p.toString());
        }
        return cells_[static_cast<std::size_t>(encode(p))];
    }

    void Grid::setCell(Vector2i p, CellType type) {
        if (!inBounds(p)) {
            throw std::out_of_range("Grid::setCell: out of bounds " + p.toString());
        }
        cells_[static_cast<std::size_t>(encode(p))] = type;

        if (type == CellType::Spawn) spawn_ = p;
        if (type == CellType::Goal)  goal_ = p;
    }

    Grid Grid::FromString(const std::string& ascii) {
        // Split into lines
        std::vector<std::string> lines;
        std::istringstream stream(ascii);
        std::string line;
        while (std::getline(stream, line)) {
            // Trim trailing \r (Windows)
            while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) {
                line.pop_back();
            }
            if (!line.empty()) lines.push_back(line);
        }

        if (lines.empty()) {
            throw std::invalid_argument("Grid::FromString: empty input");
        }

        int h = static_cast<int>(lines.size());
        int w = 0;
        for (const auto& l : lines) {
            w = std::max(w, static_cast<int>(l.size()));
        }

        Grid grid(w, h);

        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                char c = (x < static_cast<int>(lines[y].size())) ? lines[y][x] : '.';
                Vector2i p{ x, y };
                switch (c) {
                case '#': grid.setCell(p, CellType::Blocked); break;
                case 'S': grid.setCell(p, CellType::Spawn);   break;
                case 'G': grid.setCell(p, CellType::Goal);    break;
                case '.':
                default:  grid.setCell(p, CellType::Empty);   break;
                }
            }
        }

        return grid;
    }

    Grid Grid::CreateDefault(int width, int height) {
        Grid grid(width, height);

        // Place spawn on the left middle, goal on the right middle.
        int midY = height / 2;
        grid.spawn_ = { 0, midY };
        grid.setCell(grid.spawn_, CellType::Spawn);

        grid.goal_ = { width - 1, midY };
        grid.setCell(grid.goal_, CellType::Goal);

        // Add a few obstacles to make the map interesting.
        // We deliberately leave gaps so a path exists.
        for (int x = width / 4; x < width / 4 + 1; ++x) {
            for (int y = 0; y < height; ++y) {
                if (y != midY - 1) {
                    grid.setCell({ x, y }, CellType::Blocked);
                }
            }
        }
        for (int x = width / 2; x < width / 2 + 1; ++x) {
            for (int y = 0; y < height; ++y) {
                if (y != midY + 1) {
                    grid.setCell({ x, y }, CellType::Blocked);
                }
            }
        }
        for (int x = (3 * width) / 4; x < (3 * width) / 4 + 1; ++x) {
            for (int y = 0; y < height; ++y) {
                if (y != midY - 1) {
                    grid.setCell({ x, y }, CellType::Blocked);
                }
            }
        }

        return grid;
    }

    std::string Grid::toString() const {
        std::string result;
        result.reserve(static_cast<std::size_t>((width_ + 1) * height_));
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                switch (cells_[static_cast<std::size_t>(encode({ x, y }))]) {
                case CellType::Empty:   result += '.'; break;
                case CellType::Blocked: result += '#'; break;
                case CellType::Spawn:   result += 'S'; break;
                case CellType::Goal:    result += 'G'; break;
                }
            }
            result += '\n';
        }
        return result;
    }

} // namespace td