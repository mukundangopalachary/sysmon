#include "plugin_state.h"
#include "config_paths.h"
#include "toml.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PLUGINS 64

typedef struct {
    char name[64];
    bool enabled;
} PluginEntry;

static void get_plugins_toml_path(char* path, int max_len) {
    config_get_user_config_path(path, max_len);
    // Replace /sysmon.toml with /plugins.toml
    char* last_slash = strrchr(path, '/');
    if (last_slash) {
        strcpy(last_slash + 1, "plugins.toml");
    }
}

static int read_all_states(PluginEntry entries[MAX_PLUGINS]) {
    char path[512];
    get_plugins_toml_path(path, sizeof(path));

    FILE* fp = fopen(path, "r");
    if (!fp) return 0;

    char errbuf[200];
    toml_table_t* conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!conf) return 0;

    int count = 0;
    toml_table_t* plugins = toml_table_in(conf, "plugins");
    if (plugins) {
        for (int i = 0; ; i++) {
            const char* key = toml_key_in(plugins, i);
            if (!key) break;
            
            toml_datum_t res = toml_bool_in(plugins, key);
            if (res.ok && count < MAX_PLUGINS) {
                strncpy(entries[count].name, key, sizeof(entries[count].name) - 1);
                entries[count].enabled = (bool)res.u.b;
                count++;
            }
        }
    }

    toml_free(conf);
    return count;
}

bool plugin_state_is_enabled(const char* plugin_name) {
    PluginEntry entries[MAX_PLUGINS];
    int count = read_all_states(entries);
    
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].name, plugin_name) == 0) {
            return entries[i].enabled;
        }
    }
    return false;
}

bool plugin_state_set_enabled(const char* plugin_name, bool enabled) {
    PluginEntry entries[MAX_PLUGINS];
    int count = read_all_states(entries);
    
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].name, plugin_name) == 0) {
            entries[i].enabled = enabled;
            found = true;
            break;
        }
    }
    
    if (!found && count < MAX_PLUGINS) {
        strncpy(entries[count].name, plugin_name, sizeof(entries[count].name) - 1);
        entries[count].enabled = enabled;
        count++;
    }

    char path[512];
    get_plugins_toml_path(path, sizeof(path));
    
    config_ensure_directories();

    FILE* fp = fopen(path, "w");
    if (!fp) return false;

    fprintf(fp, "[plugins]\n");
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s = %s\n", entries[i].name, entries[i].enabled ? "true" : "false");
    }

    fclose(fp);
    return true;
}
