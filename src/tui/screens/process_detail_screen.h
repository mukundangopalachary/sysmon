#ifndef SYSMON_PROCESS_DETAIL_SCREEN_H
#define SYSMON_PROCESS_DETAIL_SCREEN_H
#include "screens/screen.h"
#include "components/process_detail.h"
#include <memory>
#include <functional>

class ProcessDetailScreen : public Screen {
public:
    ProcessDetailScreen();
    ~ProcessDetailScreen() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;
    void on_resize() override;

    void open(int pid);
    std::function<void()> on_close;

private:
    std::unique_ptr<ProcessDetailPanel> detail_panel_;
};
#endif
