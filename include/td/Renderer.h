#pragma once

#include "td/Vector2.h"
#include <SFML/Graphics.hpp>
#include <string>

namespace td {

    // Обёртка над SFML: рисует всё, что нужно игре.
    // Не хранит игровое состояние — только визуальные ресурсы.
    class Renderer {
    public:
        Renderer(unsigned int windowWidth, unsigned int windowHeight, float cellSize);

        sf::RenderWindow& window() noexcept { return window_; }
        bool isOpen() const noexcept { return window_.isOpen(); }

        // Управление окном
        void pollEvents();                       // обрабатывает закрытие окна
        void clear(const sf::Color& color = sf::Color(30, 30, 30));
        void display();

        // Примитивы рисования (в пиксельных координатах экрана)
        void drawRect(float x, float y, float w, float h,
            const sf::Color& fill,
            const sf::Color& outline = sf::Color::Transparent,
            float outlineThickness = 0.0f);

        void drawCircle(float x, float y, float radius,
            const sf::Color& fill,
            const sf::Color& outline = sf::Color::Transparent);

        void drawLine(float x1, float y1, float x2, float y2,
            const sf::Color& color, float thickness = 1.0f);

        void drawText(const std::string& text, float x, float y,
            unsigned int size, const sf::Color& color);

        // Специализированные хелперы
        void drawGrid(int width, int height, float cellSize,
            const sf::Color& color);

        void drawHealthBar(float x, float y, float w, float h,
            float fraction,
            const sf::Color& bg, const sf::Color& fg);

        // Возвращает координаты мыши в пикселях окна
        Vector2 mousePosition() const;

        // Размер окна
        float windowWidth()  const noexcept { return static_cast<float>(window_.getSize().x); }
        float windowHeight() const noexcept { return static_cast<float>(window_.getSize().y); }

    private:
        sf::RenderWindow window_;
        sf::Font         font_;
        bool             fontLoaded_ = false;
    };

} // namespace td