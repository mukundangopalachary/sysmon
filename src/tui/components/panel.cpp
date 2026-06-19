#include "components/panel.h"

Panel::Panel(int height, int width, int start_y, int start_x)
    : window(nullptr), height_(height), width_(width),
      start_y_(start_y), start_x_(start_x), has_focus_(false) {
    if (height > 0 && width > 0) {
        window = newwin(height, width, start_y, start_x);
    }
}

Panel::~Panel() {
    if (window != nullptr) {
        delwin(window);
    }
}

void Panel::on_focus() { has_focus_ = true; }
void Panel::on_blur() { has_focus_ = false; }

void Panel::on_resize(int height, int width, int start_y, int start_x) {
    height_ = height;
    width_ = width;
    start_y_ = start_y;
    start_x_ = start_x;
    if (window != nullptr) {
        delwin(window);
        window = nullptr;
    }
    if (height > 0 && width > 0) {
        window = newwin(height, width, start_y, start_x);
    }
}

bool Panel::handle_input(int /*key*/) { return false; }

void Panel::draw_border(bool /*focused*/) {
    if (window) {
        box(window, 0, 0);
    }
}

void Panel::draw_title(const char* title) {
    if (window && title) {
        mvwprintw(window, 0, 2, " %s ", title);
    }
}

void Panel::draw_footer(const char* text) {
    if (window && text && height_ > 2) {
        mvwprintw(window, height_ - 1, 2, " %s ", text);
    }
}

void Panel::draw_top_right(const char* text) {
    if (window && text && width_ > 10) {
        int len = 0;
        for (const char* p = text; *p; p++) len++;
        int start_x = width_ - len - 3;
        if (start_x < 2) start_x = 2;
        mvwprintw(window, 0, start_x, " %s ", text);
    }
}

void Panel::clear_content() {
    if (window) {
        werase(window);
    }
}
