#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "plugin_manifest.h"
#include "plugin_state.h"
#include "plugin_protocol.h"
#include "config_paths.h"

int cli_plugin_main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: sysmon plugin <command> [args]\n");
        printf("Commands:\n");
        printf("  list            List all installed plugins\n");
        printf("  info <name>     Show manifest details\n");
        printf("  enable <name>   Enable a plugin\n");
        printf("  disable <name>  Disable a plugin\n");
        printf("  check <name>    Run plugin's check command\n");
        return 1;
    }

    const char* subcmd = argv[2];
    
    // For builtin plugins path
    char plugins_dir[512];
    // Normally this searches multiple dirs, for simplicity we look in scripts/plugins/builtin/
    // if running from source, or ~/.local/share/sysmon/plugins/
    config_get_user_data_path(plugins_dir, sizeof(plugins_dir));
    strncat(plugins_dir, "/plugins", sizeof(plugins_dir) - strlen(plugins_dir) - 1);

    if (strcmp(subcmd, "list") == 0) {
        printf("Installed Plugins:\n");
        DIR* d = opendir(plugins_dir);
        if (d) {
            struct dirent* dir;
            while ((dir = readdir(d)) != NULL) {
                if (dir->d_type == DT_DIR && dir->d_name[0] != '.') {
                    char full_dir[512];
                    snprintf(full_dir, sizeof(full_dir), "%s/%s", plugins_dir, dir->d_name);
                    PluginManifest manifest;
                    if (plugin_manifest_parse(full_dir, &manifest)) {
                        bool enabled = plugin_state_is_enabled(dir->d_name);
                        printf("  [%c] %-15s v%s - %s\n", enabled ? 'x' : ' ', manifest.name, manifest.version, manifest.description);
                    }
                }
            }
            closedir(d);
        } else {
            printf("  (No plugins directory found at %s)\n", plugins_dir);
        }
    } else if (strcmp(subcmd, "enable") == 0 || strcmp(subcmd, "disable") == 0) {
        if (argc < 4) {
            printf("Usage: sysmon plugin %s <name>\n", subcmd);
            return 1;
        }
        bool enable = (strcmp(subcmd, "enable") == 0);
        if (plugin_state_set_enabled(argv[3], enable)) {
            printf("Plugin '%s' has been %s.\n", argv[3], enable ? "enabled" : "disabled");
        } else {
            printf("Failed to update plugin state.\n");
            return 1;
        }
    } else if (strcmp(subcmd, "info") == 0) {
        if (argc < 4) {
            printf("Usage: sysmon plugin info <name>\n");
            return 1;
        }
        char full_dir[512];
        snprintf(full_dir, sizeof(full_dir), "%s/%s", plugins_dir, argv[3]);
        PluginManifest manifest;
        if (plugin_manifest_parse(full_dir, &manifest)) {
            printf("Name:        %s\n", manifest.name);
            printf("Version:     %s\n", manifest.version);
            printf("Description: %s\n", manifest.description);
            printf("Author:      %s\n", manifest.author);
            printf("License:     %s\n", manifest.license);
            printf("Status:      %s\n", plugin_state_is_enabled(argv[3]) ? "Enabled" : "Disabled");
        } else {
            printf("Manifest not found for plugin '%s'\n", argv[3]);
            return 1;
        }
    } else if (strcmp(subcmd, "check") == 0) {
        if (argc < 4) {
            printf("Usage: sysmon plugin check <name>\n");
            return 1;
        }
        char executable[512];
        snprintf(executable, sizeof(executable), "%s/%s/plugin.sh", plugins_dir, argv[3]);
        char* output = NULL;
        printf("Running check for plugin '%s'...\n", argv[3]);
        bool success = plugin_protocol_execute(executable, "check", &output);
        if (output) {
            printf("%s\n", output);
            free(output);
        }
        if (success) {
            printf("Check PASSED.\n");
        } else {
            printf("Check FAILED.\n");
            return 1;
        }
    } else {
        printf("Unknown plugin command: %s\n", subcmd);
        return 1;
    }

    return 0;
}
