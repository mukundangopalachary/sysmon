#ifndef SYSMON_PROCESS_LIST_SCREEN_H
#define SYSMON_PROCESS_LIST_SCREEN_H
#include "screens/screen.h"
#include "components/process_table.h"
#include "components/header.h"
#include "components/help_panel.h"
#include <memory>
#include <functional>

class ProcessListScreen : public Screen {
public:
    ProcessListScreen();
    ~ProcessListScreen() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;
    void on_resize() override;
    
    std::function<void(int)> on_process_selected;

private:
    std::unique_ptr<HeaderPanel> header_panel_;
    std::unique_ptr<ProcessTablePanel> table_panel_;
    std::unique_ptr<HelpPanel> help_panel_;
};
#endif
