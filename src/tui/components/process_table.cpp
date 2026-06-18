#include "components/process_table.h"

ProcessTablePanel::ProcessTablePanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

ProcessTablePanel::~ProcessTablePanel() {}

void ProcessTablePanel::render(const SystemSnapshot* /*snapshot*/) {}

bool ProcessTablePanel::handle_input(int /*key*/) { return false; }
