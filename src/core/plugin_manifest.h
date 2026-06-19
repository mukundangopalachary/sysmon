#ifndef SYSMON_PLUGIN_MANIFEST_H
#define SYSMON_PLUGIN_MANIFEST_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PANE_NAME 32
#define MAX_REQUIREMENTS 8

typedef struct {
    char name[MAX_PANE_NAME];
    char type[16];      // GAUGE, TABLE, TEXT
    int refresh_rate;
} PluginPane;

typedef struct {
    char name[64];
    char version[16];
    char description[256];
    char author[64];
    char license[32];
    
    char requirements[MAX_REQUIREMENTS][32];
    int num_requirements;
    
    PluginPane panes[8];
    int num_panes;
} PluginManifest;

// Parse manifest.toml from the given plugin directory path
bool plugin_manifest_parse(const char* plugin_dir, PluginManifest* manifest);

#ifdef __cplusplus
}
#endif

#endif // SYSMON_PLUGIN_MANIFEST_H
