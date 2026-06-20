#include "components/help_panel.h"
#include <ncurses.h>

HelpPanel::HelpPanel(int height, int width, int y, int x)
    : Panel(height, width, y, x) {
}

HelpPanel::~HelpPanel() {}

void HelpPanel::render(const SystemSnapshot* /*snapshot*/) {
    draw_border(false);
    draw_title("Keybindings");
    
    if (cfg_) {
        mvwprintw(window, 1, 2, "[%s] Dashboard  [%s] Processes  [%s] Network  [%s] Plugins  [%s] Quit",
                  cfg_->keybindings.dashboard,
                  cfg_->keybindings.process_list,
                  cfg_->keybindings.connections,
                  cfg_->keybindings.plugins,
                  cfg_->keybindings.quit);
    } else {
        mvwprintw(window, 1, 2, "No config loaded.");
    }
    wnoutrefresh(window);
}

bool HelpPanel::handle_input(int /*key*/) {
    return false;
}
