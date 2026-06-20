#ifndef SYSMON_CONNECTION_TABLE_PANEL_H
#define SYSMON_CONNECTION_TABLE_PANEL_H
#include "components/panel.h"
#include <string>
#include <functional>

class ConnectionTablePanel : public Panel {
public:
    ConnectionTablePanel(int height, int width, int start_y, int start_x);
    ~ConnectionTablePanel() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;

    std::function<void(int)> on_connection_selected;

private:
    int selected_row_ = 0;
    int scroll_offset_ = 0;
    int selected_pid_ = -1;
    int sort_column_ = 0; // 0=TYPE, 1=STATE, 2=LOCAL, 3=REMOTE, 4=PID, 5=INODE
    bool sort_desc_ = false;
    
    bool filter_mode_ = false;
    std::string filter_query_;
};
#endif
