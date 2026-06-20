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
    void set_config(const SysmonConfig* cfg);

    Screen* get_screen(const std::string& name);
    Screen* get_active_screen() { return current_screen_; }
private:
    std::unordered_map<std::string, std::unique_ptr<Screen>> screens_;
    Screen* current_screen_ = nullptr;
    const SysmonConfig* cfg_ = nullptr;
};
#endif
