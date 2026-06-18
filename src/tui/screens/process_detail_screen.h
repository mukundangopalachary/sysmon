#ifndef SYSMON_PROCESS_DETAIL_SCREEN_H
#define SYSMON_PROCESS_DETAIL_SCREEN_H
#include "screens/screen.h"

class ProcessDetailScreen : public Screen {
public:
    ProcessDetailScreen();
    ~ProcessDetailScreen() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;
    void on_resize() override;
};
#endif
