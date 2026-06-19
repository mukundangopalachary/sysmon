#ifndef SYSMON_PROCESS_TABLE_PANEL_H
#define SYSMON_PROCESS_TABLE_PANEL_H
#include "components/panel.h"
#include <functional>

class ProcessTablePanel : public Panel {
public:
    ProcessTablePanel(int height, int width, int start_y, int start_x);
    ~ProcessTablePanel() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;

    std::function<void(int)> on_process_selected;

private:
    int selected_row_ = 0;
    int scroll_offset_ = 0;
    int sort_column_ = 0;
    bool sort_desc_ = true;
    int selected_pid_ = -1;
};
#endif
