#include "components/help_panel.h"
#include <ncurses.h>

HelpPanel::HelpPanel(int height, int width, int y, int x)
    : Panel(height, width, y, x) {
}

HelpPanel::~HelpPanel() {}

void HelpPanel::render(const SystemSnapshot* /*snapshot*/) {
    draw_border(false);
    draw_title("Keybindings");
    
    mvwprintw(window, 1, 2, "[d] Dashboard  [p] Process List  [n] Connections  [^P] Plugins  [q] Quit");
    wrefresh(window);
}

bool HelpPanel::handle_input(int /*key*/) {
    return false;
}
