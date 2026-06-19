#include "components/process_table.h"
#include <string.h>
#include <stdlib.h>
#include <vector>

ProcessTablePanel::ProcessTablePanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

ProcessTablePanel::~ProcessTablePanel() {}

static const char* format_state(char state) {
    switch (state) {
        case 'R': return "Run";
        case 'S': return "Slp";
        case 'D': return "DSp";
        case 'Z': return "Zmb";
        case 'T': return "Stp";
        case 't': return "TrC";
        case 'X': return "Ded";
        case 'x': return "Ded";
        case 'K': return "Wkl";
        case 'W': return "Wak";
        case 'P': return "Prk";
        case 'I': return "Idl";
        default: return "???";
    }
}

void ProcessTablePanel::render(const SystemSnapshot* snapshot) {
    if (!snapshot) return;
    
    draw_border(has_focus_);
    draw_title("Processes");
    
    int h, w;
    getmaxyx(window, h, w);
    
    // Copy and filter/sort the processes
    int total_procs = snapshot->processes.num_processes;
    std::vector<ProcessSnapshot> procs;
    procs.reserve(total_procs);
    
    for (int i = 0; i < total_procs; ++i) {
        if (process_matches_filter(&snapshot->processes.processes[i], filter_query_.c_str())) {
            procs.push_back(snapshot->processes.processes[i]);
        }
    }
    
    // Sort
    ProcessTableSnapshot temp_table;
    temp_table.processes = procs.data();
    temp_table.num_processes = procs.size();
    temp_table.capacity = procs.capacity();
    process_sort_table(&temp_table, sort_column_, sort_order_);
    
    // Draw Header
    const char* cols[] = {"PID", "USER", "CPU%", "MEM", "STATE", "TIME", "COMMAND"};
    SortColumn col_enums[] = {SORT_COL_PID, SORT_COL_USER, SORT_COL_CPU, SORT_COL_MEM, SORT_COL_STATE, SORT_COL_TIME, SORT_COL_CMD};
    int col_widths[] = {8, 12, 7, 10, 6, 10, w - 53};
    if (col_widths[6] < 10) col_widths[6] = 10;
    
    wattron(window, A_BOLD | A_REVERSE);
    int x = 1;
    for (int i = 0; i < 7; ++i) {
        char header[64];
        if (sort_column_ == col_enums[i]) {
            snprintf(header, sizeof(header), "%s[%s]", cols[i], sort_order_ == SORT_ORDER_ASC ? "^" : "v");
        } else {
            snprintf(header, sizeof(header), "%s", cols[i]);
        }
        mvwprintw(window, 1, x, "%-*.*s", col_widths[i], col_widths[i], header);
        x += col_widths[i];
    }
    wattroff(window, A_BOLD | A_REVERSE);
    
    // Draw rows
    int visible_rows = h - 4; // border top, header, border bottom, footer
    if (filter_mode_) visible_rows--; // space for search bar
    
    // Adjust scroll
    if (selected_row_ >= (int)procs.size()) selected_row_ = procs.size() - 1;
    if (selected_row_ < 0) selected_row_ = 0;
    
    if (selected_row_ < scroll_offset_) scroll_offset_ = selected_row_;
    if (selected_row_ >= scroll_offset_ + visible_rows) scroll_offset_ = selected_row_ - visible_rows + 1;
    
    for (int i = 0; i < visible_rows; ++i) {
        int idx = scroll_offset_ + i;
        if (idx >= (int)procs.size()) {
            mvwprintw(window, i + 2, 1, "%*s", w - 2, ""); // clear line
            continue;
        }
        
        if (idx == selected_row_ && has_focus_) {
            wattron(window, A_REVERSE);
        }
        
        const auto& p = procs[idx];
        if (idx == selected_row_) {
            selected_pid_ = p.pid;
        }
        
        // Format memory
        char mem_str[32];
        if (p.rss_bytes > 1024*1024*1024) snprintf(mem_str, sizeof(mem_str), "%.1fG", p.rss_bytes / (1024.0*1024.0*1024.0));
        else if (p.rss_bytes > 1024*1024) snprintf(mem_str, sizeof(mem_str), "%.1fM", p.rss_bytes / (1024.0*1024.0));
        else snprintf(mem_str, sizeof(mem_str), "%.1fK", p.rss_bytes / 1024.0);
        
        // Format time
        char time_str[32];
        uint64_t total_sec = (p.utime_ticks + p.stime_ticks) / (snapshot->system_info->clock_ticks_per_sec > 0 ? snapshot->system_info->clock_ticks_per_sec : 100);
        snprintf(time_str, sizeof(time_str), "%02llu:%02llu.%02llu", 
            (unsigned long long)(total_sec / 60), 
            (unsigned long long)(total_sec % 60), 
            (unsigned long long)((p.utime_ticks + p.stime_ticks) % 100));

        mvwprintw(window, i + 2, 1, "%-8d%-12.11s%6.1f%% %-9s %-5s %-9s %-.*s",
            p.pid, p.username, p.cpu_percent, mem_str, format_state(p.state), time_str, col_widths[6], p.comm);
            
        if (idx == selected_row_ && has_focus_) {
            wattroff(window, A_REVERSE);
        }
    }
    
    // Draw Footer / Search Bar
    if (filter_mode_) {
        mvwprintw(window, h - 2, 1, "Search: %s_", filter_query_.c_str());
    } else {
        char footer[128];
        snprintf(footer, sizeof(footer), " Processes: %d/%d | </> Sort | i Invert | / Search ", (int)procs.size(), total_procs);
        draw_footer(footer);
    }
    
    wnoutrefresh(window);
}

bool ProcessTablePanel::handle_input(int key) {
    if (filter_mode_) {
        if (key == '\n' || key == KEY_ENTER || key == 27) { // 27 is ESC
            filter_mode_ = false;
        } else if (key == KEY_BACKSPACE || key == 127 || key == '\b') {
            if (!filter_query_.empty()) filter_query_.pop_back();
        } else if (key >= 32 && key <= 126) {
            filter_query_ += (char)key;
        }
        selected_row_ = 0; // reset selection on search
        return true;
    }

    switch (key) {
        case KEY_UP:
        case 'k':
            if (selected_row_ > 0) selected_row_--;
            return true;
        case KEY_DOWN:
        case 'j':
            selected_row_++; // Bound check in render
            return true;
        case KEY_PPAGE:
            selected_row_ -= 10;
            if (selected_row_ < 0) selected_row_ = 0;
            return true;
        case KEY_NPAGE:
            selected_row_ += 10;
            return true;
        case '<':
        case ',':
            sort_column_ = static_cast<SortColumn>((sort_column_ - 1 + 7) % 7);
            return true;
        case '>':
        case '.':
            sort_column_ = static_cast<SortColumn>((sort_column_ + 1) % 7);
            return true;
        case 'i':
        case 'I':
            sort_order_ = (sort_order_ == SORT_ORDER_ASC) ? SORT_ORDER_DESC : SORT_ORDER_ASC;
            return true;
        case '/':
            filter_mode_ = true;
            return true;
        case '\n':
        case KEY_ENTER:
            if (on_process_selected && selected_pid_ != -1) {
                on_process_selected(selected_pid_);
            }
            return true;
    }
    return false;
}
