#include "components/plugin_panel.h"
#include "plugin_manager.h"
#include <ncurses.h>

PluginPanel::PluginPanel(int height, int width, int y, int x)
    : Panel(height, width, y, x) {
}

PluginPanel::~PluginPanel() {}

void PluginPanel::render(const SystemSnapshot* snapshot) {
    draw_border(false);
    draw_title("Plugins");
    draw_footer("External script data");
    
    if (!snapshot || !snapshot->plugin_data) {
        mvwprintw(window, 1, 2, "No plugin data");
        wnoutrefresh(window);
        return;
    }
    
    PluginData* pdata = (PluginData*)snapshot->plugin_data;
    
    for (int i = 0; i < pdata->num_metrics && i < height_ - 2; i++) {
        mvwprintw(window, 1 + i, 2, "%.*s", width_ - 4, pdata->metrics[i]);
    }
    
    wnoutrefresh(window);
}
