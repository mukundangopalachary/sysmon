#include "app.h"
#include <ncurses.h>
#include "screens/dashboard_screen.h"
#include "screens/process_list_screen.h"
#include "screens/connection_screen.h"
Application::Application(SnapshotManager* snap_mgr) : snap_mgr_(snap_mgr), event_loop_(&screen_mgr_, &input_handler_) {}

int Application::run(int argc, char** argv) {
    (void)argc;
    (void)argv;

    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
        start_color();
        use_default_colors();
    }

    theme_mgr_.init();

    register_screen("dashboard", std::make_unique<DashboardScreen>());
    register_screen("process_list", std::make_unique<ProcessListScreen>());
    register_screen("connection", std::make_unique<ConnectionScreen>());
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
