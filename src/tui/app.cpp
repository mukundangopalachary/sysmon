#include "app.h"
#include <ncurses.h>
#include "screens/dashboard_screen.h"
#include "screens/process_list_screen.h"
#include "screens/connection_screen.h"
#include "screens/process_detail_screen.h"
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
    register_screen("connection", std::make_unique<ConnectionScreen>());
    
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
