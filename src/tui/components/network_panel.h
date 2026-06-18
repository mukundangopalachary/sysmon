#ifndef SYSMON_NETWORK_PANEL_H
#define SYSMON_NETWORK_PANEL_H
#include "components/panel.h"

class NetworkPanel : public Panel {
public:
    NetworkPanel(int height, int width, int start_y, int start_x);
    ~NetworkPanel() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;
};
#endif
