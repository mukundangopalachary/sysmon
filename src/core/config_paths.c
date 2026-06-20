#include "config_paths.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

static void get_xdg_path(const char* env_var, const char* default_suffix, char* path, int max_len) {
    const char* env_val = getenv(env_var);
    if (env_val && env_val[0] != '\0') {
        snprintf(path, max_len, "%s/sysmon", env_val);
    } else {
        const char* home = getenv("HOME");
        if (home) {
            snprintf(path, max_len, "%s/%s/sysmon", home, default_suffix);
        } else {
            snprintf(path, max_len, "/tmp/sysmon");
        }
    }
}

void config_get_user_config_path(char* path, int max_len) {
    get_xdg_path("XDG_CONFIG_HOME", ".config", path, max_len);
    strncat(path, "/sysmon.toml", max_len - strlen(path) - 1);
}

void config_get_user_data_path(char* path, int max_len) {
    get_xdg_path("XDG_DATA_HOME", ".local/share", path, max_len);
    // Keep as directory
}

void config_get_user_cache_path(char* path, int max_len) {
    get_xdg_path("XDG_CACHE_HOME", ".cache", path, max_len);
    // Keep as directory
}

static bool make_dir_recursive(const char* path) {
    char tmp[512];
    snprintf(tmp, sizeof(tmp), "%s", path);
    size_t len = strlen(tmp);
    if(tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for(char* p = tmp + 1; *p; p++)
        if(*p == '/') {
            *p = 0;
            mkdir(tmp, 0755);
            *p = '/';
        }
    return (mkdir(tmp, 0755) == 0 || errno == EEXIST);
}

bool config_ensure_directories(void) {
    char path[512];
    
    get_xdg_path("XDG_CONFIG_HOME", ".config", path, sizeof(path));
    if (!make_dir_recursive(path)) return false;
    
    config_get_user_data_path(path, sizeof(path));
    if (!make_dir_recursive(path)) return false;
    
    char plugins_path[1024];
    snprintf(plugins_path, sizeof(plugins_path), "%s/plugins", path);
    if (!make_dir_recursive(plugins_path)) return false;

    char registry_path[1024];
    snprintf(registry_path, sizeof(registry_path), "%s/registry", path);
    if (!make_dir_recursive(registry_path)) return false;
    
    config_get_user_cache_path(path, sizeof(path));
    if (!make_dir_recursive(path)) return false;
    
    return true;
}
