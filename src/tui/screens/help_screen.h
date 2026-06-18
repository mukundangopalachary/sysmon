#ifndef SYSMON_HELP_SCREEN_H
#define SYSMON_HELP_SCREEN_H
#include "screens/screen.h"

class HelpScreen : public Screen {
public:
    HelpScreen();
    ~HelpScreen() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;
    void on_resize() override;
};
#endif
