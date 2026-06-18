#ifndef SYSMON_HEADER_PANEL_H
#define SYSMON_HEADER_PANEL_H
#include "components/panel.h"

class HeaderPanel : public Panel {
public:
    HeaderPanel(int height, int width, int start_y, int start_x);
    ~HeaderPanel() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;
};
#endif
