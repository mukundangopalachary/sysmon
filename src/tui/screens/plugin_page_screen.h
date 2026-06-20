#ifndef SYSMON_PLUGIN_PAGE_SCREEN_H
#define SYSMON_PLUGIN_PAGE_SCREEN_H
#include "screens/screen.h"
#include "components/header.h"
#include "components/help_panel.h"
#include "components/plugin_pane.h"
#include <vector>

class PluginPageScreen : public Screen {
public:
    PluginPageScreen();
    ~PluginPageScreen() override;

    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;
    void on_resize() override;

private:
    std::unique_ptr<HeaderPanel> header_panel_;
    std::unique_ptr<HelpPanel> help_panel_;
    
    // Each plugin gets a PluginPaneComponent
    std::vector<std::unique_ptr<PluginPaneComponent>> plugin_panes_;
    
    int selected_plugin_idx_ = 0;
    int scroll_offset_ = 0;
    void* pm_ = nullptr; // Pointer to live PluginManager (casted to void* to avoid header dependency)
};
#endif
