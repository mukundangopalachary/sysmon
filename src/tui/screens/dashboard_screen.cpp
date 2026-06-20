#include "screens/dashboard_screen.h"
#include "components/header.h"
#include "components/cpu_panel.h"
#include "components/memory_panel.h"
#include "components/network_panel.h"
#include "components/disk_panel.h"
#include "components/plugin_panel.h"
#include "components/help_panel.h"
#include <ncurses.h>

DashboardScreen::DashboardScreen() {
    on_resize();
}

DashboardScreen::~DashboardScreen() {}

void DashboardScreen::render(const SystemSnapshot* snapshot) {
    if (!visible_) return;
    
    for (auto& panel : panels_) {
        // If the panel is a HelpPanel, pass the config
        HelpPanel* hp = dynamic_cast<HelpPanel*>(panel.get());
        if (hp) hp->set_config(cfg_);
        
        panel->render(snapshot);
    }
    doupdate();
}

bool DashboardScreen::handle_input(int key) {
    for (auto& panel : panels_) {
        if (panel->handle_input(key)) {
            return true;
        }
    }
    return false;
}

void DashboardScreen::on_resize() {
    panels_.clear();
    
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    int header_h = 3;
    int mid_h = 10;
    int half_w = max_x / 2;
    int bot_h = max_y - header_h - mid_h - 3; // reserve 3 for help panel
    if (bot_h < 0) bot_h = 0;
    int third_w = max_x / 3;

    panels_.push_back(std::make_unique<HeaderPanel>(header_h, max_x, 0, 0));
    panels_.push_back(std::make_unique<CpuPanel>(mid_h, half_w, header_h, 0));
    panels_.push_back(std::make_unique<MemoryPanel>(mid_h, max_x - half_w, header_h, half_w));
    panels_.push_back(std::make_unique<NetworkPanel>(bot_h, third_w, header_h + mid_h, 0));
    panels_.push_back(std::make_unique<DiskPanel>(bot_h, third_w, header_h + mid_h, third_w));
    panels_.push_back(std::make_unique<PluginPanel>(bot_h, max_x - 2 * third_w, header_h + mid_h, 2 * third_w));
    panels_.push_back(std::make_unique<HelpPanel>(3, max_x, header_h + mid_h + bot_h, 0));
}
