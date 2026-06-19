#ifndef SYSMON_CONNECTION_TABLE_PANEL_H
#define SYSMON_CONNECTION_TABLE_PANEL_H
#include "components/panel.h"

class ConnectionTablePanel : public Panel {
public:
    ConnectionTablePanel(int height, int width, int start_y, int start_x);
    ~ConnectionTablePanel() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;

private:
    int selected_row_ = 0;
    int scroll_offset_ = 0;
};
#endif
