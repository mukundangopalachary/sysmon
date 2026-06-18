#include "components/panel.h"

Panel::Panel(int height, int width, int start_y, int start_x)
    : window(nullptr), height_(height), width_(width),
      start_y_(start_y), start_x_(start_x), has_focus_(false) {}

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
}

bool Panel::handle_input(int /*key*/) { return false; }
void Panel::draw_border(bool /*focused*/) {}
void Panel::draw_title(const char* /*title*/) {}
void Panel::clear_content() {}
