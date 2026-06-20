#include "screen_manager.h"

void ScreenManager::register_screen(const std::string& name, std::unique_ptr<Screen> screen) {
    if (cfg_) screen->set_config(cfg_);
    screens_[name] = std::move(screen);
}

void ScreenManager::set_config(const SysmonConfig* cfg) {
    cfg_ = cfg;
    for (auto& pair : screens_) {
        pair.second->set_config(cfg);
    }
}

Screen* ScreenManager::get_screen(const std::string& name) {
    auto it = screens_.find(name);
    return it != screens_.end() ? it->second.get() : nullptr;
}

void ScreenManager::switch_screen(const std::string& name) {
    auto it = screens_.find(name);
    if (it != screens_.end()) {
        if (current_screen_) current_screen_->on_exit();
        
        // Clear stdscr to remove leftover windows from previous screen
        erase();
        refresh();
        
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
