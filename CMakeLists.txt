cmake_minimum_required(VERSION 3.20)
project(TowerDefense VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8)
endif()

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# --- Библиотека игры ---
add_library(td
    src/Grid.cpp
    src/Pathfinder.cpp
    src/Enemy.cpp
    src/Tower.cpp
    src/Projectile.cpp
    src/Wave.cpp
    src/Player.cpp
    src/Config.cpp
    src/Renderer.cpp    # <-- новое
    src/Game.cpp        # <-- новое
    third_party/json/Value.cpp
    third_party/json/Parser.cpp
    third_party/json/Serializer.cpp
    third_party/json/Serializer.cpp
)
target_include_directories(td PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

find_package(SFML COMPONENTS System Window Graphics CONFIG REQUIRED)
target_link_libraries(td PUBLIC SFML::Graphics SFML::Window SFML::System)

# --- Игра ---
add_executable(td_game apps/main.cpp)
target_link_libraries(td_game PRIVATE td)

# --- Тесты ---
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()
    find_package(GTest CONFIG REQUIRED)

    add_executable(td_tests tests/test_game.cpp)
    target_link_libraries(td_tests PRIVATE td GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(td_tests)
endif()

set_target_properties(td_game td_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)