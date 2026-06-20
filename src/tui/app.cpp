#include "app.h"
#include <ncurses.h>
#include "screens/dashboard_screen.h"
#include "screens/process_list_screen.h"
#include "screens/connection_screen.h"
#include "screens/process_detail_screen.h"
#include "screens/plugin_page_screen.h"

Application::Application(SnapshotManager* snap_mgr, CollectionEngine* engine) 
    : snap_mgr_(snap_mgr), engine_(engine), event_loop_(&screen_mgr_, &input_handler_) {}

int Application::run(int argc, char** argv) {
    (void)argc;
    (void)argv;

    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
        start_color();
        use_default_colors();
    }

    char config_path[512];
    config_get_user_config_path(config_path, sizeof(config_path));
    
    config_init_defaults(&cfg_);
    if (!config_load(&cfg_, config_path)) {
        // Automatically generate the default config file for the user if it doesn't exist
        config_ensure_directories();
        config_save(&cfg_, config_path);
    }

    theme_mgr_.set_theme(cfg_.display.theme);
    theme_mgr_.init();
    
    // Apply default theme colors to the main screen
    bkgd(COLOR_PAIR(THEME_DEFAULT) | ' ');
    
    screen_mgr_.set_config(&cfg_);
    input_handler_.init(&cfg_, &screen_mgr_, engine_);

    register_screen("dashboard", std::make_unique<DashboardScreen>());
    register_screen("connection", std::make_unique<ConnectionScreen>());
    register_screen("plugin_page", std::make_unique<PluginPageScreen>());
    
    auto process_list = std::make_unique<ProcessListScreen>();
    process_list->on_process_selected = [this](int pid) {
        auto detail = static_cast<ProcessDetailScreen*>(screen_mgr_.get_screen("process_detail"));
        if (detail) {
            detail->open(pid);
            switch_screen("process_detail");
        }
    };
    register_screen("process_list", std::move(process_list));
    
    auto process_detail = std::make_unique<ProcessDetailScreen>();
    process_detail->on_close = [this]() {
        switch_screen("process_list");
    };
    register_screen("process_detail", std::move(process_detail));
    
    switch_screen("dashboard");

    event_loop_.run(snap_mgr_);

    endwin();
    return 0;
}

void Application::register_screen(const std::string& name, std::unique_ptr<Screen> screen) {
    screen_mgr_.register_screen(name, std::move(screen));
}

void Application::switch_screen(const std::string& name) {
    screen_mgr_.switch_screen(name);
}
