#include "td/Game.h"
#include "td/Config.h"

#include <iostream>

int main() {
    try {
        // Пробуем загрузить config.json, если нет - используем значения по умолчанию.
        td::Config config = td::Config::LoadFromFile("assets/config.json");

        td::Game game(config);
        game.Run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}