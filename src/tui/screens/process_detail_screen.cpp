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
    if (!is_visible()) return;
    if (header_panel_) header_panel_->render(snapshot);
    if (detail_panel_) detail_panel_->render(snapshot);
    if (help_panel_) help_panel_->render(snapshot);
    doupdate();
}

bool ProcessDetailScreen::handle_input(int key) {
    if (detail_panel_) return detail_panel_->handle_input(key);
    return false;
}

void ProcessDetailScreen::on_resize() {
    int h, w;
    getmaxyx(stdscr, h, w);
    int header_h = 3;
    int help_h = 3;
    int detail_h = h - header_h - help_h;
    if (detail_h < 0) detail_h = 0;

    if (!header_panel_) header_panel_.reset(new HeaderPanel(header_h, w, 0, 0));
    else header_panel_->on_resize(header_h, w, 0, 0);

    if (!detail_panel_) {
        detail_panel_.reset(new ProcessDetailPanel(detail_h, w, header_h, 0));
        detail_panel_->on_close = [this]() {
            if (this->on_close) this->on_close();
        };
    } else {
        detail_panel_->on_resize(detail_h, w, header_h, 0);
    }

    if (!help_panel_) help_panel_.reset(new HelpPanel(help_h, w, header_h + detail_h, 0));
    else help_panel_->on_resize(help_h, w, header_h + detail_h, 0);
}
