#include "td/Game.h"
#include "td/Pathfinder.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>

namespace td {

    // ============================================================
    // Цвета
    // ============================================================

    namespace {
        const sf::Color kBgColor(30, 30, 40);
        const sf::Color kGridColor(60, 60, 80);
        const sf::Color kPathColor(90, 80, 60);
        const sf::Color kBlockedColor(50, 50, 60);
        const sf::Color kSpawnColor(120, 60, 60);
        const sf::Color kGoalColor(60, 120, 60);
        const sf::Color kEnemyNormal(220, 80, 80);
        const sf::Color kEnemyFast(250, 200, 80);
        const sf::Color kEnemyTank(140, 60, 200);
        const sf::Color kTowerArrow(80, 160, 220);
        const sf::Color kTowerCannon(220, 140, 60);
        const sf::Color kTowerFrost(120, 220, 240);
        const sf::Color kProjectile(240, 240, 100);
        const sf::Color kHpBarBg(60, 20, 20);
        const sf::Color kHpBarFg(60, 220, 80);
        const sf::Color kHoverValid(100, 220, 100, 120);
        const sf::Color kHoverInvalid(220, 80, 80, 120);
        const sf::Color kUiBg(20, 20, 30);
        const sf::Color kUiText(230, 230, 240);
        const sf::Color kUiSelected(255, 220, 80);
    }

    // ============================================================
    // Конструктор
    // ============================================================

    Game::Game(const Config& config)
        : config_(config),
        grid_(Grid::CreateDefault(config.gridWidth, config.gridHeight)),
        renderer_(static_cast<unsigned int>(config.gridWidth* config.cellSize + 260),
            static_cast<unsigned int>(config.gridHeight* config.cellSize),
            config.cellSize),
        player_(config.startingGold, config.startingHp)
    {
        // Единый маршрут для всех врагов.
        enemyPath_ = Pathfinder::FindPathToGoal(grid_, grid_.spawn());

        if (enemyPath_.empty()) {
            std::cerr << "[Game] No path from spawn to goal on default grid!\n";
        }
    }

    // ============================================================
    // Главный цикл
    // ============================================================

    bool Game::Run() {
        using clock = std::chrono::steady_clock;

        auto lastTime = clock::now();

        while (renderer_.isOpen()) {
            auto now = clock::now();
            float dt = std::chrono::duration<float>(now - lastTime).count();
            lastTime = now;

            // Ограничиваем dt, чтобы после фризов физика не съезжала
            if (dt > 0.1f) dt = 0.1f;

            renderer_.pollEvents();
            HandleInput();

            if (state_ == GameState::Playing) {
                Update(dt);
            }

            Render();
            renderer_.display();
        }

        return true;
    }

    // ============================================================
    // Update
    // ============================================================

    void Game::Update(float dt) {
        UpdateWaves(dt);
        UpdateEnemies(dt);
        UpdateTowers(dt);
        UpdateProjectiles(dt);

        if (!player_.isAlive()) {
            state_ = GameState::Defeat;
        }
        if (waves_.allWavesDone() && enemies_.empty()) {
            state_ = GameState::Victory;
        }
    }

    void Game::UpdateWaves(float dt) {
        if (waitingForNextWave_) {
            wavePauseTimer_ -= dt;
            if (wavePauseTimer_ <= 0.0f && !waves_.allWavesDone()) {
                waves_.StartNextWave();
                waitingForNextWave_ = false;
                killedThisWave_ = 0;
            }
            return;
        }

        auto toSpawn = waves_.Tick(dt);
        for (EnemyType t : toSpawn) {
            SpawnEnemy(t);
        }

        // Если волна завершена и врагов на поле нет — готовимся к следующей
        if (!waves_.waveInProgress() && enemies_.empty()) {
            waitingForNextWave_ = true;
            wavePauseTimer_ = 3.0f;   // 3 секунды перерыв
        }
    }

    void Game::SpawnEnemy(EnemyType type) {
        float hp = config_.enemyNormalHp;
        float speed = config_.enemyNormalSpeed;
        int   reward = config_.enemyNormalReward;
        int   damage = config_.enemyNormalDamage;

        switch (type) {
        case EnemyType::Fast:
            hp = config_.enemyFastHp;
            speed = config_.enemyFastSpeed;
            reward = config_.enemyFastReward;
            damage = config_.enemyFastDamage;
            break;
        case EnemyType::Tank:
            hp = config_.enemyTankHp;
            speed = config_.enemyTankSpeed;
            reward = config_.enemyTankReward;
            damage = config_.enemyTankDamage;
            break;
        case EnemyType::Normal:
        default:
            break;
        }

        hp *= waves_.currentHpMultiplier();

        enemies_.emplace_back(type, enemyPath_, config_.cellSize, hp, speed, reward, damage);
    }

    void Game::UpdateEnemies(float dt) {
        for (auto& e : enemies_) {
            e.Update(dt);
        }

        // Обработка достигших базы и мёртвых
        for (auto it = enemies_.begin(); it != enemies_.end(); ) {
            if (it->reachedGoal()) {
                player_.takeDamage(it->damageToPlayer());
                it = enemies_.erase(it);
            }
            else if (!it->isAlive()) {
                player_.earn(it->reward());
                player_.addScore(10);
                ++killedThisWave_;
                it = enemies_.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    void Game::UpdateTowers(float dt) {
        for (auto& tower : towers_) {
            tower.Update(dt);

            if (!tower.canFire()) continue;

            Enemy* target = tower.AcquireTarget(enemies_);
            if (!target) continue;

            projectiles_.emplace_back(
                tower.position(),
                target,
                tower.projectileSpeed(),
                tower.damage(),
                tower.aoeRadius(),
                tower.hasSlowEffect(),
                tower.slowFactor(),
                tower.slowDuration());

            tower.OnFired();
        }
    }

    void Game::UpdateProjectiles(float dt) {
        for (auto& p : projectiles_) {
            bool done = p.Update(dt);
            if (done) {
                ApplyDamage(p, p.position());
            }
        }

        // Убираем отработавшие
        projectiles_.erase(
            std::remove_if(projectiles_.begin(), projectiles_.end(),
                [](const Projectile& p) { return p.isDone(); }),
            projectiles_.end());
    }

    void Game::ApplyDamage(const Projectile& p, const Vector2& hitPos) {
        // Одиночный урон
        if (p.aoeRadius() <= 0.0f) {
            Enemy* t = p.target();
            if (t && t->isAlive()) {
                t->TakeDamage(p.damage());
                if (p.hasSlowEffect()) {
                    t->ApplySlow(p.slowFactor(), p.slowDuration());
                }
            }
            return;
        }

        // AoE — урон всем в радиусе
        for (auto& e : enemies_) {
            if (!e.isAlive()) continue;
            float d2 = Vector2::distanceSquared(hitPos, e.position());
            if (d2 <= p.aoeRadius() * p.aoeRadius()) {
                e.TakeDamage(p.damage());
                if (p.hasSlowEffect()) {
                    e.ApplySlow(p.slowFactor(), p.slowDuration());
                }
            }
        }
    }

    // ============================================================
    // Input
    // ============================================================

    void Game::HandleInput() {
        // Обновляем hover
        Vector2 mouse = renderer_.mousePosition();
        hoverCell_ = PixelToCell(static_cast<int>(mouse.x), static_cast<int>(mouse.y));

        // Клик — ставим башню. Отслеживаем фронт нажатия (edge detection).
        static bool wasPressed = false;
        bool isPressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

        if (isPressed && !wasPressed) {
            HandleMouseClick(static_cast<int>(mouse.x), static_cast<int>(mouse.y));
        }
        wasPressed = isPressed;   // ← ГЛАВНАЯ ПРАВКА: всегда обновляем флаг

        // Клавиши выбора башни
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1)) {
            selectedTowerType_ = TowerType::Arrow;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2)) {
            selectedTowerType_ = TowerType::Cannon;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3)) {
            selectedTowerType_ = TowerType::Frost;
        }

        // Пробел — пауза
        static bool spaceHeld = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
            if (!spaceHeld) {
                spaceHeld = true;
                if (state_ == GameState::Playing) state_ = GameState::Paused;
                else if (state_ == GameState::Paused) state_ = GameState::Playing;
            }
        }
        else {
            spaceHeld = false;
        }
    }

    void Game::HandleMouseClick(int px, int py) {
        if (state_ != GameState::Playing) return;

        auto cell = PixelToCell(px, py);
        if (!cell) return;

        TryBuildTower(*cell);
    }

    void Game::TryBuildTower(Vector2i cell) {
        if (!grid_.isBuildable(cell)) return;

        int cost = Tower::CostFor(selectedTowerType_);
        if (!player_.canAfford(cost)) return;

        // Проверяем, что после постройки путь всё ещё существует.
        if (!PathStillExists(cell)) return;

        player_.spend(cost);
        towers_.emplace_back(selectedTowerType_, cell, config_.cellSize);

        // Помечаем клетку как занятую
        grid_.setCell(cell, CellType::Blocked);
    }

    bool Game::PathStillExists(Vector2i blockedCell) const {
        // Временно "блокируем" клетку и пробуем найти путь
        Grid temp = grid_;
        temp.setCell(blockedCell, CellType::Blocked);
        auto path = Pathfinder::FindPathToGoal(temp, temp.spawn());
        return !path.empty();
    }

    std::optional<Vector2i> Game::PixelToCell(int px, int py) const {
        int cellX = px / static_cast<int>(config_.cellSize);
        int cellY = py / static_cast<int>(config_.cellSize);
        Vector2i cell{ cellX, cellY };
        if (!grid_.inBounds(cell)) return std::nullopt;
        return cell;
    }

    // ============================================================
    // Render
    // ============================================================

    void Game::Render() {
        renderer_.clear(kBgColor);

        RenderGrid();
        RenderEnemies();
        RenderTowers();
        RenderProjectiles();
        RenderUI();

        if (state_ == GameState::Victory || state_ == GameState::Defeat) {
            RenderGameOver();
        }
    }

    void Game::RenderGrid() {
        float cs = config_.cellSize;

        for (int y = 0; y < grid_.height(); ++y) {
            for (int x = 0; x < grid_.width(); ++x) {
                Vector2i cell{ x, y };
                CellType t = grid_.getCell(cell);

                sf::Color color = kBgColor;
                switch (t) {
                case CellType::Blocked: color = kBlockedColor; break;
                case CellType::Spawn:   color = kSpawnColor;   break;
                case CellType::Goal:    color = kGoalColor;    break;
                case CellType::Empty:   color = kBgColor;      break;
                }

                renderer_.drawRect(x * cs, y * cs, cs, cs, color);
            }
        }

        // Рисуем путь
        if (enemyPath_.size() >= 2) {
            for (std::size_t i = 0; i + 1 < enemyPath_.size(); ++i) {
                Vector2 a = enemyPath_[i].toWorld(cs);
                Vector2 b = enemyPath_[i + 1].toWorld(cs);
                renderer_.drawLine(a.x, a.y, b.x, b.y, kPathColor, 6.0f);
            }
        }

        // Сетка сверху
        renderer_.drawGrid(grid_.width(), grid_.height(), cs, kGridColor);

        // Подсветка клетки под мышью (только в состоянии Playing)
        if (hoverCell_ && state_ == GameState::Playing) {
            CellType t = grid_.getCell(*hoverCell_);
            bool buildable = (t == CellType::Empty);
            if (buildable && player_.canAfford(Tower::CostFor(selectedTowerType_))) {
                renderer_.drawRect(hoverCell_->x * cs, hoverCell_->y * cs, cs, cs,
                    kHoverValid);
            }
            else {
                renderer_.drawRect(hoverCell_->x * cs, hoverCell_->y * cs, cs, cs,
                    kHoverInvalid);
            }
        }
    }

    void Game::RenderEnemies() {
        float cs = config_.cellSize;

        for (const auto& e : enemies_) {
            Vector2 p = e.position();

            sf::Color c = kEnemyNormal;
            if (e.type() == EnemyType::Fast) c = kEnemyFast;
            if (e.type() == EnemyType::Tank) c = kEnemyTank;

            renderer_.drawCircle(p.x, p.y, cs * 0.3f, c);

            // HP bar
            float hpFrac = e.hp() / e.maxHp();
            renderer_.drawHealthBar(p.x - cs * 0.35f, p.y - cs * 0.45f,
                cs * 0.7f, 5.0f, hpFrac,
                kHpBarBg, kHpBarFg);
        }
    }

    void Game::RenderTowers() {
        float cs = config_.cellSize;

        for (const auto& t : towers_) {
            Vector2 p = t.position();

            sf::Color c = kTowerArrow;
            if (t.type() == TowerType::Cannon) c = kTowerCannon;
            if (t.type() == TowerType::Frost)  c = kTowerFrost;

            renderer_.drawRect(p.x - cs * 0.35f, p.y - cs * 0.35f,
                cs * 0.7f, cs * 0.7f,
                c, sf::Color::White, 1.5f);

            // Показываем радиус при наведении на клетку башни
            if (hoverCell_ && *hoverCell_ == t.cell()) {
                sf::CircleShape r(t.range());
                r.setPosition({ p.x - t.range(), p.y - t.range() });
                r.setFillColor(sf::Color::Transparent);
                r.setOutlineColor(sf::Color(255, 255, 255, 80));
                r.setOutlineThickness(1.0f);
                renderer_.window().draw(r);
            }
        }
    }

    void Game::RenderProjectiles() {
        for (const auto& p : projectiles_) {
            Vector2 pos = p.position();
            renderer_.drawCircle(pos.x, pos.y, 3.0f, kProjectile);
        }
    }

    void Game::RenderUI() {
        float cs = config_.cellSize;
        float uiX = grid_.width() * cs;
        float uiW = renderer_.windowWidth() - uiX;

        // Фон панели
        renderer_.drawRect(uiX, 0.0f, uiW, renderer_.windowHeight(), kUiBg);

        float x = uiX + 15.0f;
        float y = 20.0f;

        renderer_.drawText("TOWER DEFENSE", x, y, 20, kUiText); y += 35;

        renderer_.drawText("HP:     " + std::to_string(player_.hp()), x, y, 16, kUiText); y += 24;
        renderer_.drawText("Gold:   " + std::to_string(player_.gold()), x, y, 16, kUiText); y += 24;
        renderer_.drawText("Score:  " + std::to_string(player_.score()), x, y, 16, kUiText); y += 24;
        renderer_.drawText("Wave:   " + std::to_string(waves_.currentWave()) +
            "/" + std::to_string(waves_.totalWaves()), x, y, 16, kUiText); y += 40;

        renderer_.drawText("BUILD (1-3):", x, y, 16, kUiText); y += 24;

        struct TowerEntry {
            const char* name;
            TowerType   type;
            sf::Color   color;
        };
        TowerEntry entries[] = {
            { "1. Arrow  ( 50)", TowerType::Arrow,  kTowerArrow  },
            { "2. Cannon (120)", TowerType::Cannon, kTowerCannon },
            { "3. Frost  ( 80)", TowerType::Frost,  kTowerFrost  },
        };

        for (const auto& e : entries) {
            bool selected = (e.type == selectedTowerType_);
            sf::Color textColor = selected ? kUiSelected : kUiText;

            renderer_.drawRect(x - 5.0f, y - 2.0f, 12.0f, 12.0f, e.color);
            renderer_.drawText(e.name, x + 15.0f, y - 4.0f, 14, textColor);
            y += 26;
        }

        y += 20;
        renderer_.drawText("SPACE: pause", x, y, 13, sf::Color(160, 160, 180)); y += 18;

        if (state_ == GameState::Paused) {
            renderer_.drawText("** PAUSED **", x, y + 20, 18, kUiSelected);
        }
    }

    void Game::RenderGameOver() {
        float w = renderer_.windowWidth();
        float h = renderer_.windowHeight();

        // Полупрозрачный оверлей
        renderer_.drawRect(0.0f, 0.0f, w, h, sf::Color(0, 0, 0, 180));

        std::string msg;
        sf::Color c;

        if (state_ == GameState::Victory) {
            msg = "VICTORY!";
            c = sf::Color(100, 240, 120);
        }
        else {
            msg = "DEFEAT";
            c = sf::Color(240, 100, 100);
        }

        // Рисуем по центру (примерно)
        float textX = w * 0.35f;
        float textY = h * 0.45f;
        renderer_.drawText(msg, textX, textY, 48, c);

        std::string scoreMsg = "Final score: " + std::to_string(player_.score());
        renderer_.drawText(scoreMsg, textX, textY + 70.0f, 20, kUiText);
    }

} // namespace td