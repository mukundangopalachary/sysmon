#include "screens/connection_screen.h"
#include <ncurses.h>

ConnectionScreen::ConnectionScreen() {
    on_resize();
}

ConnectionScreen::~ConnectionScreen() {}

void ConnectionScreen::set_config(const SysmonConfig* cfg) {
    cfg_ = cfg;
    if (help_panel_) help_panel_->set_config(cfg);
}

void ConnectionScreen::render(const SystemSnapshot* snapshot) {
    if (!is_visible()) return;
    if (header_panel_) header_panel_->render(snapshot);
    if (table_panel_) table_panel_->render(snapshot);
    if (help_panel_) help_panel_->render(snapshot);
    doupdate();
}

bool ConnectionScreen::handle_input(int key) {
    if (table_panel_) {
        return table_panel_->handle_input(key);
    }
    return false;
}

void ConnectionScreen::on_resize() {
    int h, w;
    getmaxyx(stdscr, h, w);
    int header_h = 3;
    int help_h = 3;
    int table_h = h - header_h - help_h;
    if (table_h < 0) table_h = 0;

    if (!header_panel_) header_panel_.reset(new HeaderPanel(header_h, w, 0, 0));
    else header_panel_->on_resize(header_h, w, 0, 0);

    if (!table_panel_) {
        table_panel_.reset(new ConnectionTablePanel(table_h, w, header_h, 0));
        table_panel_->on_connection_selected = [this](int pid) {
            if (this->on_connection_selected) {
                this->on_connection_selected(pid);
            }
        };
    } else {
        table_panel_->on_resize(table_h, w, header_h, 0);
    }

    if (!help_panel_) help_panel_.reset(new HelpPanel(help_h, w, header_h + table_h, 0));
    else help_panel_->on_resize(help_h, w, header_h + table_h, 0);
}
