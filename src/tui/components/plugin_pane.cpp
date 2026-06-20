#include "components/plugin_pane.h"
#include <ncurses.h>
#include <cstring>

extern "C" {
#include "plugin_manager.h"
}

PluginPaneComponent::PluginPaneComponent(int height, int width, int start_y, int start_x, const std::string& plugin_name)
    : Panel(height, width, start_y, start_x), plugin_name_(plugin_name) {}

PluginPaneComponent::~PluginPaneComponent() {}

void PluginPaneComponent::render(const SystemSnapshot* snapshot) {
    draw_border(has_focus_);
    
    char title[128];
    snprintf(title, sizeof(title), "%s%s [%s]", 
             expanded_ ? "v " : "> ", 
             plugin_name_.c_str(),
             enabled_ ? "ON" : "OFF");
    draw_title(title);
    
    if (!expanded_) return;
    
    // Find PluginInstance
    PluginInstance* p_inst = nullptr;
    if (snapshot && snapshot->plugin_mgr) {
        PluginManager* pm = (PluginManager*)snapshot->plugin_mgr;
        for (int i = 0; i < pm->num_plugins; i++) {
            if (plugin_name_ == pm->plugins[i].name) {
                p_inst = &pm->plugins[i];
                break;
            }
        }
    }
    
    if (!p_inst) {
        mvwprintw(window, 2, 2, "Plugin not found.");
        wnoutrefresh(window);
        return;
    }
    
    if (!p_inst->enabled) {
        mvwprintw(window, 2, 2, "Plugin is disabled.");
        wnoutrefresh(window);
        return;
    }
    
    // Simple JSON flattening to make it look nicer
    char temp[4096];
    snprintf(temp, sizeof(temp), "%s", p_inst->cached_output);
    char* line = strtok(temp, "\n");
    int y = 2;
    while (line && y < height_ - 1) {
        // Strip leading spaces
        while (*line == ' ') line++;
        // Skip brackets if they are alone
        if (strcmp(line, "{") == 0 || strcmp(line, "}") == 0 || 
            strcmp(line, "[") == 0 || strcmp(line, "]") == 0 ||
            strcmp(line, "},") == 0 || strcmp(line, "],") == 0) {
            line = strtok(NULL, "\n");
            continue;
        }
        
        mvwprintw(window, y++, 2, "%.*s", width_ - 4, line);
        line = strtok(NULL, "\n");
    }
    
    wnoutrefresh(window);
}

bool PluginPaneComponent::handle_input(int /*key*/) {
    return false;
}

void PluginPaneComponent::set_expanded(bool expanded) {
    expanded_ = expanded;
}

bool PluginPaneComponent::is_expanded() const {
    return expanded_;
}

const std::string& PluginPaneComponent::get_plugin_name() const {
    return plugin_name_;
}

void PluginPaneComponent::set_enabled(bool enabled) {
    enabled_ = enabled;
}

bool PluginPaneComponent::is_enabled() const {
    return enabled_;
}

int PluginPaneComponent::get_desired_height(const SystemSnapshot* /*snapshot*/) const {
    if (!expanded_) return 3; // Just the title bar
    
    // Height depends on how many panes/data rows
    return 15; // Increased to show more data lines
}
