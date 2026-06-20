#ifndef SYSMON_PLUGIN_PANE_H
#define SYSMON_PLUGIN_PANE_H
#include "components/panel.h"
#include <string>

class PluginPaneComponent : public Panel {
public:
    PluginPaneComponent(int height, int width, int start_y, int start_x, const std::string& plugin_name);
    ~PluginPaneComponent() override;

    void render(const SystemSnapshot* snapshot) override;
    bool handle_input(int key) override;

    void set_expanded(bool expanded);
    bool is_expanded() const;

    const std::string& get_plugin_name() const;
    void set_enabled(bool enabled);
    bool is_enabled() const;

    // Helper to estimate height required when expanded
    int get_desired_height(const SystemSnapshot* snapshot) const;

private:
    std::string plugin_name_;
    bool expanded_ = true;
    bool enabled_ = true;
    
    // Parsed metrics from PluginData
    void parse_and_render_metrics(const char* data_buf);
};
#endif
