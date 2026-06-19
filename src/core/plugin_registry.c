#include "plugin_registry.h"
#include "config_paths.h"
#include "toml.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>

#define REGISTRY_URL "https://raw.githubusercontent.com/mukundangopalachary/sysmon/master/registry/index.toml"

static void get_registry_dir(char* path, int max_len) {
    config_get_user_data_path(path, max_len);
    strncat(path, "/registry", max_len - strlen(path) - 1);
}

static void get_registry_index_path(char* path, int max_len) {
    get_registry_dir(path, max_len);
    strncat(path, "/index.toml", max_len - strlen(path) - 1);
}

static void parse_string(toml_table_t* tab, const char* key, char* dest, size_t max_len) {
    toml_datum_t res = toml_string_in(tab, key);
    if (res.ok) {
        strncpy(dest, res.u.s, max_len - 1);
        dest[max_len - 1] = '\0';
        free(res.u.s);
    }
}

bool registry_load_local(PluginRegistry* registry) {
    char path[512];
    get_registry_index_path(path, sizeof(path));
    
    FILE* fp = fopen(path, "r");
    if (!fp) return false;

    memset(registry, 0, sizeof(PluginRegistry));

    char errbuf[200];
    toml_table_t* conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!conf) return false;

    toml_array_t* plugins = toml_array_in(conf, "plugin");
    if (plugins) {
        int count = toml_array_nelem(plugins);
        for (int i = 0; i < count && registry->count < MAX_REGISTRY_PLUGINS; i++) {
            toml_table_t* p = toml_table_at(plugins, i);
            if (p) {
                RegistryPlugin* rp = &registry->plugins[registry->count];
                parse_string(p, "name", rp->name, sizeof(rp->name));
                parse_string(p, "version", rp->version, sizeof(rp->version));
                parse_string(p, "description", rp->description, sizeof(rp->description));
                parse_string(p, "url", rp->url, sizeof(rp->url));
                parse_string(p, "checksum", rp->checksum, sizeof(rp->checksum));
                registry->count++;
            }
        }
    }

    toml_free(conf);
    return true;
}

bool registry_update(void) {
    char dir[512];
    get_registry_dir(dir, sizeof(dir));
    
    // Ensure registry directory exists
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", dir);
    if (system(cmd) != 0) return false;

    char path[512];
    get_registry_index_path(path, sizeof(path));

    printf("Fetching registry index from %s...\n", REGISTRY_URL);
    snprintf(cmd, sizeof(cmd), "curl -fsSL %s -o %s", REGISTRY_URL, path);
    int status = system(cmd);
    
    if (status != 0) {
        printf("Failed to fetch remote registry. Attempting local fallback...\n");
        // Fallback for local testing if the master branch doesn't have it yet
        snprintf(cmd, sizeof(cmd), "cp registry/index.toml %s 2>/dev/null", path);
        status = system(cmd);
    }
    
    return status == 0;
}

// Helper for case-insensitive substring search
static const char* stristr(const char* haystack, const char* needle) {
    if (!*needle) return haystack;
    for (; *haystack; ++haystack) {
        if (toupper(*haystack) == toupper(*needle)) {
            const char *h, *n;
            for (h = haystack, n = needle; *h && *n; ++h, ++n) {
                if (toupper(*h) != toupper(*n)) break;
            }
            if (!*n) return haystack;
        }
    }
    return NULL;
}

int registry_search(const PluginRegistry* registry, const char* query, RegistryPlugin* results, int max_results) {
    int found = 0;
    for (int i = 0; i < registry->count && found < max_results; i++) {
        const RegistryPlugin* p = &registry->plugins[i];
        if (!query || query[0] == '\0' || 
            stristr(p->name, query) != NULL || 
            stristr(p->description, query) != NULL) {
            results[found++] = *p;
        }
    }
    return found;
}

bool registry_find_exact(const PluginRegistry* registry, const char* name, RegistryPlugin* result) {
    for (int i = 0; i < registry->count; i++) {
        if (strcmp(registry->plugins[i].name, name) == 0) {
            *result = registry->plugins[i];
            return true;
        }
    }
    return false;
}

bool registry_install_plugin(const RegistryPlugin* plugin) {
    char plugins_dir[512];
    config_get_user_data_path(plugins_dir, sizeof(plugins_dir));
    strncat(plugins_dir, "/plugins", sizeof(plugins_dir) - strlen(plugins_dir) - 1);

    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", plugins_dir);
    if (system(cmd) != 0) {
        printf("Failed to create plugins directory.\n");
    }

    char tarball_path[512];
    snprintf(tarball_path, sizeof(tarball_path), "%s/%s-%s.tar.gz", plugins_dir, plugin->name, plugin->version);

    printf("Downloading %s from %s...\n", plugin->name, plugin->url);
    snprintf(cmd, sizeof(cmd), "curl -fsSL %s -o %s", plugin->url, tarball_path);
    if (system(cmd) != 0) {
        printf("Failed to download plugin.\n");
        return false;
    }

    printf("Extracting plugin...\n");
    char target_dir[512];
    snprintf(target_dir, sizeof(target_dir), "%s/%s", plugins_dir, plugin->name);
    
    snprintf(cmd, sizeof(cmd), "mkdir -p %s && tar -xzf %s -C %s", target_dir, tarball_path, target_dir);
    if (system(cmd) != 0) {
        printf("Failed to extract plugin.\n");
        remove(tarball_path);
        return false;
    }

    // Write version.lock
    char lock_path[1024];
    snprintf(lock_path, sizeof(lock_path), "%s/version.lock", target_dir);
    FILE* fp = fopen(lock_path, "w");
    if (fp) {
        fprintf(fp, "%s\n", plugin->version);
        fclose(fp);
    }

    remove(tarball_path);
    printf("Successfully installed %s v%s\n", plugin->name, plugin->version);
    return true;
}

bool registry_uninstall_plugin(const char* name) {
    char plugins_dir[512];
    config_get_user_data_path(plugins_dir, sizeof(plugins_dir));
    strncat(plugins_dir, "/plugins", sizeof(plugins_dir) - strlen(plugins_dir) - 1);

    char target_dir[512];
    snprintf(target_dir, sizeof(target_dir), "%s/%s", plugins_dir, name);

    struct stat st;
    if (stat(target_dir, &st) != 0) {
        printf("Plugin %s is not installed.\n", name);
        return false;
    }

    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", target_dir);
    if (system(cmd) == 0) {
        printf("Successfully uninstalled %s.\n", name);
        return true;
    }
    
    printf("Failed to uninstall %s.\n", name);
    return false;
}
