#ifndef SYSMON_CONNECTION_SCREEN_H
#define SYSMON_CONNECTION_SCREEN_H
#include "screens/screen.h"
#include "components/connection_table.h"
#include "components/header.h"
#include "components/help_panel.h"
#include <memory>
#include <functional>

class ConnectionScreen : public Screen {
public:
    ConnectionScreen();
    ~ConnectionScreen() override;
    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;
    void on_resize() override;
    void set_config(const SysmonConfig* cfg) override;

    std::function<void(int)> on_connection_selected;

private:
    std::unique_ptr<HeaderPanel> header_panel_;
    std::unique_ptr<ConnectionTablePanel> table_panel_;
    std::unique_ptr<HelpPanel> help_panel_;
};
#endif
