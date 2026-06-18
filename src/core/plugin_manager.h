#ifndef SYSMON_PLUGIN_MANAGER_H
#define SYSMON_PLUGIN_MANAGER_H

#include "types.h"

/*
 * PLUGIN MANAGER
 * 
 * Manages lifecycle of Bash plugins.
 * Each plugin runs as a child process.
 * Manager reads plugin stdout, parses protocol, stores in snapshot.
 */
typedef struct PluginManager PluginManager;

typedef struct {
    char name[64];
    char path[512];                 /* Path to script */
    pid_t pid;
    int stdout_fd;                  /* Pipe for reading plugin output */
    int interval_ms;
    bool running;
    bool enabled;
} PluginInstance;

struct PluginManager {
    int num_plugins;
    PluginInstance plugins[32];     /* Max 32 plugins */
    
    /* Plugin data for current snapshot */
    void* plugin_metrics;           /* Parsed metrics, opaque to core */
};

/* Public API */
int plugin_manager_init(PluginManager* mgr, const char* plugin_dir);
int plugin_manager_load_plugins(PluginManager* mgr);
int plugin_manager_start_all(PluginManager* mgr);
int plugin_manager_stop_all(PluginManager* mgr);
int plugin_manager_collect(PluginManager* mgr);  /* Non-blocking read of all plugin pipes */

#endif /* SYSMON_PLUGIN_MANAGER_H */
