#include "components/memory_panel.h"

MemoryPanel::MemoryPanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

MemoryPanel::~MemoryPanel() {}

void MemoryPanel::render(const SystemSnapshot* /*snapshot*/) {}

bool MemoryPanel::handle_input(int /*key*/) { return false; }
