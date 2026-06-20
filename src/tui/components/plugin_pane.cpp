#include "components/plugin_pane.h"
#include <ncurses.h>
#include <cstring>
#include <cstdlib>

extern "C" {
#include "plugin_manager.h"
}

PluginPaneComponent::PluginPaneComponent(int height, int width, int start_y, int start_x, const std::string& plugin_name)
    : Panel(height, width, start_y, start_x), plugin_name_(plugin_name) {}

PluginPaneComponent::~PluginPaneComponent() {}

// Parse "key=value:TYPE:unit" into a nice display like "  Key Name        42 ms"
static void render_metric_line(WINDOW* win, int y, int max_w, const char* raw) {
    // Skip PANE: and ENDPANE lines
    if (strncmp(raw, "PANE:", 5) == 0 || strncmp(raw, "ENDPANE", 7) == 0) return;

    // Parse: metric.name=value:TYPE:unit
    char key[128] = {0}, value[64] = {0}, unit[32] = {0};
    const char* eq = strchr(raw, '=');
    if (!eq) {
        // Not a metric line, just print as-is
        mvwprintw(win, y, 2, "%.*s", max_w - 4, raw);
        return;
    }

    size_t key_len = eq - raw;
    if (key_len >= sizeof(key)) key_len = sizeof(key) - 1;
    strncpy(key, raw, key_len);

    const char* rest = eq + 1;
    const char* colon1 = strchr(rest, ':');
    if (colon1) {
        size_t val_len = colon1 - rest;
        if (val_len >= sizeof(value)) val_len = sizeof(value) - 1;
        strncpy(value, rest, val_len);

        const char* colon2 = strchr(colon1 + 1, ':');
        if (colon2) {
            strncpy(unit, colon2 + 1, sizeof(unit) - 1);
        }
    } else {
        strncpy(value, rest, sizeof(value) - 1);
    }

    // Pretty-print the key: replace dots and dashes with spaces, capitalize first letter
    char pretty_key[128] = {0};
    int ki = 0;
    bool cap_next = true;
    for (int i = 0; key[i] && ki < (int)sizeof(pretty_key) - 1; i++) {
        if (key[i] == '.' || key[i] == '_' || key[i] == '-') {
            pretty_key[ki++] = ' ';
            cap_next = true;
        } else if (cap_next) {
            pretty_key[ki++] = (key[i] >= 'a' && key[i] <= 'z') ? key[i] - 32 : key[i];
            cap_next = false;
        } else {
            pretty_key[ki++] = key[i];
            cap_next = false;
        }
    }

    // Render: "  Key Name            42 unit"
    wattron(win, A_BOLD);
    mvwprintw(win, y, 3, "%-20s", pretty_key);
    wattroff(win, A_BOLD);
    mvwprintw(win, y, 24, "%s %s", value, unit);
}

void PluginPaneComponent::render(const SystemSnapshot* snapshot) {
    werase(window);
    draw_border(has_focus_);
    
    char title[128];
    snprintf(title, sizeof(title), "%s%s [%s]", 
             expanded_ ? "v " : "> ", 
             plugin_name_.c_str(),
             enabled_ ? "ON" : "OFF");
    draw_title(title);
    
    if (!expanded_) {
        wnoutrefresh(window);
        return;
    }
    
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
        wattron(window, A_DIM);
        mvwprintw(window, 2, 3, "Plugin not found in backend.");
        wattroff(window, A_DIM);
        wnoutrefresh(window);
        return;
    }
    
    if (!p_inst->enabled) {
        wattron(window, A_DIM);
        mvwprintw(window, 2, 3, "Plugin is disabled. Press [e] to enable.");
        wattroff(window, A_DIM);
        wnoutrefresh(window);
        return;
    }

    if (p_inst->cached_output[0] == '\0') {
        wattron(window, A_DIM);
        mvwprintw(window, 2, 3, "Waiting for data...");
        wattroff(window, A_DIM);
        wnoutrefresh(window);
        return;
    }
    
    // Parse and render each metric line
    char temp[4096];
    snprintf(temp, sizeof(temp), "%s", p_inst->cached_output);
    char* line = strtok(temp, "\n");
    int y = 1;

    // Show pane name if present
    if (line && strncmp(line, "PANE:", 5) == 0) {
        wattron(window, COLOR_PAIR(2) | A_BOLD);
        mvwprintw(window, y++, 3, "%s", line + 5);
        wattroff(window, COLOR_PAIR(2) | A_BOLD);
        // Draw a thin separator
        mvwhline(window, y, 1, ACS_HLINE, width_ - 2);
        y++;
        line = strtok(NULL, "\n");
    }

    while (line && y < height_ - 1) {
        // Strip leading whitespace
        while (*line == ' ') line++;
        
        if (strncmp(line, "ENDPANE", 7) == 0) {
            line = strtok(NULL, "\n");
            continue;
        }

        render_metric_line(window, y++, width_, line);
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
    if (!expanded_) return 3;
    return 12;
}
