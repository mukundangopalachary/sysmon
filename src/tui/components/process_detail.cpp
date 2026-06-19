#include "components/process_detail.h"
#include "sysmon_bridge.h"
#include <fstream>
#include <sstream>
#include <string>

ProcessDetailPanel::ProcessDetailPanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

ProcessDetailPanel::~ProcessDetailPanel() {}

void ProcessDetailPanel::open(int pid) {
    pid_ = pid;
}

void ProcessDetailPanel::render(const SystemSnapshot* snapshot) {
    draw_border();
    draw_title("Process Detail");

    if (pid_ == -1 || !snapshot) {
        mvwprintw(window, 2, 2, "No process selected.");
        wnoutrefresh(window);
        return;
    }

    const ProcessSnapshot* p = process_find_by_pid(snapshot, pid_);
    if (!p) {
        mvwprintw(window, 2, 2, "Process %d not found (may have exited).", pid_);
    } else {
        std::string vsz = format_bytes(p->vsize_bytes);
        std::string rss = format_bytes(p->rss_bytes);

        mvwprintw(window, 2, 2, "Name:     %s", p->comm);
        mvwprintw(window, 3, 2, "PID:      %d", p->pid);
        mvwprintw(window, 4, 2, "User:     %s", p->username[0] ? p->username : "root");
        mvwprintw(window, 5, 2, "State:    %c", p->state);
        mvwprintw(window, 6, 2, "CPU:      %.1f%%", p->cpu_percent);
        mvwprintw(window, 7, 2, "Memory:   %.1f%%", p->mem_percent);
        mvwprintw(window, 8, 2, "Virtual:  %s", vsz.c_str());
        mvwprintw(window, 9, 2, "Resident: %s", rss.c_str());

        // Parse cmdline
        std::string cmdline_path = "/proc/" + std::to_string(p->pid) + "/cmdline";
        std::ifstream ifs(cmdline_path);
        if (ifs) {
            std::stringstream buffer;
            buffer << ifs.rdbuf();
            std::string cmdline = buffer.str();
            for (char& c : cmdline) {
                if (c == '\0') c = ' ';
            }
            if (!cmdline.empty()) {
                mvwprintw(window, 11, 2, "Cmdline: %.80s", cmdline.c_str());
            }
        }
    }

    mvwprintw(window, getmaxy(window) - 2, 2, "Press ESC/q/d to return");
    wnoutrefresh(window);
}

bool ProcessDetailPanel::handle_input(int key) {
    if (key == 27 || key == 'q' || key == 'd') {
        if (on_close) on_close();
        return true;
    }
    return false;
}
