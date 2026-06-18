#ifndef SYSMON_DISK_PANEL_H
#define SYSMON_DISK_PANEL_H
#include "components/panel.h"

class DiskPanel : public Panel {
public:
    DiskPanel(int height, int width, int start_y, int start_x);
    ~DiskPanel() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;
};
#endif
