#include "components/disk_panel.h"

DiskPanel::DiskPanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

DiskPanel::~DiskPanel() {}

void DiskPanel::render(const SystemSnapshot* /*snapshot*/) {}

bool DiskPanel::handle_input(int /*key*/) { return false; }
