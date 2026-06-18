#include "components/process_detail.h"

ProcessDetailPanel::ProcessDetailPanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

ProcessDetailPanel::~ProcessDetailPanel() {}

void ProcessDetailPanel::render(const SystemSnapshot* /*snapshot*/) {}

bool ProcessDetailPanel::handle_input(int /*key*/) { return false; }
