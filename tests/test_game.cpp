#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

#include "td/Vector2.h"
#include "td/Grid.h"
#include "td/Pathfinder.h"
#include "td/Enemy.h"
#include "td/Tower.h"
#include "td/Projectile.h"

using namespace td;

// ============================================================
// Vector2
// ============================================================

TEST(Vector2, Addition) {
    Vector2 a(1.0f, 2.0f);
    Vector2 b(3.0f, 4.0f);
    Vector2 c = a + b;
    EXPECT_FLOAT_EQ(c.x, 4.0f);
    EXPECT_FLOAT_EQ(c.y, 6.0f);
}

TEST(Vector2, Length) {
    Vector2 v(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v.length(), 5.0f);
}

TEST(Vector2, Normalize) {
    Vector2 v(3.0f, 4.0f);
    Vector2 n = v.normalized();
    EXPECT_NEAR(n.length(), 1.0f, 1e-5f);
}

TEST(Vector2, Distance) {
    Vector2 a(0.0f, 0.0f);
    Vector2 b(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(Vector2::distance(a, b), 5.0f);
}

TEST(Vector2i, ToWorld) {
    Vector2i cell(2, 3);
    Vector2 world = cell.toWorld(32.0f);
    EXPECT_FLOAT_EQ(world.x, 2 * 32.0f + 16.0f);
    EXPECT_FLOAT_EQ(world.y, 3 * 32.0f + 16.0f);
}

// ============================================================
// Grid
// ============================================================

TEST(Grid, Constructor) {
    Grid g(10, 5);
    EXPECT_EQ(g.width(), 10);
    EXPECT_EQ(g.height(), 5);
}

TEST(Grid, BoundsCheck) {
    Grid g(10, 5);
    EXPECT_TRUE(g.inBounds({ 0, 0 }));
    EXPECT_TRUE(g.inBounds({ 9, 4 }));
    EXPECT_FALSE(g.inBounds({ -1, 0 }));
    EXPECT_FALSE(g.inBounds({ 0, -1 }));
    EXPECT_FALSE(g.inBounds({ 10, 0 }));
    EXPECT_FALSE(g.inBounds({ 0, 5 }));
}

TEST(Grid, FromString) {
    std::string ascii =
        "S..G\n"
        ".#..\n";
    Grid g = Grid::FromString(ascii);

    EXPECT_EQ(g.width(), 4);
    EXPECT_EQ(g.height(), 2);
    EXPECT_EQ(g.spawn(), (Vector2i{ 0, 0 }));
    EXPECT_EQ(g.goal(), (Vector2i{ 3, 0 }));
    EXPECT_EQ(g.getCell({ 1, 1 }), CellType::Blocked);
    EXPECT_EQ(g.getCell({ 2, 1 }), CellType::Empty);
}

TEST(Grid, WalkableAndBuildable) {
    std::string ascii = "S.#G\n";
    Grid g = Grid::FromString(ascii);

    EXPECT_TRUE(g.isWalkable({ 0, 0 }));
    EXPECT_TRUE(g.isWalkable({ 1, 0 }));
    EXPECT_FALSE(g.isWalkable({ 2, 0 }));
    EXPECT_TRUE(g.isWalkable({ 3, 0 }));

    EXPECT_FALSE(g.isBuildable({ 0, 0 }));
    EXPECT_TRUE(g.isBuildable({ 1, 0 }));
    EXPECT_FALSE(g.isBuildable({ 2, 0 }));
    EXPECT_FALSE(g.isBuildable({ 3, 0 }));
}

TEST(Grid, EncodeDecodeRoundTrip) {
    Grid g(10, 5);
    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 10; ++x) {
            Vector2i p{ x, y };
            int enc = g.encode(p);
            Vector2i back = g.decode(enc);
            EXPECT_EQ(p, back);
        }
    }
}

// ============================================================
// Pathfinder
// ============================================================

TEST(Pathfinder, StraightLine) {
    std::string ascii = "S....G\n";
    Grid g = Grid::FromString(ascii);

    auto path = Pathfinder::FindPath(g, g.spawn(), g.goal());
    EXPECT_EQ(path.size(), 6u);
    EXPECT_EQ(path.front(), (Vector2i{ 0, 0 }));
    EXPECT_EQ(path.back(), (Vector2i{ 5, 0 }));
}

TEST(Pathfinder, AroundObstacle) {
    std::string ascii =
        "S.#..\n"
        "..#..\n"
        "..G..\n";
    Grid g = Grid::FromString(ascii);

    auto path = Pathfinder::FindPath(g, g.spawn(), g.goal());
    EXPECT_FALSE(path.empty());
    EXPECT_EQ(path.front(), (Vector2i{ 0, 0 }));
    EXPECT_EQ(path.back(), (Vector2i{ 2, 2 }));

    for (const auto& p : path) {
        EXPECT_NE(g.getCell(p), CellType::Blocked);
    }
}

TEST(Pathfinder, NoPath) {
    std::string ascii = "S#G\n";
    Grid g = Grid::FromString(ascii);

    auto path = Pathfinder::FindPath(g, g.spawn(), g.goal());
    EXPECT_TRUE(path.empty());
}

TEST(Pathfinder, StartEqualsGoal) {
    std::string ascii = "S....\n";
    Grid g = Grid::FromString(ascii);

    auto path = Pathfinder::FindPath(g, g.spawn(), g.spawn());
    ASSERT_EQ(path.size(), 1u);
    EXPECT_EQ(path[0], g.spawn());
}

TEST(Pathfinder, FindPathToGoal) {
    std::string ascii =
        "S....\n"
        ".....\n"
        "....G\n";
    Grid g = Grid::FromString(ascii);

    auto path = Pathfinder::FindPathToGoal(g, g.spawn());
    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front(), g.spawn());
    EXPECT_EQ(path.back(), g.goal());
}

TEST(Pathfinder, ShortestPathLength) {
    std::string ascii =
        "S.#\n"
        ".##\n"
        "..G\n";
    Grid g = Grid::FromString(ascii);

    auto path = Pathfinder::FindPath(g, g.spawn(), g.goal());
    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.size(), 5u);
}

TEST(Pathfinder, OutOfBounds) {
    Grid g(5, 5);
    auto path = Pathfinder::FindPath(g, { -1, 0 }, { 4, 4 });
    EXPECT_TRUE(path.empty());

    path = Pathfinder::FindPath(g, { 0, 0 }, { 5, 5 });
    EXPECT_TRUE(path.empty());
}

// ============================================================
// Enemy
// ============================================================

TEST(Enemy, ConstructorSetsInitialPosition) {
    std::vector<Vector2i> path = { {0, 0}, {1, 0}, {2, 0} };
    Enemy e(EnemyType::Normal, path, 32.0f, 100.0f, 50.0f, 10, 1);
    EXPECT_FLOAT_EQ(e.position().x, 16.0f);
    EXPECT_FLOAT_EQ(e.position().y, 16.0f);
    EXPECT_TRUE(e.isAlive());
    EXPECT_FALSE(e.reachedGoal());
}

TEST(Enemy, MovesAlongPath) {
    std::vector<Vector2i> path = { {0, 0}, {1, 0} };
    Enemy e(EnemyType::Normal, path, 32.0f, 100.0f, 32.0f, 10, 1);

    e.Update(1.0f);
    EXPECT_TRUE(e.reachedGoal());
}

TEST(Enemy, TakeDamage) {
    std::vector<Vector2i> path = { {0, 0}, {1, 0} };
    Enemy e(EnemyType::Normal, path, 32.0f, 100.0f, 50.0f, 10, 1);

    EXPECT_FALSE(e.TakeDamage(50.0f));
    EXPECT_FLOAT_EQ(e.hp(), 50.0f);
    EXPECT_TRUE(e.TakeDamage(60.0f));
    EXPECT_FALSE(e.isAlive());
}

TEST(Enemy, SlowEffect) {
    std::vector<Vector2i> path = { {0, 0}, {1, 0} };
    Enemy e(EnemyType::Normal, path, 32.0f, 100.0f, 100.0f, 10, 1);

    EXPECT_FLOAT_EQ(e.currentSpeedMultiplier(), 1.0f);
    e.ApplySlow(0.5f, 2.0f);
    EXPECT_FLOAT_EQ(e.currentSpeedMultiplier(), 0.5f);

    e.Update(3.0f);
    EXPECT_FLOAT_EQ(e.currentSpeedMultiplier(), 1.0f);
}

TEST(Enemy, EmptyPathThrows) {
    EXPECT_THROW(
        Enemy(EnemyType::Normal, std::vector<Vector2i>{}, 32.0f, 100.0f, 50.0f, 10, 1),
        std::invalid_argument);
}

// ============================================================
// Tower
// ============================================================

TEST(Tower, ArrowStats) {
    Tower t(TowerType::Arrow, { 2, 3 }, 32.0f);
    EXPECT_EQ(t.type(), TowerType::Arrow);
    EXPECT_EQ(t.cell(), (Vector2i{ 2, 3 }));
    EXPECT_FLOAT_EQ(t.position().x, 2 * 32.0f + 16.0f);
    EXPECT_GT(t.range(), 0.0f);
    EXPECT_GT(t.damage(), 0.0f);
    EXPECT_TRUE(t.canFire());
}

TEST(Tower, CooldownAfterFire) {
    Tower t(TowerType::Arrow, { 0, 0 }, 32.0f);
    t.OnFired();
    EXPECT_FALSE(t.canFire());

    t.Update(0.2f);
    EXPECT_FALSE(t.canFire());

    t.Update(0.3f);
    EXPECT_TRUE(t.canFire());
}

TEST(Tower, CostPerType) {
    EXPECT_GT(Tower::CostFor(TowerType::Arrow), 0);
    EXPECT_GT(Tower::CostFor(TowerType::Cannon), 0);
    EXPECT_GT(Tower::CostFor(TowerType::Frost), 0);
    EXPECT_GT(Tower::CostFor(TowerType::Cannon),
        Tower::CostFor(TowerType::Arrow));
}

// ============================================================
// Projectile
// ============================================================

TEST(Projectile, ReachesTarget) {
    std::vector<Vector2i> path = { {0, 0}, {5, 0} };
    Enemy e(EnemyType::Normal, path, 32.0f, 100.0f, 0.0f, 10, 1);

    Projectile p({ 0.0f, 0.0f }, &e, 1000.0f, 10.0f, 0.0f, false, 1.0f, 0.0f);

    bool done = false;
    for (int i = 0; i < 100 && !done; ++i) {
        done = p.Update(0.01f);
    }
    EXPECT_TRUE(done);
    EXPECT_TRUE(p.isDone());
}

TEST(Projectile, DeadTargetEndsFlight) {
    std::vector<Vector2i> path = { {0, 0}, {5, 0} };
    Enemy e(EnemyType::Normal, path, 32.0f, 100.0f, 0.0f, 10, 1);

    Projectile p({ 0.0f, 0.0f }, &e, 100.0f, 10.0f, 0.0f, false, 1.0f, 0.0f);

    e.TakeDamage(200.0f);
    EXPECT_TRUE(p.Update(0.01f));
}

TEST(Projectile, NullTargetEndsImmediately) {
    Projectile p({ 0.0f, 0.0f }, nullptr, 100.0f, 10.0f, 0.0f, false, 1.0f, 0.0f);
    EXPECT_TRUE(p.Update(0.01f));
}

TEST(Projectile, FrostCarriesSlowInfo) {
    std::vector<Vector2i> path = { {0, 0}, {5, 0} };
    Enemy e(EnemyType::Normal, path, 32.0f, 100.0f, 0.0f, 10, 1);

    Projectile p({ 0.0f, 0.0f }, &e, 100.0f, 5.0f, 0.0f, true, 0.5f, 2.0f);
    EXPECT_TRUE(p.hasSlowEffect());
    EXPECT_FLOAT_EQ(p.slowFactor(), 0.5f);
    EXPECT_FLOAT_EQ(p.slowDuration(), 2.0f);
}

#include "td/Player.h"
#include "td/Wave.h"
#include "td/Config.h"

// ============================================================
// Player
// ============================================================

TEST(Player, StartsWithInitialValues) {
    Player p(200, 20);
    EXPECT_EQ(p.gold(), 200);
    EXPECT_EQ(p.hp(), 20);
    EXPECT_EQ(p.score(), 0);
    EXPECT_TRUE(p.isAlive());
}

TEST(Player, CanAfford) {
    Player p(100, 10);
    EXPECT_TRUE(p.canAfford(50));
    EXPECT_TRUE(p.canAfford(100));
    EXPECT_FALSE(p.canAfford(150));
}

TEST(Player, SpendAndEarn) {
    Player p(100, 10);
    p.spend(30);
    EXPECT_EQ(p.gold(), 70);
    p.earn(15);
    EXPECT_EQ(p.gold(), 85);
}

TEST(Player, TakeDamage) {
    Player p(100, 3);
    p.takeDamage(1);
    EXPECT_EQ(p.hp(), 2);
    EXPECT_TRUE(p.isAlive());
    p.takeDamage(5);
    EXPECT_EQ(p.hp(), 0);
    EXPECT_FALSE(p.isAlive());
}

TEST(Player, ScoreAccumulates) {
    Player p(0, 1);
    p.addScore(10);
    p.addScore(25);
    EXPECT_EQ(p.score(), 35);
}

// ============================================================
// WaveManager
// ============================================================

TEST(Wave, StartsIdle) {
    WaveManager w;
    EXPECT_EQ(w.currentWave(), 0);
    EXPECT_GT(w.totalWaves(), 0);
    EXPECT_FALSE(w.waveInProgress());
    EXPECT_FALSE(w.allWavesDone());
}

TEST(Wave, StartNextWave) {
    WaveManager w;
    EXPECT_TRUE(w.StartNextWave());
    EXPECT_TRUE(w.waveInProgress());
}

TEST(Wave, SpawnsEnemiesOverTime) {
    WaveManager w;
    w.StartNextWave();

    // За очень большой шаг — все враги волны должны появиться сразу.
    auto spawned = w.Tick(100.0f);
    EXPECT_GT(spawned.size(), 0u);
}

TEST(Wave, FinishesAfterAllSpawned) {
    WaveManager w;
    w.StartNextWave();

    // Прокручиваем достаточно времени — волна должна закончиться.
    for (int i = 0; i < 1000; ++i) {
        w.Tick(0.1f);
    }

    EXPECT_FALSE(w.waveInProgress());
    EXPECT_EQ(w.currentWave(), 1);
}

TEST(Wave, AllWavesEventuallyDone) {
    WaveManager w;
    while (w.StartNextWave()) {
        for (int i = 0; i < 10000; ++i) w.Tick(0.1f);
    }
    EXPECT_TRUE(w.allWavesDone());
}

// ============================================================
// Config
// ============================================================

TEST(Config, DefaultsAreSane) {
    Config c = Config::Default();
    EXPECT_GT(c.gridWidth, 0);
    EXPECT_GT(c.gridHeight, 0);
    EXPECT_GT(c.cellSize, 0.0f);
    EXPECT_GT(c.startingGold, 0);
    EXPECT_GT(c.startingHp, 0);
}

TEST(Config, MissingFileUsesDefaults) {
    Config c = Config::LoadFromFile("nonexistent_file_xyz.json");
    EXPECT_EQ(c.gridWidth, 20);
    EXPECT_EQ(c.startingGold, 200);
}