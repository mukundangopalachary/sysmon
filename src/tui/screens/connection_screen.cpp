#include "screens/connection_screen.h"
#include <ncurses.h>

ConnectionScreen::ConnectionScreen() {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    table_panel_ = std::make_unique<ConnectionTablePanel>(max_y, max_x, 0, 0);
}

ConnectionScreen::~ConnectionScreen() {}

void ConnectionScreen::render(const SystemSnapshot* snapshot) {
    if (table_panel_) {
        table_panel_->render(snapshot);
    }
}

bool ConnectionScreen::handle_input(int key) {
    if (table_panel_) {
        return table_panel_->handle_input(key);
    }
    return false;
}

void ConnectionScreen::on_resize() {
    if (table_panel_) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        table_panel_->on_resize(max_y, max_x, 0, 0);
    }
}
