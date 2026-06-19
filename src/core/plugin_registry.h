#ifndef SYSMON_PLUGIN_REGISTRY_H
#define SYSMON_PLUGIN_REGISTRY_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_REGISTRY_PLUGINS 128

typedef struct {
    char name[64];
    char version[16];
    char description[256];
    char url[256];
    char checksum[128];
} RegistryPlugin;

typedef struct {
    RegistryPlugin plugins[MAX_REGISTRY_PLUGINS];
    int count;
} PluginRegistry;

// Load the local cached registry index
bool registry_load_local(PluginRegistry* registry);

// Sync remote registry to local cache
bool registry_update(void);

// Search local registry for a plugin by name or description
// Results populated in 'results'. Returns number of matches.
int registry_search(const PluginRegistry* registry, const char* query, RegistryPlugin* results, int max_results);

// Find an exact match by name
bool registry_find_exact(const PluginRegistry* registry, const char* name, RegistryPlugin* result);

// Download and install a plugin
bool registry_install_plugin(const RegistryPlugin* plugin);

// Uninstall a plugin
bool registry_uninstall_plugin(const char* name);

#ifdef __cplusplus
}
#endif

#endif // SYSMON_PLUGIN_REGISTRY_H
