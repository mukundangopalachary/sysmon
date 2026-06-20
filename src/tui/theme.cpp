#include "theme.h"
#include <ncurses.h>

void ThemeManager::init() {
    if (!has_colors()) return;
    set_theme(current_theme_);
}

void ThemeManager::set_theme(const std::string& name) {
    current_theme_ = name;
    if (!has_colors()) return;

    // By default, use -1 for transparent background if terminal supports default colors
    int bg = (use_default_colors() == OK) ? -1 : COLOR_BLACK;
    
    if (name == "dracula") {
        init_pair(THEME_DEFAULT, COLOR_WHITE, bg);
        init_pair(THEME_BORDER, COLOR_MAGENTA, bg);
        init_pair(THEME_HEADER, COLOR_CYAN, bg);
        init_pair(THEME_HIGHLIGHT, COLOR_BLACK, COLOR_CYAN); // Inverted focus
        init_pair(THEME_CRITICAL, COLOR_RED, bg);
        init_pair(THEME_WARN, COLOR_YELLOW, bg);
        init_pair(THEME_GOOD, COLOR_GREEN, bg);
    } else if (name == "cyberpunk") {
        init_pair(THEME_DEFAULT, COLOR_CYAN, bg);
        init_pair(THEME_BORDER, COLOR_YELLOW, bg);
        init_pair(THEME_HEADER, COLOR_MAGENTA, bg);
        init_pair(THEME_HIGHLIGHT, COLOR_BLACK, COLOR_YELLOW);
        init_pair(THEME_CRITICAL, COLOR_RED, bg);
        init_pair(THEME_WARN, COLOR_MAGENTA, bg);
        init_pair(THEME_GOOD, COLOR_CYAN, bg);
    } else if (name == "monokai") {
        init_pair(THEME_DEFAULT, COLOR_WHITE, bg);
        init_pair(THEME_BORDER, COLOR_YELLOW, bg);
        init_pair(THEME_HEADER, COLOR_GREEN, bg);
        init_pair(THEME_HIGHLIGHT, COLOR_BLACK, COLOR_YELLOW);
        init_pair(THEME_CRITICAL, COLOR_RED, bg);
        init_pair(THEME_WARN, COLOR_MAGENTA, bg);
        init_pair(THEME_GOOD, COLOR_GREEN, bg);
    } else {
        // default
        init_pair(THEME_DEFAULT, COLOR_WHITE, bg);
        init_pair(THEME_BORDER, COLOR_WHITE, bg);
        init_pair(THEME_HEADER, COLOR_CYAN, bg);
        init_pair(THEME_HIGHLIGHT, COLOR_BLACK, COLOR_WHITE);
        init_pair(THEME_CRITICAL, COLOR_RED, bg);
        init_pair(THEME_WARN, COLOR_YELLOW, bg);
        init_pair(THEME_GOOD, COLOR_GREEN, bg);
    }
}
