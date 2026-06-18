#ifndef SYSMON_MEMORY_PANEL_H
#define SYSMON_MEMORY_PANEL_H
#include "components/panel.h"

class MemoryPanel : public Panel {
public:
    MemoryPanel(int height, int width, int start_y, int start_x);
    ~MemoryPanel() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;
};
#endif
