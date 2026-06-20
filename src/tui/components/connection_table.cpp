#include "components/connection_table.h"
#include <string.h>
#include <ctype.h>
#include <vector>
#include <algorithm>

ConnectionTablePanel::ConnectionTablePanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {
    has_focus_ = true;
}

ConnectionTablePanel::~ConnectionTablePanel() {}

static const char* format_conn_type(ConnectionType t) {
    switch(t) {
        case CONN_TCP: return "TCP";
        case CONN_TCP6: return "TCP6";
        case CONN_UDP: return "UDP";
        case CONN_UDP6: return "UDP6";
        case CONN_UNIX: return "UNIX";
        default: return "???";
    }
}

static const char* format_conn_state(ConnectionState s) {
    switch(s) {
        case CONN_ESTABLISHED: return "ESTABLISHED";
        case CONN_LISTEN: return "LISTEN";
        case CONN_TIME_WAIT: return "TIME_WAIT";
        case CONN_CLOSE_WAIT: return "CLOSE_WAIT";
        case CONN_SYN_SENT: return "SYN_SENT";
        case CONN_OTHER: return "OTHER";
        default: return "???";
    }
}

// Helper for case-insensitive substring search
static bool stristr_local(const char* haystack, const char* needle) {
    if (!*needle) return true;
    for (; *haystack; ++haystack) {
        if (toupper(*haystack) == toupper(*needle)) {
            const char *h, *n;
            for (h = haystack, n = needle; *h && *n; ++h, ++n) {
                if (toupper(*h) != toupper(*n)) break;
            }
            if (!*n) return true;
        }
    }
    return false;
}

void ConnectionTablePanel::render(const SystemSnapshot* snapshot) {
    if (!snapshot || !snapshot->connections) return;
    
    draw_border(has_focus_);
    draw_title("Network Connections");
    
    int h, w;
    getmaxyx(window, h, w);

    int total_conns = snapshot->connections->num_connections;
    std::vector<ConnectionSnapshot> conns;
    conns.reserve(total_conns);

    // Filter
    for (int i = 0; i < total_conns; ++i) {
        const auto& c = snapshot->connections->connections[i];
        if (filter_query_.empty() || 
            stristr_local(c.local_addr, filter_query_.c_str()) || 
            stristr_local(c.remote_addr, filter_query_.c_str()) ||
            stristr_local(format_conn_type(c.type), filter_query_.c_str()) ||
            stristr_local(format_conn_state(c.state), filter_query_.c_str())) 
        {
            conns.push_back(c);
        }
    }

    // Sort
    int col = sort_column_;
    bool desc = sort_desc_;
    std::sort(conns.begin(), conns.end(), [col, desc](const ConnectionSnapshot& a, const ConnectionSnapshot& b) {
        int cmp = 0;
        switch (col) {
            case 0: cmp = strcmp(format_conn_type(a.type), format_conn_type(b.type)); break;
            case 1: cmp = strcmp(format_conn_state(a.state), format_conn_state(b.state)); break;
            case 2: cmp = strcmp(a.local_addr, b.local_addr); break;
            case 3: cmp = strcmp(a.remote_addr, b.remote_addr); break;
            case 4: cmp = a.pid - b.pid; break;
            case 5: cmp = (a.inode < b.inode) ? -1 : (a.inode > b.inode ? 1 : 0); break;
        }
        if (cmp == 0) cmp = (a.inode < b.inode) ? -1 : (a.inode > b.inode ? 1 : 0);
        return desc ? (cmp > 0) : (cmp < 0);
    });

    const char* cols[] = {"TYPE", "STATE", "LOCAL", "REMOTE", "PID", "INODE"};
    int col_widths[] = {6, 12, 22, 22, 8, w - 74};
    if (col_widths[5] < 10) col_widths[5] = 10;
    
    wattron(window, A_BOLD | A_REVERSE);
    int x = 1;
    for (int i = 0; i < 6; ++i) {
        char header[64];
        if (sort_column_ == i) {
            snprintf(header, sizeof(header), "%s[%s]", cols[i], sort_desc_ ? "v" : "^");
        } else {
            snprintf(header, sizeof(header), "%s", cols[i]);
        }
        mvwprintw(window, 1, x, "%-*.*s", col_widths[i], col_widths[i], header);
        x += col_widths[i];
    }
    wattroff(window, A_BOLD | A_REVERSE);
    
    int visible_rows = h - 4;
    if (filter_mode_) visible_rows--;

    if (selected_row_ >= (int)conns.size()) selected_row_ = conns.size() - 1;
    if (selected_row_ < 0) selected_row_ = 0;
    
    if (selected_row_ < scroll_offset_) scroll_offset_ = selected_row_;
    if (selected_row_ >= scroll_offset_ + visible_rows) scroll_offset_ = selected_row_ - visible_rows + 1;
    
    for (int i = 0; i < visible_rows; ++i) {
        int idx = scroll_offset_ + i;
        if (idx >= (int)conns.size()) {
            mvwprintw(window, i + 2, 1, "%*s", w - 2, "");
            continue;
        }
        
        if (idx == selected_row_ && has_focus_) wattron(window, A_REVERSE);
        
        const auto& c = conns[idx];
        if (idx == selected_row_) {
            selected_pid_ = c.pid;
        }

        char pid_str[16] = "-";
        if (c.pid > 0) snprintf(pid_str, sizeof(pid_str), "%d", c.pid);
        
        mvwprintw(window, i + 2, 1, "%-6.5s%-12.11s%-22.21s%-22.21s%-8s%-llu",
            format_conn_type(c.type), format_conn_state(c.state),
            c.local_addr, c.remote_addr, pid_str, (unsigned long long)c.inode);
            
        if (idx == selected_row_ && has_focus_) wattroff(window, A_REVERSE);
    }
    
    if (filter_mode_) {
        mvwprintw(window, h - 2, 1, "Search: %s_", filter_query_.c_str());
    } else {
        char footer[128];
        snprintf(footer, sizeof(footer), " Conns: %d/%d | <-/-> Sort | i Invert | / Search ", (int)conns.size(), total_conns);
        draw_footer(footer);
    }
    
    wnoutrefresh(window);
}

bool ConnectionTablePanel::handle_input(int key) {
    if (filter_mode_) {
        if (key == '\n' || key == KEY_ENTER) {
            filter_mode_ = false;
        } else if (key == 27) { // ESC
            filter_mode_ = false;
            filter_query_.clear();
        } else if (key == KEY_BACKSPACE || key == 127 || key == '\b') {
            if (!filter_query_.empty()) filter_query_.pop_back();
        } else if (key >= 32 && key <= 126) {
            filter_query_ += (char)key;
        }
        selected_row_ = 0;
        return true;
    }

    switch (key) {
        case '\n':
        case '\r':
        case KEY_ENTER:
            if (on_connection_selected && selected_pid_ > 0) {
                on_connection_selected(selected_pid_);
            }
            return true;
        case KEY_UP:
        case 'k':
            if (selected_row_ > 0) selected_row_--;
            return true;
        case KEY_DOWN:
        case 'j':
            selected_row_++;
            return true;
        case KEY_PPAGE:
            selected_row_ -= 10;
            if (selected_row_ < 0) selected_row_ = 0;
            return true;
        case KEY_NPAGE:
            selected_row_ += 10;
            return true;
        case KEY_LEFT:
        case '<':
        case ',':
            sort_column_ = (sort_column_ - 1 + 6) % 6;
            return true;
        case KEY_RIGHT:
        case '>':
        case '.':
            sort_column_ = (sort_column_ + 1) % 6;
            return true;
        case 'i':
        case 'I':
            sort_desc_ = !sort_desc_;
            return true;
        case '/':
            filter_mode_ = true;
            return true;
    }
    return false;
}
