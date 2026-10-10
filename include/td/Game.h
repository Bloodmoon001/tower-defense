#pragma once

#include "td/Config.h"
#include "td/Grid.h"
#include "td/Renderer.h"
#include "td/Enemy.h"
#include "td/Tower.h"
#include "td/Projectile.h"
#include "td/Player.h"
#include "td/Wave.h"

#include <vector>
#include <optional>

namespace td {

    enum class GameState {
        Playing,
        Paused,
        Victory,
        Defeat
    };

    class Game {
    public:
        Game(const Config& config);
        ~Game() = default;

        Game(const Game&) = delete;
        Game& operator=(const Game&) = delete;

        // Главный цикл. Возвращает false, когда игрок закрыл окно.
        bool Run();

    private:
        // --- Update ---
        void Update(float dt);
        void UpdateEnemies(float dt);
        void UpdateTowers(float dt);
        void UpdateProjectiles(float dt);
        void UpdateWaves(float dt);
        void SpawnEnemy(EnemyType type);

        // --- Input ---
        void HandleInput();
        void HandleMouseClick(int pixelX, int pixelY);

        // --- Helpers ---
        std::optional<Vector2i> PixelToCell(int px, int py) const;
        void TryBuildTower(Vector2i cell);
        void ApplyDamage(const Projectile& p, const Vector2& hitPos);
        bool PathStillExists(Vector2i blockedCell) const;
        void RenderGameOver();

        // --- Render ---
        void Render();
        void RenderGrid();
        void RenderEnemies();
        void RenderTowers();
        void RenderProjectiles();
        void RenderUI();

        // --- State ---
        Config    config_;
        Grid      grid_;
        Renderer  renderer_;
        Player    player_;
        WaveManager waves_;
        GameState state_ = GameState::Playing;

        std::vector<Vector2i> enemyPath_;      // общий маршрут для всех врагов
        std::vector<Enemy>     enemies_;
        std::vector<Tower>     towers_;
        std::vector<Projectile> projectiles_;

        // Какой тип башни выбран в UI (Arrow по умолчанию)
        TowerType selectedTowerType_ = TowerType::Arrow;

        // Для UI-подсветки клетки под мышью
        std::optional<Vector2i> hoverCell_;

        // Пауза между волнами
        float wavePauseTimer_ = 0.0f;
        bool  waitingForNextWave_ = true;

        // Счётчики для рендера
        int killedThisWave_ = 0;
    };

} // namespace td