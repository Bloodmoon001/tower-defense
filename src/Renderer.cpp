#include "td/Renderer.h"
#include <iostream>

namespace td {

    Renderer::Renderer(unsigned int windowWidth, unsigned int windowHeight, float cellSize)
        : window_(sf::VideoMode({ windowWidth, windowHeight }),
            "Tower Defense",
            sf::Style::Titlebar | sf::Style::Close)
    {
        (void)cellSize;
        window_.setFramerateLimit(60);

        // Пробуем загрузить системный шрифт Windows
        const char* paths[] = {
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/segoeui.ttf",
            "C:/Windows/Fonts/calibri.ttf",
        };
        for (const char* p : paths) {
            if (font_.openFromFile(p)) {
                fontLoaded_ = true;
                break;
            }
        }
        if (!fontLoaded_) {
            std::cerr << "[Renderer] Could not load system font, text will be invisible.\n";
        }
    }

    void Renderer::pollEvents() {
        while (const std::optional event = window_.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window_.close();
            }
        }
    }

    void Renderer::clear(const sf::Color& color) {
        window_.clear(color);
    }

    void Renderer::display() {
        window_.display();
    }

    void Renderer::drawRect(float x, float y, float w, float h,
        const sf::Color& fill,
        const sf::Color& outline,
        float outlineThickness)
    {
        sf::RectangleShape rect({ w, h });
        rect.setPosition({ x, y });
        rect.setFillColor(fill);
        if (outlineThickness > 0.0f) {
            rect.setOutlineColor(outline);
            rect.setOutlineThickness(outlineThickness);
        }
        window_.draw(rect);
    }

    void Renderer::drawCircle(float x, float y, float radius,
        const sf::Color& fill,
        const sf::Color& outline)
    {
        sf::CircleShape circle(radius);
        circle.setPosition({ x - radius, y - radius });
        circle.setFillColor(fill);
        circle.setOutlineColor(outline);
        circle.setOutlineThickness(outline == sf::Color::Transparent ? 0.0f : 1.5f);
        window_.draw(circle);
    }

    void Renderer::drawLine(float x1, float y1, float x2, float y2,
        const sf::Color& color, float thickness)
    {
        // SFML 3: sf::Vertex — это aggregate. Инициализируем поля напрямую.
        sf::Vertex line[2];
        line[0].position = sf::Vector2f{ x1, y1 };
        line[0].color = color;
        line[1].position = sf::Vector2f{ x2, y2 };
        line[1].color = color;

        (void)thickness;   // SFML не поддерживает толщину линий напрямую

        window_.draw(line, 2, sf::PrimitiveType::Lines);
    }

    void Renderer::drawText(const std::string& text, float x, float y,
        unsigned int size, const sf::Color& color)
    {
        if (!fontLoaded_) return;

        sf::Text t(font_, text, size);
        t.setPosition({ x, y });
        t.setFillColor(color);
        window_.draw(t);
    }

    void Renderer::drawGrid(int width, int height, float cellSize,
        const sf::Color& color)
    {
        for (int i = 0; i <= width; ++i) {
            float x = i * cellSize;
            drawLine(x, 0.0f, x, height * cellSize, color);
        }
        for (int j = 0; j <= height; ++j) {
            float y = j * cellSize;
            drawLine(0.0f, y, width * cellSize, y, color);
        }
    }

    void Renderer::drawHealthBar(float x, float y, float w, float h,
        float fraction,
        const sf::Color& bg, const sf::Color& fg)
    {
        drawRect(x, y, w, h, bg);
        if (fraction > 0.0f) {
            drawRect(x, y, w * fraction, h, fg);
        }
    }

    Vector2 Renderer::mousePosition() const {
        sf::Vector2i p = sf::Mouse::getPosition(window_);
        return Vector2(static_cast<float>(p.x), static_cast<float>(p.y));
    }

} // namespace td