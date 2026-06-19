#ifndef SYSMON_TUI_PLUGIN_PANEL_H
#define SYSMON_TUI_PLUGIN_PANEL_H

#include "components/panel.h"

class PluginPanel : public Panel {
public:
    PluginPanel(int height, int width, int y, int x);
    ~PluginPanel() override;

    void render(const SystemSnapshot* snapshot) override;
};

#endif // SYSMON_TUI_PLUGIN_PANEL_H
