#ifndef SYSMON_MINI_PROCESS_PANEL_H
#define SYSMON_MINI_PROCESS_PANEL_H

#include "components/panel.h"
#include "process_sort.h"

class MiniProcessPanel : public Panel {
public:
    MiniProcessPanel(int height, int width, int start_y, int start_x);
    ~MiniProcessPanel() override;
    void render(const SystemSnapshot* snapshot) override;
};

#endif
