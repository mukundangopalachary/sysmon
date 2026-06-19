#include "components/header.h"

HeaderPanel::HeaderPanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

HeaderPanel::~HeaderPanel() {}

void HeaderPanel::render(const SystemSnapshot* snapshot) {
    if (!window || !snapshot || !snapshot->system_info) return;
    
    clear_content();
    
    uint64_t uptime_sec = snapshot->collection_timestamp_us / 1000000;
    
    mvwprintw(window, 0, 0, "Hostname: %s | OS: %s | Uptime: %s",
              snapshot->system_info->hostname,
              snapshot->system_info->kernel_release,
              format_duration_us(uptime_sec * 1000000));
              
    mvwprintw(window, 1, 0, "Load Average: %.2f %.2f %.2f",
              snapshot->cpu.load_avg_1min,
              snapshot->cpu.load_avg_5min,
              snapshot->cpu.load_avg_15min);
              
    wnoutrefresh(window);
}

bool HeaderPanel::handle_input(int /*key*/) { return false; }
