#include "plugin_manager.h"
#include <string.h>

int plugin_manager_init(PluginManager* mgr, const char* plugin_dir) {
    (void)plugin_dir;
    memset(mgr, 0, sizeof(*mgr));
    return 0;
}

int plugin_manager_load_plugins(PluginManager* mgr) {
    (void)mgr;
    return 0;
}

int plugin_manager_start_all(PluginManager* mgr) {
    (void)mgr;
    return 0;
}

int plugin_manager_stop_all(PluginManager* mgr) {
    (void)mgr;
    return 0;
}

int plugin_manager_collect(PluginManager* mgr) {
    (void)mgr;
    return 0;
}
