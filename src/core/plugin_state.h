#ifndef SYSMON_PLUGIN_STATE_H
#define SYSMON_PLUGIN_STATE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Check if a plugin is enabled in ~/.config/sysmon/plugins.toml
bool plugin_state_is_enabled(const char* plugin_name);

// Set a plugin's enabled state
bool plugin_state_set_enabled(const char* plugin_name, bool enabled);

#ifdef __cplusplus
}
#endif

#endif // SYSMON_PLUGIN_STATE_H
