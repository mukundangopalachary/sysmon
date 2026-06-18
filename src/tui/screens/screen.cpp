#include "screens/screen.h"

Screen::~Screen() {}
void Screen::on_enter() { visible_ = true; }
void Screen::on_exit() { visible_ = false; }
void Screen::on_resize() {}
void Screen::set_visible(bool visible) { visible_ = visible; }
bool Screen::is_visible() const { return visible_; }
