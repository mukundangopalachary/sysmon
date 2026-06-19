#include "components/process_table.h"
#include "sysmon_bridge.h"
#include <vector>
#include <algorithm>
#include <string>

ProcessTablePanel::ProcessTablePanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

ProcessTablePanel::~ProcessTablePanel() {}

void ProcessTablePanel::render(const SystemSnapshot* snapshot) {
    if (!snapshot || !snapshot->processes.processes) return;
    draw_border();
    draw_title("Processes");

    int h, w;
    getmaxyx(window, h, w);
    
    std::vector<const ProcessSnapshot*> procs;
    for (int i = 0; i < snapshot->processes.num_processes; ++i) {
        procs.push_back(&snapshot->processes.processes[i]);
    }

    std::sort(procs.begin(), procs.end(), [this](const ProcessSnapshot* a, const ProcessSnapshot* b) {
        bool result = false;
        switch (sort_column_) {
            case 0: result = a->pid < b->pid; break;
            case 1: result = a->cpu_percent < b->cpu_percent; break;
            case 2: result = a->mem_percent < b->mem_percent; break;
            case 3: {
                std::string u1 = a->username, u2 = b->username;
                result = u1 < u2;
                break;
            }
            default: result = a->pid < b->pid; break;
        }
        return sort_desc_ ? !result : result;
    });

    int max_rows = h - 3;
    if (selected_row_ >= (int)procs.size() && !procs.empty()) selected_row_ = procs.size() - 1;
    if (selected_row_ < scroll_offset_) scroll_offset_ = selected_row_;
    if (selected_row_ >= scroll_offset_ + max_rows) scroll_offset_ = selected_row_ - max_rows + 1;
    if (scroll_offset_ < 0) scroll_offset_ = 0;

    mvwprintw(window, 1, 1, "%-6s %-8s %-5s %-5s %-8s %-8s %-4s %-8s %s",
              "PID", "USER", "CPU%", "MEM%", "VSZ", "RSS", "STAT", "TIME", "COMMAND");

    int clock_ticks = (snapshot->system_info && snapshot->system_info->clock_ticks_per_sec > 0) 
                      ? snapshot->system_info->clock_ticks_per_sec : 100;

    for (int i = 0; i < max_rows && (scroll_offset_ + i) < (int)procs.size(); ++i) {
        int idx = scroll_offset_ + i;
        const ProcessSnapshot* p = procs[idx];
        if (idx == selected_row_) {
            wattron(window, A_REVERSE);
            selected_pid_ = p->pid;
        }
        
        int total_seconds = (p->utime_ticks + p->stime_ticks) / clock_ticks;
        char time_str[32];
        snprintf(time_str, sizeof(time_str), "%02d:%02d", total_seconds / 60, total_seconds % 60);

        std::string vsz = format_bytes(p->vsize_bytes);
        std::string rss = format_bytes(p->rss_bytes);

        mvwprintw(window, i + 2, 1, "%-6d %-8.8s %5.1f %5.1f %-8s %-8s %-4c %-8s %.30s",
                  p->pid, p->username[0] ? p->username : "root",
                  p->cpu_percent, p->mem_percent,
                  vsz.c_str(),
                  rss.c_str(),
                  p->state,
                  time_str,
                  p->comm);

        if (idx == selected_row_) {
            wattroff(window, A_REVERSE);
        }
    }
    wnoutrefresh(window);
}

bool ProcessTablePanel::handle_input(int key) {
    switch (key) {
        case '\n':
        case '\r':
        case KEY_ENTER:
            if (on_process_selected && selected_pid_ != -1) {
                on_process_selected(selected_pid_);
            }
            return true;
        case KEY_UP:
        case 'k':
            if (selected_row_ > 0) selected_row_--;
            return true;
        case KEY_DOWN:
        case 'j':
            selected_row_++; // Bound checking happens in render()
            return true;
        case '<':
            if (sort_column_ > 0) sort_column_--;
            return true;
        case '>':
            if (sort_column_ < 3) sort_column_++;
            return true;
        case 'r':
            sort_desc_ = !sort_desc_;
            return true;
    }
    return false;
}
