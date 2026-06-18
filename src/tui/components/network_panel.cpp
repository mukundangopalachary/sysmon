#include "components/network_panel.h"

NetworkPanel::NetworkPanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

NetworkPanel::~NetworkPanel() {}

void NetworkPanel::render(const SystemSnapshot* /*snapshot*/) {}

bool NetworkPanel::handle_input(int /*key*/) { return false; }
