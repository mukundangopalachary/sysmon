cmake_minimum_required(VERSION 3.15)
project(sysmon 
    VERSION 2.0.1
    DESCRIPTION "Terminal System Resource Monitor"
    LANGUAGES C CXX
)

# ── Project Options ──────────────────────────────────────────
option(ENABLE_SANITIZERS "Enable address/undefined sanitizers" OFF)
option(ENABLE_LTO "Enable link-time optimization" OFF)
option(BUILD_TESTS "Build test suite" ON)
option(INSTALL_PLUGINS "Install Bash plugins" ON)

# ── Standards ─────────────────────────────────────────────────
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Compiler Flags ────────────────────────────────────────────
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

if(ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

if(ENABLE_LTO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# ── Dependencies ──────────────────────────────────────────────
find_package(Curses REQUIRED)
find_package(Threads REQUIRED)

# Optional: nlohmann_json for plugin config parsing
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
    add_compile_definitions(HAVE_NLOHMANN_JSON)
endif()

# ── Source Files ──────────────────────────────────────────────
# C Core Library
set(CORE_SOURCES
    src/core/cpu_stats.c
    src/core/mem_stats.c
    src/core/net_stats.c
    src/core/disk_stats.c
    src/core/process_list.c
    src/core/process_sort.c
    src/core/proc_reader.c
    src/core/sysinfo.c
    src/core/snapshot_manager.c
    src/core/collection_engine.c
    src/core/plugin_manager.c
    src/core/username_cache.c
    src/core/toml.c
    src/core/config_parser.c
    src/core/config_paths.c
    src/core/plugin_manifest.c
    src/core/plugin_state.c
    src/core/plugin_protocol.c
    src/core/plugin_registry.c
    src/bridge/sysmon_bridge.c
)

set(CORE_HEADERS
    src/core/cpu_stats.h
    src/core/mem_stats.h
    src/core/net_stats.h
    src/core/disk_stats.h
    src/core/process_list.h
    src/core/proc_reader.h
    src/core/sysinfo.h
    src/core/snapshot_manager.h
    src/core/collection_engine.h
    src/core/plugin_manager.h
    src/core/username_cache.h
    src/bridge/sysmon_bridge.h
    src/core/types.h
    src/core/toml.h
    src/core/config_parser.h
    src/core/config_paths.h
    src/core/plugin_manifest.h
    src/core/plugin_state.h
    src/core/plugin_protocol.h
    src/core/plugin_registry.h
)

# C++ TUI Library
set(TUI_SOURCES
    src/tui/app.cpp
    src/tui/event_loop.cpp
    src/tui/theme.cpp
    src/tui/input_handler.cpp
    src/tui/screen_manager.cpp
    src/tui/components/panel.cpp
    src/tui/components/header.cpp
    src/tui/components/cpu_panel.cpp
    src/tui/components/memory_panel.cpp
    src/tui/components/network_panel.cpp
    src/tui/components/disk_panel.cpp
    src/tui/components/mini_process_panel.cpp
    src/tui/components/plugin_pane.cpp
    src/tui/components/help_panel.cpp
    src/tui/components/process_table.cpp
    src/tui/components/connection_table.cpp
    src/tui/components/process_detail.cpp
    src/tui/screens/screen.cpp
    src/tui/screens/dashboard_screen.cpp
    src/tui/screens/process_list_screen.cpp
    src/tui/screens/connection_screen.cpp
    src/tui/screens/process_detail_screen.cpp
    src/tui/screens/help_screen.cpp
    src/tui/screens/plugin_page_screen.cpp
)

set(TUI_HEADERS
    src/tui/app.h
    src/tui/event_loop.h
    src/tui/theme.h
    src/tui/input_handler.h
    src/tui/screen_manager.h
    src/tui/components/panel.h
    src/tui/components/header.h
    src/tui/components/cpu_panel.h
    src/tui/components/memory_panel.h
    src/tui/components/network_panel.h
    src/tui/components/disk_panel.h
    src/tui/components/mini_process_panel.h
    src/tui/components/help_panel.h
    src/tui/components/process_table.h
    src/tui/components/connection_table.h
    src/tui/components/process_detail.h
    src/tui/screens/screen.h
    src/tui/screens/dashboard_screen.h
    src/tui/screens/process_list_screen.h
    src/tui/screens/connection_screen.h
    src/tui/screens/process_detail_screen.h
    src/tui/screens/help_screen.h
)

# ── Libraries ─────────────────────────────────────────────────
# C Core Library (static)
add_library(sysmon_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})
target_include_directories(sysmon_core PUBLIC
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/bridge
)
target_link_libraries(sysmon_core PUBLIC Threads::Threads)

# C++ TUI Library (static)
add_library(sysmon_tui STATIC ${TUI_SOURCES} ${TUI_HEADERS})
target_include_directories(sysmon_tui PUBLIC
    ${CMAKE_SOURCE_DIR}/src/tui
    ${CMAKE_SOURCE_DIR}/src/bridge
)
target_link_libraries(sysmon_tui PUBLIC 
    sysmon_core
    ${CURSES_LIBRARIES}
)

# ── Main Executable ───────────────────────────────────────────
add_executable(sysmon src/main.cpp src/cli/cli_main.c src/cli/cli_config.c src/cli/cli_plugin.c src/cli/cli_registry.c)
target_include_directories(sysmon PRIVATE ${CMAKE_SOURCE_DIR}/src/cli)
target_link_libraries(sysmon PRIVATE sysmon_tui)

add_executable(sysmon-cli src/cli/main.c src/cli/cli_main.c src/cli/cli_config.c src/cli/cli_plugin.c src/cli/cli_registry.c)
target_include_directories(sysmon-cli PRIVATE ${CMAKE_SOURCE_DIR}/src/cli)
target_link_libraries(sysmon-cli PRIVATE sysmon_core)

# Create sysmon-tui alias
add_custom_command(TARGET sysmon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_NAME:sysmon> ${CMAKE_CURRENT_BINARY_DIR}/sysmon-tui
)

# ── Installation ──────────────────────────────────────────────
include(GNUInstallDirs)

install(TARGETS sysmon sysmon-cli 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sysmon-tui
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Config file
install(FILES config/sysmon.toml 
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/sysmon
)

# Plugins
if(INSTALL_PLUGINS)
    install(DIRECTORY scripts/plugins/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/sysmon/plugins
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                         GROUP_READ GROUP_EXECUTE
                         WORLD_READ WORLD_EXECUTE
    )
    
    install(DIRECTORY scripts/alerts/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/sysmon/alerts
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                         GROUP_READ GROUP_EXECUTE
                         WORLD_READ WORLD_EXECUTE
    )
endif()

# ── Tests ─────────────────────────────────────────────────────
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ── Print Summary ─────────────────────────────────────────────
message(STATUS "SYSMON v${PROJECT_VERSION}")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Curses:         ${CURSES_LIBRARIES}")
message(STATUS "  Sanitizers:     ${ENABLE_SANITIZERS}")
message(STATUS "  LTO:            ${ENABLE_LTO}")
message(STATUS "  Tests:          ${BUILD_TESTS}")
