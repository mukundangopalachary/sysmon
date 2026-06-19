#ifndef SYSMON_SCREEN_MANAGER_H
#define SYSMON_SCREEN_MANAGER_H
#include <string>
#include <unordered_map>
#include <memory>
#include "sysmon_bridge.h"
#include "screens/screen.h"

class ScreenManager {
public:
    void register_screen(const std::string& name, std::unique_ptr<Screen> screen);
    void switch_screen(const std::string& name);
    void render(const SystemSnapshot* snapshot);
    bool handle_input(int key);
    void on_resize();
private:
    std::unordered_map<std::string, std::unique_ptr<Screen>> screens_;
    Screen* current_screen_ = nullptr;
};
#endif
