#include "components/header.h"

HeaderPanel::HeaderPanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

HeaderPanel::~HeaderPanel() {}

void HeaderPanel::render(const SystemSnapshot* /*snapshot*/) {}

bool HeaderPanel::handle_input(int /*key*/) { return false; }
