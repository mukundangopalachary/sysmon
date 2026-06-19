#include "components/network_panel.h"

NetworkPanel::NetworkPanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

NetworkPanel::~NetworkPanel() {}

void NetworkPanel::render(const SystemSnapshot* snapshot) {
    if (!snapshot) return;
    draw_border();
    draw_title("Network");

    int h, w;
    getmaxyx(window, h, w);

    for (int i = 0; i < snapshot->network.num_interfaces && i < h - 2; ++i) {
        const auto& iface = snapshot->network.interfaces[i];
        mvwprintw(window, i + 1, 1, "%-6s Rx: %10s/s Tx: %10s/s",
                  iface.name,
                  format_bytes((uint64_t)iface.rx_bytes_per_sec),
                  format_bytes((uint64_t)iface.tx_bytes_per_sec));
    }
    wnoutrefresh(window);
}

bool NetworkPanel::handle_input(int /*key*/) { return false; }
