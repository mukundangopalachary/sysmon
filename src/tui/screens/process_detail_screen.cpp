#include "screens/process_detail_screen.h"
#include <ncurses.h>

ProcessDetailScreen::ProcessDetailScreen() {
    on_resize();
}

ProcessDetailScreen::~ProcessDetailScreen() {}

void ProcessDetailScreen::open(int pid) {
    if (detail_panel_) detail_panel_->open(pid);
}

void ProcessDetailScreen::render(const SystemSnapshot* snapshot) {
    if (detail_panel_) detail_panel_->render(snapshot);
}

bool ProcessDetailScreen::handle_input(int key) {
    if (detail_panel_) return detail_panel_->handle_input(key);
    return false;
}

void ProcessDetailScreen::on_resize() {
    int h, w;
    getmaxyx(stdscr, h, w);
    detail_panel_.reset(new ProcessDetailPanel(h, w, 0, 0));
    detail_panel_->on_close = [this]() {
        if (this->on_close) this->on_close();
    };
}
