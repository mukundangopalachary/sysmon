#include "screens/plugin_page_screen.h"
#include <ncurses.h>
#include <cstdlib>

extern "C" {
#include "plugin_manager.h"
#include "config_paths.h"
}

PluginPageScreen::PluginPageScreen() {
    on_resize();
}

PluginPageScreen::~PluginPageScreen() {}

void PluginPageScreen::render(const SystemSnapshot* snapshot) {
    if (!is_visible()) return;
    
    werase(stdscr);
    wnoutrefresh(stdscr);
    
    if (header_panel_) header_panel_->render(snapshot);
    if (help_panel_) help_panel_->render(snapshot);

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    int content_start_y = 3;
    int content_end_y = max_y - 3;
    int content_h = content_end_y - content_start_y;
    
    if (!snapshot || !snapshot->plugin_mgr) {
        attron(A_BOLD);
        mvprintw(content_start_y + 2, 2, "Plugin system not initialized or unavailable.");
        attroff(A_BOLD);
        return;
    }
    
    PluginManager* pm = (PluginManager*)snapshot->plugin_mgr;
    pm_ = pm;
    
    // Sync plugin panes with plugin instances
    // For simplicity, we recreate or ensure panes match
    if (plugin_panes_.size() != (size_t)pm->num_plugins) {
        plugin_panes_.clear();
        for (int i = 0; i < pm->num_plugins; i++) {
            plugin_panes_.push_back(std::make_unique<PluginPaneComponent>(5, max_x, 0, 0, pm->plugins[i].name));
        }
    }
    
    if (plugin_panes_.empty()) {
        attron(A_BOLD);
        mvprintw(content_start_y + 2, 2, "No plugins loaded.");
        attroff(A_BOLD);
        mvprintw(content_start_y + 4, 2, "Use the CLI to install plugins:");
        mvprintw(content_start_y + 5, 2, "  sysmon plugin install <name>");
        return;
    }
    
    // Calculate layout
    int current_y = 0;
    
    // Process constraints
    if (selected_plugin_idx_ >= (int)plugin_panes_.size()) {
        selected_plugin_idx_ = plugin_panes_.size() - 1;
    }
    if (selected_plugin_idx_ < 0) {
        selected_plugin_idx_ = 0;
    }
    
    // Render
    for (size_t i = 0; i < plugin_panes_.size(); ++i) {
        auto& pane = plugin_panes_[i];
        pane->set_enabled(pm->plugins[i].enabled);
        
        int pane_h = pane->get_desired_height(snapshot);
        
        // Is it visible?
        if (current_y + pane_h > scroll_offset_ && current_y < scroll_offset_ + content_h) {
            int draw_y = content_start_y + current_y - scroll_offset_;
            // We just let the pane render normally and rely on ncurses clipping if it extends past screen,
            // or we manually adjust its dimensions.
            // For simplicity, we just set its y and render
            pane->on_resize(pane_h, max_x, draw_y, 0);
            
            if (i == (size_t)selected_plugin_idx_) {
                pane->on_focus();
            } else {
                pane->on_blur();
            }
            
            pane->render(snapshot);
        }
        
        current_y += pane_h;
    }
    
    // Ensure selected is visible
    int selected_start_y = 0;
    for (size_t i = 0; i < plugin_panes_.size(); ++i) {
        if (i == (size_t)selected_plugin_idx_) break;
        selected_start_y += plugin_panes_[i]->get_desired_height(snapshot);
    }
    int selected_end_y = selected_start_y + (plugin_panes_.empty() ? 0 : plugin_panes_[selected_plugin_idx_]->get_desired_height(snapshot));
    
    if (selected_start_y < scroll_offset_) {
        scroll_offset_ = selected_start_y;
    } else if (selected_end_y > scroll_offset_ + content_h) {
        scroll_offset_ = selected_end_y - content_h;
    }
    
    // Draw Plugin-specific keybindings
    attron(A_DIM);
    mvprintw(content_end_y, 2, "[j/k] Navigate  [Enter] Expand  [e] Enable/Disable  [c] Edit Config  [r] Refresh");
    attroff(A_DIM);
    
    doupdate();
}

bool PluginPageScreen::handle_input(int key) {
    if (plugin_panes_.empty()) return false;
    
    switch (key) {
        case KEY_UP:
        case 'k':
            if (selected_plugin_idx_ > 0) selected_plugin_idx_--;
            return true;
        case KEY_DOWN:
        case 'j':
            if (selected_plugin_idx_ < (int)plugin_panes_.size() - 1) selected_plugin_idx_++;
            return true;
        case '\n':
        case KEY_ENTER:
            plugin_panes_[selected_plugin_idx_]->set_expanded(!plugin_panes_[selected_plugin_idx_]->is_expanded());
            return true;
        case 'e':
        case 'E': {
            auto& pane = plugin_panes_[selected_plugin_idx_];
            bool new_state = !pane->is_enabled();
            pane->set_enabled(new_state);
            
            // Persist the state using the CLI
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "sysmon-cli plugin %s %s > /dev/null 2>&1", 
                     new_state ? "enable" : "disable", 
                     pane->get_plugin_name().c_str());
            int ret = system(cmd);
            (void)ret;
            
            // Also immediately mutate the live backend engine
            if (pm_) {
                PluginManager* live_pm = (PluginManager*)pm_;
                if (selected_plugin_idx_ < live_pm->num_plugins) {
                    live_pm->plugins[selected_plugin_idx_].enabled = new_state;
                }
            }
            return true;
        }
        case 'c':
        case 'C': {
            def_prog_mode();
            endwin();
            const char* editor = getenv("EDITOR");
            if (!editor) editor = "nano";
            
            char path[1024];
            config_get_user_config_path(path, sizeof(path));
            // Instead of sysmon.toml, we want plugins.toml
            // We'll just construct it manually to ensure we get plugins.toml
            snprintf(path, sizeof(path), "%s/.config/sysmon/plugins.toml", getenv("HOME"));
            
            char cmd[2048];
            snprintf(cmd, sizeof(cmd), "%s \"%s\"", editor, path);
            int ret = system(cmd);
            (void)ret;
            
            reset_prog_mode();
            refresh();
            return true;
        }
        case 'r':
        case 'R': {
            // Force immediate refresh by zeroing the last run timestamp in the live backend engine
            // AND actively pulling the data immediately so the UI is instantaneous
            if (pm_) {
                PluginManager* live_pm = (PluginManager*)pm_;
                if (selected_plugin_idx_ < live_pm->num_plugins) {
                    live_pm->plugins[selected_plugin_idx_].last_run_us = 0;
                    
                    // Call collect directly to immediately run the plugin script
                    // We'll preserve other plugins' timestamps, so only this one (or ones at interval) will run
                    plugin_manager_collect(live_pm, nullptr);
                }
            }
            return true;
        }
    }
    return false;
}

void PluginPageScreen::on_resize() {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    int header_h = 3;
    int help_h = 3;
    
    if (!header_panel_) header_panel_.reset(new HeaderPanel(header_h, max_x, 0, 0));
    else header_panel_->on_resize(header_h, max_x, 0, 0);
    
    if (!help_panel_) help_panel_.reset(new HelpPanel(help_h, max_x, max_y - help_h, 0));
    else help_panel_->on_resize(help_h, max_x, max_y - help_h, 0);
}
