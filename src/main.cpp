#include "app.h"
#include "sysmon_bridge.h"
#include "collection_engine.h"
#include "snapshot_manager.h"
#include <iostream>
#include <string>
#include <string.h>
#include "cli_main.h"

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
    bool is_cli = false;
    
    // Check if binary is invoked as sysmon-cli
    if (strstr(argv[0], "sysmon-cli") != NULL) {
        is_cli = true;
    }
    
    // Or if first argument is a known CLI command
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "plugin" || arg == "config" || arg == "theme" || arg == "registry") {
            is_cli = true;
        }
    }

    if (is_cli) {
        return cli_main(argc, argv);
    }

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
