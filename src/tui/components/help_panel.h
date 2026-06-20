#ifndef SYSMON_HELP_PANEL_H
#define SYSMON_HELP_PANEL_H

#include "components/panel.h"
#include "config_parser.h"

class HelpPanel : public Panel {
public:
    HelpPanel(int height, int width, int y, int x);
    ~HelpPanel() override;

    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;
    
    void set_config(const SysmonConfig* cfg) { cfg_ = cfg; }
private:
    const SysmonConfig* cfg_ = nullptr;
};

#endif // SYSMON_HELP_PANEL_H
