#include "components/connection_table.h"
#include <ncurses.h>
#include <string.h>

ConnectionTablePanel::ConnectionTablePanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

ConnectionTablePanel::~ConnectionTablePanel() {}

void ConnectionTablePanel::render(const SystemSnapshot* snapshot) {
    if (!snapshot || !snapshot->connections || snapshot->connections->num_connections == 0) {
        mvwprintw(window, 1, 1, "No data");
        wrefresh(window);
        return;
    }

    // Draw headers
    wattron(window, A_BOLD);
    mvwprintw(window, 1, 1, "%-8s %-22s %-22s %-12s %s", "PROTO", "LOCAL ADDRESS", "REMOTE ADDRESS", "STATE", "INODE");
    wattroff(window, A_BOLD);

    int num_connections = snapshot->connections->num_connections;
    int max_rows = height_ - 3; // accounting for borders and header

    // Bounds checking for selected_row_
    if (selected_row_ < 0) selected_row_ = 0;
    if (selected_row_ >= num_connections) selected_row_ = num_connections - 1;

    // Adjust scroll_offset_ to keep selected_row_ visible
    if (selected_row_ < scroll_offset_) {
        scroll_offset_ = selected_row_;
    } else if (selected_row_ >= scroll_offset_ + max_rows) {
        scroll_offset_ = selected_row_ - max_rows + 1;
    }

    for (int i = 0; i < max_rows; ++i) {
        int idx = scroll_offset_ + i;
        if (idx >= num_connections) {
            // clear remaining rows
            wmove(window, i + 2, 1);
            wclrtoeol(window);
            continue;
        }

        const ConnectionSnapshot& conn = snapshot->connections->connections[idx];

        if (idx == selected_row_) {
            wattron(window, A_REVERSE);
        }

        const char* proto = "UNKNOWN";
        switch (conn.type) {
            case CONN_TCP: proto = "TCP"; break;
            case CONN_TCP6: proto = "TCP6"; break;
            case CONN_UDP: proto = "UDP"; break;
            case CONN_UDP6: proto = "UDP6"; break;
            case CONN_UNIX: proto = "UNIX"; break;
        }

        const char* state = "";
        switch (conn.state) {
            case CONN_ESTABLISHED: state = "ESTABLISHED"; break;
            case CONN_LISTEN: state = "LISTEN"; break;
            case CONN_TIME_WAIT: state = "TIME_WAIT"; break;
            case CONN_CLOSE_WAIT: state = "CLOSE_WAIT"; break;
            case CONN_SYN_SENT: state = "SYN_SENT"; break;
            case CONN_OTHER: state = "OTHER"; break;
        }

        mvwprintw(window, i + 2, 1, "%-8s %-22s %-22s %-12s %llu", 
            proto, conn.local_addr, conn.remote_addr, state, (unsigned long long)conn.inode);

        if (idx == selected_row_) {
            wattroff(window, A_REVERSE);
        }
    }

    box(window, 0, 0); // Redraw box in case it was overwritten
    wrefresh(window);
}

bool ConnectionTablePanel::handle_input(int key) {
    if (key == KEY_UP || key == 'k') {
        if (selected_row_ > 0) {
            selected_row_--;
            return true;
        }
    } else if (key == KEY_DOWN || key == 'j') {
        selected_row_++; // Bound check is handled in render
        return true;
    }
    return false;
}
