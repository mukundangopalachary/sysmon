#ifndef SYSMON_SCREEN_MANAGER_H
#define SYSMON_SCREEN_MANAGER_H
#include <string>
#include "sysmon_bridge.h"

class ScreenManager {
public:
    void switch_screen(const std::string& name);
    void render(const SystemSnapshot* snapshot);
    bool handle_input(int key);
    void on_resize();
};
#endif
