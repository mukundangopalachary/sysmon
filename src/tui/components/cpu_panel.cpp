#include "components/cpu_panel.h"

CpuPanel::CpuPanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

CpuPanel::~CpuPanel() {}

void CpuPanel::render(const SystemSnapshot* snapshot) {
    if (!snapshot) return;
    draw_border();
    draw_title("CPU");

    int h, w;
    getmaxyx(window, h, w);

    for (int i = 0; i < snapshot->cpu.num_cores && i < h - 2; ++i) {
        int usage = (int)snapshot->cpu.cores[i].usage_percent;
        int bar_width = 10;
        int filled = (usage * bar_width) / 100;

        mvwprintw(window, i + 1, 1, "CPU%d [", i);
        for (int j = 0; j < bar_width; ++j) {
            if (j < filled) {
                waddch(window, '#');
            } else {
                waddch(window, ' ');
            }
        }
        wprintw(window, "] %3d%%", usage);
    }
    wnoutrefresh(window);
}

bool CpuPanel::handle_input(int /*key*/) { return false; }
