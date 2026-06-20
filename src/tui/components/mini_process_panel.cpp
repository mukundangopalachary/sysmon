#include "components/mini_process_panel.h"
#include "theme.h"
#include <ncurses.h>
#include <vector>
#include <algorithm>

MiniProcessPanel::MiniProcessPanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

MiniProcessPanel::~MiniProcessPanel() {}

void MiniProcessPanel::render(const SystemSnapshot* snapshot) {
    draw_border(false);
    draw_title("Top Processes (CPU)");
    draw_footer("Highest consumers");
    
    if (!snapshot || snapshot->processes.num_processes == 0) {
        mvwprintw(window, 1, 2, "No processes found.");
        wnoutrefresh(window);
        return;
    }
    
    // Create a vector of pointers to sort them
    std::vector<const ProcessSnapshot*> procs;
    procs.reserve(snapshot->processes.num_processes);
    for (int i = 0; i < snapshot->processes.num_processes; i++) {
        procs.push_back(&snapshot->processes.processes[i]);
    }
    
    // Sort by CPU percent descending
    std::sort(procs.begin(), procs.end(), [](const ProcessSnapshot* a, const ProcessSnapshot* b) {
        return a->cpu_percent > b->cpu_percent;
    });
    
    // Draw header
    wattron(window, A_BOLD | COLOR_PAIR(THEME_HEADER));
    mvwprintw(window, 1, 1, " %-8s %-12s %5s ", "PID", "USER", "CPU%");
    wattroff(window, A_BOLD | COLOR_PAIR(THEME_HEADER));
    
    int max_rows = height_ - 3;
    if (max_rows <= 0) {
        wnoutrefresh(window);
        return;
    }
    
    for (int i = 0; i < max_rows && i < (int)procs.size(); i++) {
        const ProcessSnapshot* p = procs[i];
        
        char user_str[32];
        snprintf(user_str, sizeof(user_str), "%s", p->username[0] != '\0' ? p->username : "unknown");
        user_str[12] = '\0'; // Truncate manually for display
        
        mvwprintw(window, 2 + i, 1, " %-8d %-12s %5.1f%% ", 
                  p->pid, user_str, p->cpu_percent);
    }
    
    wnoutrefresh(window);
}
