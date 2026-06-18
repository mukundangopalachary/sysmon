#ifndef SYSMON_PROCESS_TABLE_PANEL_H
#define SYSMON_PROCESS_TABLE_PANEL_H
#include "components/panel.h"

class ProcessTablePanel : public Panel {
public:
    ProcessTablePanel(int height, int width, int start_y, int start_x);
    ~ProcessTablePanel() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;
};
#endif
