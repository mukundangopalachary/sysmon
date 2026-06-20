#include "app.h"
#include "sysmon_bridge.h"
#include "collection_engine.h"
#include "snapshot_manager.h"
#include <iostream>
#include <string>
#include <string.h>
#include "cli_main.h"
#include <unistd.h>
#include <fcntl.h>

void print_help() {
    std::cout << "sysmon v1.0.0 - Interactive System Monitor\n\n"
              << "Usage:\n"
              << "  sysmon [OPTIONS]            (Starts the TUI)\n"
              << "  sysmon <command> [args]     (Runs CLI commands)\n\n"
              << "TUI Options:\n"
              << "  -h, --help     Show this help message and exit\n"
              << "  -v, --version  Show version information and exit\n\n"
              << "CLI Commands:\n"
              << "  plugin         Manage plugins (install, list, update)\n"
              << "  config         Manage configuration\n"
              << "  theme          Manage UI themes\n"
              << "  registry       Sync plugin registry\n\n"
              << "TUI Interactive Keybindings:\n"
              << "  [F1] Dashboard view\n"
              << "  [F2] Process list view\n"
              << "  [F3] Connections view\n"
              << "  [F4] Plugins view\n"
              << "  [p]  Pause/Resume data collection\n"
              << "  [/]  Search/Filter mode in tables\n"
              << "  [q]  Quit application\n"
              << "  [Up/Down/j/k] Navigate tables\n"
              << "  [ENTER] View process details\n"
              << "  [ESC] Return from details/cancel search\n";
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

    // Check if /proc is mounted
    if (access("/proc/stat", R_OK) != 0) {
        std::cerr << "Error: /proc is not accessible. Sysmon requires a mounted procfs.\n";
        return 1;
    }

    // Single-instance lock file
    char lock_file[256];
    snprintf(lock_file, sizeof(lock_file), "/tmp/sysmon_%d.lock", getuid());
    int lock_fd = open(lock_file, O_RDWR | O_CREAT, 0600);
    if (lock_fd < 0) {
        std::cerr << "Error: Could not create lock file " << lock_file << "\n";
        return 1;
    }
    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if (fcntl(lock_fd, F_SETLK, &fl) == -1) {
        std::cerr << "Error: Another instance of sysmon is already running for this user.\n";
        close(lock_fd);
        return 1;
    }

    SnapshotManager snap_mgr;
    snapshot_manager_init(&snap_mgr);

    CollectionEngine engine;
    collection_engine_init(&engine, &snap_mgr);
    collection_engine_start(&engine);

    Application app(&snap_mgr, &engine);
    int result = app.run(argc, argv);

    collection_engine_stop(&engine);
    collection_engine_destroy(&engine);
    snapshot_manager_destroy(&snap_mgr);

    return result;
}
