#include "components/connection_table.h"

ConnectionTablePanel::ConnectionTablePanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

ConnectionTablePanel::~ConnectionTablePanel() {}

void ConnectionTablePanel::render(const SystemSnapshot* /*snapshot*/) {}

bool ConnectionTablePanel::handle_input(int /*key*/) { return false; }
