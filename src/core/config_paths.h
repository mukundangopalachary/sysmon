#ifndef SYSMON_CONFIG_PATHS_H
#define SYSMON_CONFIG_PATHS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Resolves path to ~/.config/sysmon/config.toml (or XDG_CONFIG_HOME)
void config_get_user_config_path(char* path, int max_len);

// Resolves path to ~/.local/share/sysmon/ (or XDG_DATA_HOME)
void config_get_user_data_path(char* path, int max_len);

// Resolves path to ~/.cache/sysmon/ (or XDG_CACHE_HOME)
void config_get_user_cache_path(char* path, int max_len);

// Ensure directories exist
bool config_ensure_directories(void);

#ifdef __cplusplus
}
#endif

#endif // SYSMON_CONFIG_PATHS_H
