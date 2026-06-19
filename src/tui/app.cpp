#include "app.h"
#include <ncurses.h>

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
    event_loop_.run(snap_mgr_);

    endwin();
    return 0;
}

void Application::register_screen(const std::string&, std::unique_ptr<Screen>) {}

void Application::switch_screen(const std::string&) {}
