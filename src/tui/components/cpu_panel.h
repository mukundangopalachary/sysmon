#ifndef SYSMON_CPU_PANEL_H
#define SYSMON_CPU_PANEL_H
#include "components/panel.h"

class CpuPanel : public Panel {
public:
    CpuPanel(int height, int width, int start_y, int start_x);
    ~CpuPanel() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;
private:
    bool show_per_core_ = false;
};
#endif
