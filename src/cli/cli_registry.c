#include <stdio.h>
#include <string.h>
#include "plugin_registry.h"

int cli_registry_main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: sysmon registry <command>\n");
        printf("Commands:\n");
        printf("  update     Fetch latest plugin index from remote\n");
        return 1;
    }

    const char* subcmd = argv[2];

    if (strcmp(subcmd, "update") == 0) {
        if (registry_update()) {
            printf("Registry successfully updated.\n");
        } else {
            printf("Failed to update registry.\n");
            return 1;
        }
    } else {
        printf("Unknown registry command: %s\n", subcmd);
        return 1;
    }

    return 0;
}
