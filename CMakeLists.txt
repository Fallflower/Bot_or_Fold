cmake_minimum_required(VERSION 3.14)
project(Holdem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
    main.cpp
    src/card.cpp
    src/handType.cpp
    src/handType_.cpp
    src/ranker_data.cpp
    src/ranker_data_short.cpp
    src/player.cpp
    src/humanPlayer.cpp
    src/botPlayer.cpp
    src/game.cpp
    src/position.cpp
)

add_executable(main ${SOURCES})

# ============================================================================
# Include directories
# ============================================================================

target_include_directories(main PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# ============================================================================
# Compiler flags
# ============================================================================

target_compile_options(main PRIVATE
    -Wall -Wextra
)

# ============================================================================
# Convenience targets
# ============================================================================

add_custom_target(run
    COMMAND main lhy 9 200 8 1
    DEPENDS main
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
