cmake_minimum_required(VERSION 3.14)
project(Holdem VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(HOLDEM_BUILD_CLI "Build the command-line frontend" ON)
option(HOLDEM_BUILD_GUI "Build the EUI-NEO graphical frontend" ON)
set(HOLDEM_VERSION "${PROJECT_VERSION}" CACHE STRING "Application version used in packages")
set(HOLDEM_APP_ICON "" CACHE FILEPATH "Generated ICO or ICNS derived from assets/icon.png")
string(REGEX MATCH "^[0-9]+(\\.[0-9]+)*" HOLDEM_BUNDLE_VERSION "${HOLDEM_VERSION}")
if(NOT HOLDEM_BUNDLE_VERSION)
    set(HOLDEM_BUNDLE_VERSION "${PROJECT_VERSION}")
endif()

if(DEFINED HOLDEM_ROOT)
    set(HOLDEM_RESOURCE_DIR "${HOLDEM_ROOT}/resources")
else()
    set(HOLDEM_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
endif()
if(EXISTS "${HOLDEM_RESOURCE_DIR}")
    file(COPY "${HOLDEM_RESOURCE_DIR}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
endif()

set(HOLDEM_CORE_SOURCES
    src/card.cpp
    src/handType.cpp
    src/handType_.cpp
    src/resource_locator.cpp
    src/pot_calculator.cpp
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

    if(EUI_WINDOW_BACKEND STREQUAL "sdl2")
        set(HOLDEM_EUI_APP_MAIN external/EUI-NEO/core/app/sdl2_app_main.cpp)
    elseif(EUI_WINDOW_BACKEND STREQUAL "glfw")
        set(HOLDEM_EUI_APP_MAIN external/EUI-NEO/core/app/glfw_app_main.cpp)
    else()
        message(FATAL_ERROR "Unsupported EUI window backend: ${EUI_WINDOW_BACKEND}")
    endif()

    set(HOLDEM_GUI_PLATFORM_SOURCES)
    if(WIN32 AND HOLDEM_APP_ICON)
        if(NOT EXISTS "${HOLDEM_APP_ICON}")
            message(FATAL_ERROR "HOLDEM_APP_ICON does not exist: ${HOLDEM_APP_ICON}")
        endif()
        file(TO_NATIVE_PATH "${HOLDEM_APP_ICON}" HOLDEM_WINDOWS_ICON_NATIVE)
        string(REPLACE "\\" "\\\\" HOLDEM_WINDOWS_ICON_RC "${HOLDEM_WINDOWS_ICON_NATIVE}")
        configure_file(
            packaging/windows/app-icon.rc.in
            "${CMAKE_CURRENT_BINARY_DIR}/holdem_app_icon.rc"
            @ONLY
        )
        list(APPEND HOLDEM_GUI_PLATFORM_SOURCES
            "${CMAKE_CURRENT_BINARY_DIR}/holdem_app_icon.rc")
    elseif(APPLE AND HOLDEM_APP_ICON)
        if(NOT EXISTS "${HOLDEM_APP_ICON}")
            message(FATAL_ERROR "HOLDEM_APP_ICON does not exist: ${HOLDEM_APP_ICON}")
        endif()
        set_source_files_properties("${HOLDEM_APP_ICON}" PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources")
        list(APPEND HOLDEM_GUI_PLATFORM_SOURCES "${HOLDEM_APP_ICON}")
    endif()

    add_executable(holdem_gui
        ${HOLDEM_EUI_APP_MAIN}
        src/gui/app.cpp
        src/gui/presentation.cpp
        ${HOLDEM_GUI_PLATFORM_SOURCES}
    )
    if(WIN32)
        set_target_properties(holdem_gui PROPERTIES
            OUTPUT_NAME "Bot or Fold"
            WIN32_EXECUTABLE TRUE)
        if(MINGW)
            target_link_options(holdem_gui PRIVATE -static-libgcc -static-libstdc++)
        endif()
    elseif(APPLE)
        configure_file(
            packaging/macos/Info.plist.in
            "${CMAKE_CURRENT_BINARY_DIR}/BotOrFold-Info.plist"
            @ONLY
        )
        set_target_properties(holdem_gui PROPERTIES
            OUTPUT_NAME "Bot or Fold"
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/BotOrFold-Info.plist"
            MACOSX_BUNDLE_BUNDLE_NAME "Bot or Fold"
            MACOSX_BUNDLE_GUI_IDENTIFIER "io.github.fallflower.botorfold"
            MACOSX_BUNDLE_ICON_FILE "BotOrFold.icns"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${HOLDEM_VERSION}")
    else()
        set_target_properties(holdem_gui PROPERTIES OUTPUT_NAME "bot-or-fold")
    endif()
    target_link_libraries(holdem_gui PRIVATE holdem_core)
    eui_neo_configure_app(holdem_gui)
    add_custom_command(TARGET holdem_gui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:holdem_gui>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/external/EUI-NEO/assets"
            "$<TARGET_FILE_DIR:holdem_gui>/assets"
        COMMAND ${CMAKE_COMMAND} -E remove -f
            "$<TARGET_FILE_DIR:holdem_gui>/assets/icon.png"
            "$<TARGET_FILE_DIR:holdem_gui>/assets/icon.ico"
            "$<TARGET_FILE_DIR:holdem_gui>/assets/icon.svg"
    )
    eui_neo_copy_app_assets(holdem_gui
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    )
    add_custom_command(TARGET holdem_gui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/resources"
            "$<TARGET_FILE_DIR:holdem_gui>/resources"
    )

    if(APPLE)
        install(TARGETS holdem_gui BUNDLE DESTINATION .)
        set(CPACK_GENERATOR "DragNDrop")
        set(CPACK_PACKAGE_NAME "Bot or Fold")
        set(CPACK_PACKAGE_VENDOR "Fallflower")
        set(CPACK_PACKAGE_VERSION "${HOLDEM_VERSION}")
        set(CPACK_PACKAGE_FILE_NAME "Bot-or-Fold-macOS")
        set(CPACK_DMG_VOLUME_NAME "Bot or Fold")
        include(CPack)
    elseif(WIN32)
        install(TARGETS holdem_gui RUNTIME DESTINATION .)
        install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/external/EUI-NEO/assets/"
            DESTINATION assets
            PATTERN "icon.png" EXCLUDE
            PATTERN "icon.ico" EXCLUDE
            PATTERN "icon.svg" EXCLUDE)
        install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/" DESTINATION assets)
        install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources/" DESTINATION resources)
    else()
        install(TARGETS holdem_gui RUNTIME DESTINATION bin)
        install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/external/EUI-NEO/assets/"
            DESTINATION bin/assets
            PATTERN "icon.png" EXCLUDE
            PATTERN "icon.ico" EXCLUDE
            PATTERN "icon.svg" EXCLUDE)
        install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/" DESTINATION bin/assets)
        install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources/" DESTINATION bin/resources)
    endif()
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

    add_executable(pot_calculator_test tests/pot_calculator_test.cpp)
    target_link_libraries(pot_calculator_test PRIVATE holdem_core)
    add_test(NAME pot_calculator COMMAND pot_calculator_test)
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
