#include "components/memory_panel.h"
#include <string>

MemoryPanel::MemoryPanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

MemoryPanel::~MemoryPanel() {}

void MemoryPanel::render(const SystemSnapshot* snapshot) {
    if (!snapshot) return;
    draw_border();
    draw_title("Memory");
    draw_footer("RAM & SWAP (Total/Used/Free)");

    // RAM
    mvwprintw(window, 2, 2, "RAM:");
    mvwprintw(window, 3, 4, "Total: %s", format_bytes(snapshot->memory.total_bytes));
    mvwprintw(window, 4, 4, "Used:  %s", format_bytes(snapshot->memory.used_bytes));
    mvwprintw(window, 5, 4, "Free:  %s", format_bytes(snapshot->memory.free_bytes));

    int bar_width = 20;
    int ram_filled = (int)(snapshot->memory.usage_percent / 100.0 * bar_width);
    std::string ram_bar(ram_filled, '#');
    std::string ram_empty(bar_width - ram_filled > 0 ? bar_width - ram_filled : 0, ' ');
    mvwprintw(window, 6, 4, "[%s%s] %.1f%%", ram_bar.c_str(), ram_empty.c_str(), snapshot->memory.usage_percent);

    // SWAP
    mvwprintw(window, 8, 2, "SWAP:");
    mvwprintw(window, 9, 4, "Total: %s", format_bytes(snapshot->memory.swap_total_bytes));
    mvwprintw(window, 10, 4, "Used:  %s", format_bytes(snapshot->memory.swap_used_bytes));
    mvwprintw(window, 11, 4, "Free:  %s", format_bytes(snapshot->memory.swap_free_bytes));

    int swap_filled = (int)(snapshot->memory.swap_usage_percent / 100.0 * bar_width);
    std::string swap_bar(swap_filled, '#');
    std::string swap_empty(bar_width - swap_filled > 0 ? bar_width - swap_filled : 0, ' ');
    mvwprintw(window, 12, 4, "[%s%s] %.1f%%", swap_bar.c_str(), swap_empty.c_str(), snapshot->memory.swap_usage_percent);

    wnoutrefresh(window);
}

bool MemoryPanel::handle_input(int /*key*/) { return false; }
