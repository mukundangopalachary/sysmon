#include "components/cpu_panel.h"

CpuPanel::CpuPanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

CpuPanel::~CpuPanel() {}

void CpuPanel::render(const SystemSnapshot* /*snapshot*/) {}

bool CpuPanel::handle_input(int /*key*/) { return false; }
