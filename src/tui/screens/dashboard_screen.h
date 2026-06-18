#ifndef SYSMON_DASHBOARD_SCREEN_H
#define SYSMON_DASHBOARD_SCREEN_H
#include "screens/screen.h"

class DashboardScreen : public Screen {
public:
    DashboardScreen();
    ~DashboardScreen() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;
    void on_resize() override;
};
#endif
