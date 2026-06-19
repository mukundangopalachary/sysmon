#ifndef SYSMON_PLUGIN_MANAGER_H
#define SYSMON_PLUGIN_MANAGER_H

#include "types.h"
#include <sys/types.h>

typedef struct {
    char metrics[16][128];
    int num_metrics;
} PluginData;

typedef struct PluginManager PluginManager;

typedef struct {
    char name[64];
    char path[512];
    pid_t pid;
    int stdout_fd;
    int interval_ms;
    bool running;
    bool enabled;
} PluginInstance;

struct PluginManager {
    int num_plugins;
    PluginInstance plugins[32];
    void* plugin_metrics;
};

int plugin_manager_init(PluginManager* mgr, const char* plugin_dir);
int plugin_manager_load_plugins(PluginManager* mgr);
int plugin_manager_start_all(PluginManager* mgr);
int plugin_manager_stop_all(PluginManager* mgr);
int plugin_manager_collect(PluginManager* mgr, PluginData* out_data);

#endif /* SYSMON_PLUGIN_MANAGER_H */
