#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config_parser.h"
#include "config_paths.h"

int cli_config_main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: sysmon config <command> [args]\n");
        printf("Commands:\n");
        printf("  show       Print current effective config\n");
        printf("  get <key>  Get specific value\n");
        printf("  set <key> <value> Set value and save\n");
        printf("  reset      Restore to defaults\n");
        return 1;
    }

    const char* subcmd = argv[2];
    
    char config_path[512];
    config_get_user_config_path(config_path, sizeof(config_path));
    
    SysmonConfig cfg;
    config_init_defaults(&cfg);
    config_load(&cfg, config_path);

    if (strcmp(subcmd, "show") == 0) {
        config_print(&cfg);
        return 0;
    } else if (strcmp(subcmd, "get") == 0) {
        if (argc < 4) {
            printf("Usage: sysmon config get <key>\n");
            return 1;
        }
        const char* key = argv[3];
        if (strcmp(key, "display.theme") == 0) {
            printf("%s\n", cfg.display.theme);
        } else if (strcmp(key, "collection.interval_ms") == 0) {
            printf("%d\n", cfg.collection.interval_ms);
        } else {
            printf("Unknown or unsupported key for get: %s\n", key);
            return 1;
        }
    } else if (strcmp(subcmd, "set") == 0) {
        if (argc < 5) {
            printf("Usage: sysmon config set <key> <value>\n");
            return 1;
        }
        const char* key = argv[3];
        const char* val = argv[4];
        
        if (strcmp(key, "display.theme") == 0) {
            strncpy(cfg.display.theme, val, sizeof(cfg.display.theme)-1);
        } else if (strcmp(key, "collection.interval_ms") == 0) {
            cfg.collection.interval_ms = atoi(val);
        } else {
            printf("Unknown or unsupported key for set: %s\n", key);
            return 1;
        }
        
        config_ensure_directories();
        if (config_save(&cfg, config_path)) {
            printf("Config saved to %s\n", config_path);
        } else {
            printf("Failed to save config to %s\n", config_path);
            return 1;
        }
    } else if (strcmp(subcmd, "reset") == 0) {
        remove(config_path);
        printf("Config reset to defaults (removed %s)\n", config_path);
    } else if (strcmp(subcmd, "migrate") == 0) {
        config_ensure_directories();
        if (config_save(&cfg, config_path)) {
            printf("Migrated V1 config to TOML format at %s\n", config_path);
        } else {
            printf("Failed to migrate config\n");
            return 1;
        }
    } else {
        printf("Unknown config command: %s\n", subcmd);
        return 1;
    }

    return 0;
}
