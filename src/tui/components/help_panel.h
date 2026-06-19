#ifndef SYSMON_HELP_PANEL_H
#define SYSMON_HELP_PANEL_H

#include "components/panel.h"

class HelpPanel : public Panel {
public:
    HelpPanel(int height, int width, int start_y, int start_x);
    virtual ~HelpPanel();
    
    virtual void render(const SystemSnapshot* snapshot) override;
    virtual bool handle_input(int key) override;
};

#endif // SYSMON_HELP_PANEL_H
