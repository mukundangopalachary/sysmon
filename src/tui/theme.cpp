#include "theme.h"
#include <ncurses.h>

void ThemeManager::init() {
    init_pair(1, COLOR_CYAN, -1);
    init_pair(2, COLOR_GREEN, -1);
    init_pair(3, COLOR_YELLOW, -1);
    init_pair(4, COLOR_RED, -1);
}

void ThemeManager::set_theme(const std::string&) {}
