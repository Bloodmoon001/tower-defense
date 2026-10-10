#pragma once

#include <cmath>
#include <cstdint>
#include <string>

namespace td {

    // 2D vector with float coordinates (world space).
    struct Vector2 {
        float x = 0.0f;
        float y = 0.0f;

        Vector2() = default;
        Vector2(float xv, float yv) : x(xv), y(yv) {}

        Vector2 operator+(const Vector2& o) const { return { x + o.x, y + o.y }; }
        Vector2 operator-(const Vector2& o) const { return { x - o.x, y - o.y }; }
        Vector2 operator*(float s) const { return { x * s, y * s }; }
        Vector2 operator/(float s) const { return { x / s, y / s }; }

        Vector2& operator+=(const Vector2& o) { x += o.x; y += o.y; return *this; }
        Vector2& operator-=(const Vector2& o) { x -= o.x; y -= o.y; return *this; }
        Vector2& operator*=(float s) { x *= s; y *= s; return *this; }

        bool operator==(const Vector2& o) const { return x == o.x && y == o.y; }
        bool operator!=(const Vector2& o) const { return !(*this == o); }

        float lengthSquared() const { return x * x + y * y; }
        float length() const { return std::sqrt(lengthSquared()); }

        Vector2 normalized() const {
            float len = length();
            if (len < 1e-6f) return { 0.0f, 0.0f };
            return { x / len, y / len };
        }

        static float distance(const Vector2& a, const Vector2& b) {
            return (b - a).length();
        }

        static float distanceSquared(const Vector2& a, const Vector2& b) {
            return (b - a).lengthSquared();
        }
    };

    // 2D integer vector for grid coordinates.
    struct Vector2i {
        int x = 0;
        int y = 0;

        Vector2i() = default;
        Vector2i(int xv, int yv) : x(xv), y(yv) {}

        bool operator==(const Vector2i& o) const { return x == o.x && y == o.y; }
        bool operator!=(const Vector2i& o) const { return !(*this == o); }

        Vector2i operator+(const Vector2i& o) const { return { x + o.x, y + o.y }; }
        Vector2i operator-(const Vector2i& o) const { return { x - o.x, y - o.y }; }

        std::string toString() const {
            return "(" + std::to_string(x) + "," + std::to_string(y) + ")";
        }

        // Convert grid cell to world position (center of the cell).
        Vector2 toWorld(float cellSize) const {
            return { x * cellSize + cellSize * 0.5f,
                    y * cellSize + cellSize * 0.5f };
        }
    };

} // namespace td