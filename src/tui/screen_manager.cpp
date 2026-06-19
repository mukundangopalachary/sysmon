#include "screen_manager.h"

void ScreenManager::register_screen(const std::string& name, std::unique_ptr<Screen> screen) {
    screens_[name] = std::move(screen);
}

void ScreenManager::switch_screen(const std::string& name) {
    auto it = screens_.find(name);
    if (it != screens_.end()) {
        if (current_screen_) current_screen_->on_exit();
        current_screen_ = it->second.get();
        current_screen_->on_enter();
        current_screen_->on_resize();
    }
}

void ScreenManager::render(const SystemSnapshot* snap) {
    if (current_screen_ && snap) {
        current_screen_->render(snap);
    }
}

bool ScreenManager::handle_input(int key) {
    if (current_screen_) return current_screen_->handle_input(key);
    return false;
}

void ScreenManager::on_resize() {
    if (current_screen_) current_screen_->on_resize();
}
