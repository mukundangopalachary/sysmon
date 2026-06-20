#include "plugin_manager.h"
#include "plugin_state.h"
#include "plugin_protocol.h"
#include "sysmon_bridge.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

#include "plugin_state.h"

int plugin_manager_init(PluginManager* mgr, const char* plugin_dir) {
    memset(mgr, 0, sizeof(*mgr));
    DIR* dir = opendir(plugin_dir);
    if (!dir) return -1;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", plugin_dir, entry->d_name);
        
        // Ensure it's a directory (or try parsing manifest anyway)
        PluginManifest manifest;
        if (plugin_manifest_parse(path, &manifest) && mgr->num_plugins < 32) {
            PluginInstance* p = &mgr->plugins[mgr->num_plugins];
            snprintf(p->name, sizeof(p->name), "%.63s", entry->d_name);
            snprintf(p->path, sizeof(p->path), "%.500s/plugin.sh", path); // Read from manifest or assume plugin.sh
            p->manifest = manifest;
            p->enabled = plugin_state_is_enabled(entry->d_name);
            p->interval_ms = 1000; // Default or from manifest
            mgr->num_plugins++;
        }
    }
    closedir(dir);
    return 0;
}

int plugin_manager_load_plugins(PluginManager* mgr) {
    (void)mgr;
    return 0;
}

int plugin_manager_start_all(PluginManager* mgr) { (void)mgr; return 0; }
int plugin_manager_stop_all(PluginManager* mgr) { (void)mgr; return 0; }

int plugin_manager_collect(PluginManager* mgr, PluginData* out_data) {
    if (out_data) out_data->num_metrics = 0;
    uint64_t now = get_time_us();

    for (int i = 0; i < mgr->num_plugins; i++) {
        PluginInstance* p = &mgr->plugins[i];
        if (!p->enabled) continue;

        if (now - p->last_run_us >= p->interval_ms * 1000ULL || p->last_run_us == 0) {
            char* out = NULL;
            if (plugin_protocol_execute(p->path, "collect", &out) && out) {
                snprintf(p->cached_output, sizeof(p->cached_output), "%.4095s", out);
            }
            if (out) free(out);
            p->last_run_us = now;
        }
    }
    return 0;
}
