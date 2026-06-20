#include "cli_main.h"
#include <stdio.h>
#include <string.h>
#include "config_paths.h"

static void print_usage() {
    printf("sysmon-cli - Command Line Management for sysmon\n\n");
    printf("Usage: sysmon <command> [options]\n\n");
    printf("Commands:\n");
    printf("  plugin   Manage plugins (install, list, update)\n");
    printf("  config   Manage configuration\n");
    printf("  theme    Manage UI themes\n");
    printf("  registry Sync plugin registry\n");
}

int cli_main(int argc, char** argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    config_ensure_directories();

    const char* command = argv[1];

    if (strcmp(command, "plugin") == 0) {
        extern int cli_plugin_main(int argc, char** argv);
        return cli_plugin_main(argc, argv);
    } else if (strcmp(command, "config") == 0) {
        extern int cli_config_main(int argc, char** argv);
        return cli_config_main(argc, argv);
    } else if (strcmp(command, "theme") == 0) {
        printf("Theme command not yet implemented.\n");
    } else if (strcmp(command, "registry") == 0) {
        extern int cli_registry_main(int argc, char** argv);
        return cli_registry_main(argc, argv);
    } else if (strcmp(command, "--help") == 0 || strcmp(command, "-h") == 0) {
        print_usage();
        return 0;
    } else {
        printf("Unknown command: %s\n", command);
        print_usage();
        return 1;
    }

    return 0;
}
