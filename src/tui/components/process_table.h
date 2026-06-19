#ifndef SYSMON_PROCESS_TABLE_PANEL_H
#define SYSMON_PROCESS_TABLE_PANEL_H
#include "components/panel.h"
#include <functional>

#include "process_sort.h"
#include <string>

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
    SortColumn sort_column_ = SORT_COL_CPU;
    SortOrder sort_order_ = SORT_ORDER_DESC;
    int selected_pid_ = -1;
    
    bool filter_mode_ = false;
    std::string filter_query_;
};
#endif
