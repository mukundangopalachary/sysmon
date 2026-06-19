#include "screens/process_list_screen.h"
#include <ncurses.h>

ProcessListScreen::ProcessListScreen() {
    on_resize();
}

ProcessListScreen::~ProcessListScreen() {}

void ProcessListScreen::render(const SystemSnapshot* snapshot) {
    if (table_panel_) {
        table_panel_->render(snapshot);
    }
}

bool ProcessListScreen::handle_input(int key) {
    if (table_panel_) {
        return table_panel_->handle_input(key);
    }
    return false;
}

void ProcessListScreen::on_resize() {
    int h, w;
    getmaxyx(stdscr, h, w);
    table_panel_.reset(new ProcessTablePanel(h, w, 0, 0));
    table_panel_->on_process_selected = [this](int pid) {
        if (this->on_process_selected) {
            this->on_process_selected(pid);
        }
    };
}
