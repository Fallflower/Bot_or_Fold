cmake_minimum_required(VERSION 3.14)
project(Holdem LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(HOLDEM_BUILD_CLI "Build the command-line frontend" ON)
option(HOLDEM_BUILD_GUI "Build the EUI-NEO graphical frontend" ON)

set(HOLDEM_CORE_SOURCES
    src/card.cpp
    src/handType.cpp
    src/handType_.cpp
    src/ranker_data.cpp
    src/ranker_data_short.cpp
    src/player.cpp
    src/humanPlayer.cpp
    src/botPlayer.cpp
    src/game.cpp
    src/gameController.cpp
    src/position.cpp
)

add_library(holdem_core STATIC ${HOLDEM_CORE_SOURCES})

# ============================================================================
# Include directories
# ============================================================================

target_include_directories(holdem_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# ============================================================================
# Compiler flags
# ============================================================================

target_compile_options(holdem_core PRIVATE
    -Wall -Wextra
)

# ============================================================================
# CLI application
# ============================================================================

if(HOLDEM_BUILD_CLI)
    add_executable(holdem_cli
        src/cli/main.cpp
    )
    target_link_libraries(holdem_cli PRIVATE holdem_core)
endif()

# ============================================================================
# EUI-NEO GUI application
# ============================================================================

if(HOLDEM_BUILD_GUI)
    add_subdirectory(external/EUI-NEO)

    if(MINGW AND TARGET harfbuzz)
        target_compile_options(harfbuzz PRIVATE -Wa,-mbig-obj)
    endif()

    add_executable(holdem_gui
        external/EUI-NEO/core/app/glfw_app_main.cpp
        src/gui/app.cpp
    )
    target_link_libraries(holdem_gui PRIVATE holdem_core)
    eui_neo_configure_app(holdem_gui)
    eui_neo_copy_app_assets(holdem_gui
        "${CMAKE_CURRENT_SOURCE_DIR}/external/EUI-NEO/assets"
    )
    eui_neo_copy_app_assets(holdem_gui
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    )
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(game_api_test tests/game_api_test.cpp)
    target_link_libraries(game_api_test PRIVATE holdem_core)
    add_test(NAME game_api COMMAND game_api_test)

    add_executable(game_controller_test tests/game_controller_test.cpp)
    target_link_libraries(game_controller_test PRIVATE holdem_core)
    add_test(NAME game_controller COMMAND game_controller_test)

    add_executable(handType_evaluate_test tests/handType_evaluate_test.cpp)
    target_link_libraries(handType_evaluate_test PRIVATE holdem_core)
    add_test(NAME handType_evaluate COMMAND handType_evaluate_test)
endif()

# ============================================================================
# Convenience targets
# ============================================================================

if(HOLDEM_BUILD_CLI)
    add_custom_target(run_cli
        COMMAND $<TARGET_FILE:holdem_cli> lhy 9 200 8 1
        DEPENDS holdem_cli
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        USES_TERMINAL
    )
endif()

if(HOLDEM_BUILD_GUI)
    add_custom_target(run_gui
        COMMAND $<TARGET_FILE:holdem_gui>
        DEPENDS holdem_gui
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        USES_TERMINAL
    )
endif()
