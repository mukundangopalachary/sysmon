#ifndef SYSMON_PROCESS_DETAIL_PANEL_H
#define SYSMON_PROCESS_DETAIL_PANEL_H
#include "components/panel.h"

class ProcessDetailPanel : public Panel {
public:
    ProcessDetailPanel(int height, int width, int start_y, int start_x);
    ~ProcessDetailPanel() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;
};
#endif
