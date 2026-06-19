#include "app.h"
#include "sysmon_bridge.h"
#include "collection_engine.h"
#include "snapshot_manager.h"
#include <iostream>
#include <string>
#include <string.h>

void print_help() {
    std::cout << "sysmon v1.0.0 - Interactive System Monitor\n\n"
              << "Usage: sysmon [OPTIONS]\n\n"
              << "Options:\n"
              << "  -h, --help     Show this help message and exit\n"
              << "  -v, --version  Show version information and exit\n"
              << "  -d, --debug    Enable debug logging (not yet implemented)\n\n"
              << "Interactive Keybindings:\n"
              << "  [d] Dashboard view\n"
              << "  [p] Process list view\n"
              << "  [c] Connections view\n"
              << "  [q] Quit application\n"
              << "  [Up/Down/j/k] Navigate tables\n"
              << "  [ENTER] View process details\n"
              << "  [ESC] Return from details\n";
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_help();
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            std::cout << "sysmon version 1.0.0" << std::endl;
            return 0;
        } else if (arg == "-d" || arg == "--debug") {
            // Future debug toggle
        } else {
            std::cerr << "Unknown argument: " << arg << "\n"
                      << "Try 'sysmon --help' for more information.\n";
            return 1;
        }
    }

    SnapshotManager snap_mgr;
    snapshot_manager_init(&snap_mgr);

    CollectionEngine engine;
    collection_engine_init(&engine, &snap_mgr);
    collection_engine_start(&engine);

    Application app(&snap_mgr);
    int result = app.run(argc, argv);

    collection_engine_stop(&engine);
    collection_engine_destroy(&engine);
    snapshot_manager_destroy(&snap_mgr);

    return result;
}
