#ifndef SYSMON_CONNECTION_SCREEN_H
#define SYSMON_CONNECTION_SCREEN_H
#include "screens/screen.h"

class ConnectionScreen : public Screen {
public:
    ConnectionScreen();
    ~ConnectionScreen() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;
    void on_resize() override;
};
#endif
