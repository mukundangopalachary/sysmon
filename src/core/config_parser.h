#ifndef SYSMON_CONFIG_PARSER_H
#define SYSMON_CONFIG_PARSER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Full config structure matches TOML schema
typedef struct {
    struct {
        int interval_ms;
        int max_processes;
    } collection;

    struct {
        char theme[32];
        int refresh_rate_hz;
        bool compact_mode;
    } display;

    struct {
        char process_default_column[16];
        char process_default_order[16];
    } sorting;

    struct {
        char quit[8];
        char dashboard[8];
        char process_list[8];
        char connections[8];
        char plugins[8];
        char help[8];
    } keybindings;

    struct {
        bool enabled;
        int cpu_threshold;
        int memory_threshold;
    } alerts;
} SysmonConfig;

// Initialize config with defaults
void config_init_defaults(SysmonConfig* cfg);

// Load config from TOML file
bool config_load(SysmonConfig* cfg, const char* path);

// Save config to TOML file
bool config_save(const SysmonConfig* cfg, const char* path);

// Print config (for CLI show)
void config_print(const SysmonConfig* cfg);

#ifdef __cplusplus
}
#endif

#endif // SYSMON_CONFIG_PARSER_H
