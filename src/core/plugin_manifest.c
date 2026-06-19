#include "plugin_manifest.h"
#include "toml.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void parse_string(toml_table_t* tab, const char* key, char* dest, size_t max_len) {
    toml_datum_t res = toml_string_in(tab, key);
    if (res.ok) {
        strncpy(dest, res.u.s, max_len - 1);
        dest[max_len - 1] = '\0';
        free(res.u.s);
    }
}

bool plugin_manifest_parse(const char* plugin_dir, PluginManifest* manifest) {
    char path[512];
    snprintf(path, sizeof(path), "%s/manifest.toml", plugin_dir);
    
    FILE* fp = fopen(path, "r");
    if (!fp) return false;

    memset(manifest, 0, sizeof(PluginManifest));

    char errbuf[200];
    toml_table_t* conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!conf) return false;

    toml_table_t* plugin = toml_table_in(conf, "plugin");
    if (plugin) {
        parse_string(plugin, "name", manifest->name, sizeof(manifest->name));
        parse_string(plugin, "version", manifest->version, sizeof(manifest->version));
        parse_string(plugin, "description", manifest->description, sizeof(manifest->description));
        parse_string(plugin, "author", manifest->author, sizeof(manifest->author));
        parse_string(plugin, "license", manifest->license, sizeof(manifest->license));
    }

    toml_array_t* reqs = toml_array_in(conf, "requirements");
    if (reqs) {
        manifest->num_requirements = toml_array_nelem(reqs);
        if (manifest->num_requirements > MAX_REQUIREMENTS) 
            manifest->num_requirements = MAX_REQUIREMENTS;
            
        for (int i = 0; i < manifest->num_requirements; i++) {
            toml_datum_t res = toml_string_at(reqs, i);
            if (res.ok) {
                strncpy(manifest->requirements[i], res.u.s, sizeof(manifest->requirements[i]) - 1);
                free(res.u.s);
            }
        }
    }

    toml_array_t* panes = toml_array_in(conf, "panes");
    if (panes) {
        manifest->num_panes = toml_array_nelem(panes);
        if (manifest->num_panes > 8) manifest->num_panes = 8;
        
        for (int i = 0; i < manifest->num_panes; i++) {
            toml_table_t* p = toml_table_at(panes, i);
            if (p) {
                parse_string(p, "name", manifest->panes[i].name, sizeof(manifest->panes[i].name));
                parse_string(p, "type", manifest->panes[i].type, sizeof(manifest->panes[i].type));
                
                toml_datum_t res = toml_int_in(p, "refresh_rate");
                if (res.ok) {
                    manifest->panes[i].refresh_rate = (int)res.u.i;
                } else {
                    manifest->panes[i].refresh_rate = 1000; // default 1s
                }
            }
        }
    }

    toml_free(conf);
    return true;
}
