#include "components/cpu_panel.h"
#include "plugin_manager.h"
#include <string.h>
#include <stdlib.h>

CpuPanel::CpuPanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

CpuPanel::~CpuPanel() {}

static int compare_process_cpu(const void* a, const void* b) {
    const ProcessSnapshot* pa = (const ProcessSnapshot*)a;
    const ProcessSnapshot* pb = (const ProcessSnapshot*)b;
    if (pa->cpu_percent < pb->cpu_percent) return 1;
    if (pa->cpu_percent > pb->cpu_percent) return -1;
    return 0;
}

void CpuPanel::render(const SystemSnapshot* snapshot) {
    if (!snapshot) return;
    
    // Calculate global CPU usage metrics
    double total_usage = 0.0, user_u = 0.0, sys_u = 0.0, io_u = 0.0, steal_u = 0.0;
    for (int i = 0; i < snapshot->cpu.num_cores; ++i) {
        total_usage += snapshot->cpu.cores[i].usage_percent;
        user_u += snapshot->cpu.cores[i].user_percent;
        sys_u += snapshot->cpu.cores[i].sys_percent;
        io_u += snapshot->cpu.cores[i].iowait_percent;
        steal_u += snapshot->cpu.cores[i].steal_percent;
    }
    if (snapshot->cpu.num_cores > 0) {
        total_usage /= snapshot->cpu.num_cores;
        user_u /= snapshot->cpu.num_cores;
        sys_u /= snapshot->cpu.num_cores;
        io_u /= snapshot->cpu.num_cores;
        steal_u /= snapshot->cpu.num_cores;
    }

    draw_border();
    draw_title("CPU");

    int h, w;
    getmaxyx(window, h, w);

    if (show_per_core_) {
        // PER-CORE MODE
        draw_footer(" [c] Overview ");
        int max_cores = h - 2;
        for (int i = 0; i < snapshot->cpu.num_cores && i < max_cores; ++i) {
            int usage = (int)snapshot->cpu.cores[i].usage_percent;
            int u_fill = ((int)snapshot->cpu.cores[i].user_percent * (w - 15)) / 100;
            int s_fill = ((int)snapshot->cpu.cores[i].sys_percent * (w - 15)) / 100;
            int bar_width = w - 15;
            if (bar_width < 5) bar_width = 5;

            mvwprintw(window, i + 1, 1, "CPU%-2d [", i);
            for (int j = 0; j < bar_width; ++j) {
                if (j < u_fill) waddch(window, '#');
                else if (j < u_fill + s_fill) waddch(window, '=');
                else waddch(window, ' ');
            }
            wprintw(window, "] %3d%%", usage);
        }
    } else {
        // OVERVIEW MODE
        draw_footer(" [c] Per-Core ");
        
        int line = 1;
        
        // 1. Overall Bar
        int bar_width = w - 12;
        if (bar_width < 5) bar_width = 5;
        int u_fill = ((int)user_u * bar_width) / 100;
        int s_fill = ((int)sys_u * bar_width) / 100;
        int i_fill = ((int)io_u * bar_width) / 100;
        
        mvwprintw(window, line++, 2, "AVG [");
        for (int j = 0; j < bar_width; ++j) {
            if (j < u_fill) waddch(window, '#');
            else if (j < u_fill + s_fill) waddch(window, '=');
            else if (j < u_fill + s_fill + i_fill) waddch(window, '-');
            else waddch(window, ' ');
        }
        wprintw(window, "] %3d%%", (int)total_usage);
        
        mvwprintw(window, line++, 4, "User: %3.1f%% | Sys: %3.1f%% | IO: %3.1f%% | St: %3.1f%%",
            user_u, sys_u, io_u, steal_u);

        
        line++; // Blank line
        
        // 2. Load Averages & Rates
        mvwprintw(window, line++, 2, "Load Avg : %.2f %.2f %.2f (Cores: %d)", 
            snapshot->cpu.load_avg_1min, snapshot->cpu.load_avg_5min, snapshot->cpu.load_avg_15min, snapshot->cpu.num_cores);
        
        mvwprintw(window, line++, 2, "Ctx Sw   : %llu/s", (unsigned long long)snapshot->cpu.ctxt_rate);
        mvwprintw(window, line++, 2, "Interrupts: %llu/s", (unsigned long long)snapshot->cpu.irq_rate);
        
        line++; // Blank line
        
        // 3. Top 3 CPU Consumers
        if (line + 4 < h) {
            mvwprintw(window, line++, 2, "Top CPU Consumers:");
            
            // Sort processes by CPU (need a mutable copy of pointers or array)
            int num_procs = snapshot->processes.num_processes;
            ProcessSnapshot* sorted_procs = (ProcessSnapshot*)malloc(num_procs * sizeof(ProcessSnapshot));
            if (sorted_procs) {
                memcpy(sorted_procs, snapshot->processes.processes, num_procs * sizeof(ProcessSnapshot));
                qsort(sorted_procs, num_procs, sizeof(ProcessSnapshot), compare_process_cpu);
                
                for (int i = 0; i < 3 && i < num_procs; ++i) {
                    mvwprintw(window, line++, 4, "%-15s %5d %5.1f%%", 
                        sorted_procs[i].comm, sorted_procs[i].pid, sorted_procs[i].cpu_percent);
                }
                free(sorted_procs);
            }
        }
        // 4. Thermal Integration
        if (snapshot->plugin_data && line < h) {
            PluginData* pdata = (PluginData*)snapshot->plugin_data;
            for (int i = 0; i < pdata->num_metrics; ++i) {
                const char* m = pdata->metrics[i];
                if (strncmp(m, "cpu.temperature=", 16) == 0) {
                    int temp = atoi(m + 16);
                    int fill = (temp * 10) / 100;
                    if (fill > 10) fill = 10;
                    mvwprintw(window, line++, 2, "Thermal: %d°C [", temp);
                    for(int j=0; j<10; ++j) {
                        waddch(window, j < fill ? '#' : ' ');
                    }
                    wprintw(window, "] (Max 100°C)");
                    break;
                }
            }
        }
    }

    wnoutrefresh(window);
}

bool CpuPanel::handle_input(int key) { 
    if (key == 'c' || key == 'C') {
        show_per_core_ = !show_per_core_;
        return true;
    }
    return false; 
}
