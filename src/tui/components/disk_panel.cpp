#include "components/disk_panel.h"

DiskPanel::DiskPanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

DiskPanel::~DiskPanel() {}

void DiskPanel::render(const SystemSnapshot* snapshot) {
    if (!snapshot) return;
    draw_border();
    draw_title("Disk I/O");

    if (snapshot->disk == nullptr) {
        mvwprintw(window, 1, 1, "Disk stats disabled");
        wnoutrefresh(window);
        return;
    }

    int h, w;
    getmaxyx(window, h, w);

    for (int i = 0; i < snapshot->disk->num_devices && i < h - 2; ++i) {
        const auto& dev = snapshot->disk->devices[i];
        mvwprintw(window, i + 1, 1, "%-8s R: %10s/s W: %10s/s IOPS: %.1f",
                  dev.device_name,
                  format_bytes((uint64_t)dev.read_bytes_per_sec),
                  format_bytes((uint64_t)dev.write_bytes_per_sec),
                  dev.iops);
    }
    wnoutrefresh(window);
}

bool DiskPanel::handle_input(int /*key*/) { return false; }
