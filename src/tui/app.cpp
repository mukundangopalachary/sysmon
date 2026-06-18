#include "app.h"

int Application::run(int argc, char** argv) {
    (void)argc;
    (void)argv;
    return 0;
}

void Application::register_screen(const std::string&, std::unique_ptr<Screen>) {}

void Application::switch_screen(const std::string&) {}
