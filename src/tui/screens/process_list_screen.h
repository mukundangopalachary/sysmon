#ifndef SYSMON_PROCESS_LIST_SCREEN_H
#define SYSMON_PROCESS_LIST_SCREEN_H
#include "screens/screen.h"
#include "components/process_table.h"
#include <memory>

class ProcessListScreen : public Screen {
public:
    ProcessListScreen();
    ~ProcessListScreen() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;
    void on_resize() override;
    
private:
    std::unique_ptr<ProcessTablePanel> table_panel_;
};
#endif
