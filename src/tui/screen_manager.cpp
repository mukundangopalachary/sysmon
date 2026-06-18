#include "screen_manager.h"

void ScreenManager::switch_screen(const std::string&) {}
void ScreenManager::render(const SystemSnapshot*) {}
bool ScreenManager::handle_input(int) { return false; }
void ScreenManager::on_resize() {}
