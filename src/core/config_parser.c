#include "config_parser.h"
#include "toml.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void config_init_defaults(SysmonConfig* cfg) {
    memset(cfg, 0, sizeof(SysmonConfig));
    cfg->collection.interval_ms = 1000;
    cfg->collection.max_processes = 10000;
    
    strcpy(cfg->display.theme, "default");
    cfg->display.refresh_rate_hz = 30;
    cfg->display.compact_mode = false;
    
    strcpy(cfg->sorting.process_default_column, "cpu");
    strcpy(cfg->sorting.process_default_order, "descending");
    
    strcpy(cfg->keybindings.quit, "q");
    strcpy(cfg->keybindings.dashboard, "F1");
    strcpy(cfg->keybindings.process_list, "F2");
    strcpy(cfg->keybindings.connections, "F3");
    strcpy(cfg->keybindings.plugins, "F4");
    strcpy(cfg->keybindings.help, "?");
    
    cfg->alerts.enabled = false;
    cfg->alerts.cpu_threshold = 90;
    cfg->alerts.memory_threshold = 95;
}

static void parse_string(toml_table_t* tab, const char* key, char* dest, size_t max_len) {
    toml_datum_t res = toml_string_in(tab, key);
    if (res.ok) {
        strncpy(dest, res.u.s, max_len - 1);
        dest[max_len - 1] = '\0';
        free(res.u.s);
    }
}

static void parse_int(toml_table_t* tab, const char* key, int* dest) {
    toml_datum_t res = toml_int_in(tab, key);
    if (res.ok) *dest = (int)res.u.i;
}

static void parse_bool(toml_table_t* tab, const char* key, bool* dest) {
    toml_datum_t res = toml_bool_in(tab, key);
    if (res.ok) *dest = (bool)res.u.b;
}

bool config_load(SysmonConfig* cfg, const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) return false;

    char errbuf[200];
    toml_table_t* conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!conf) return false;

    toml_table_t* collection = toml_table_in(conf, "collection");
    if (collection) {
        parse_int(collection, "interval_ms", &cfg->collection.interval_ms);
        parse_int(collection, "max_processes", &cfg->collection.max_processes);
    }

    toml_table_t* display = toml_table_in(conf, "display");
    if (display) {
        parse_string(display, "theme", cfg->display.theme, sizeof(cfg->display.theme));
        parse_int(display, "refresh_rate_hz", &cfg->display.refresh_rate_hz);
        parse_bool(display, "compact_mode", &cfg->display.compact_mode);
    }

    toml_table_t* sorting = toml_table_in(conf, "sorting");
    if (sorting) {
        parse_string(sorting, "process_default_column", cfg->sorting.process_default_column, sizeof(cfg->sorting.process_default_column));
        parse_string(sorting, "process_default_order", cfg->sorting.process_default_order, sizeof(cfg->sorting.process_default_order));
    }

    toml_table_t* keybindings = toml_table_in(conf, "keybindings");
    if (keybindings) {
        parse_string(keybindings, "quit", cfg->keybindings.quit, sizeof(cfg->keybindings.quit));
        parse_string(keybindings, "dashboard", cfg->keybindings.dashboard, sizeof(cfg->keybindings.dashboard));
        parse_string(keybindings, "process_list", cfg->keybindings.process_list, sizeof(cfg->keybindings.process_list));
        parse_string(keybindings, "connections", cfg->keybindings.connections, sizeof(cfg->keybindings.connections));
        parse_string(keybindings, "plugins", cfg->keybindings.plugins, sizeof(cfg->keybindings.plugins));
        parse_string(keybindings, "help", cfg->keybindings.help, sizeof(cfg->keybindings.help));
    }

    toml_table_t* alerts = toml_table_in(conf, "alerts");
    if (alerts) {
        parse_bool(alerts, "enabled", &cfg->alerts.enabled);
        parse_int(alerts, "cpu_threshold", &cfg->alerts.cpu_threshold);
        parse_int(alerts, "memory_threshold", &cfg->alerts.memory_threshold);
    }

    toml_free(conf);
    return true;
}

bool config_save(const SysmonConfig* cfg, const char* path) {
    FILE* fp = fopen(path, "w");
    if (!fp) return false;

    fprintf(fp, "[collection]\n");
    fprintf(fp, "interval_ms = %d\n", cfg->collection.interval_ms);
    fprintf(fp, "max_processes = %d\n\n", cfg->collection.max_processes);

    fprintf(fp, "[display]\n");
    fprintf(fp, "theme = \"%s\"\n", cfg->display.theme);
    fprintf(fp, "refresh_rate_hz = %d\n", cfg->display.refresh_rate_hz);
    fprintf(fp, "compact_mode = %s\n\n", cfg->display.compact_mode ? "true" : "false");

    fprintf(fp, "[sorting]\n");
    fprintf(fp, "process_default_column = \"%s\"\n", cfg->sorting.process_default_column);
    fprintf(fp, "process_default_order = \"%s\"\n\n", cfg->sorting.process_default_order);

    fprintf(fp, "[keybindings]\n");
    fprintf(fp, "quit = \"%s\"\n", cfg->keybindings.quit);
    fprintf(fp, "dashboard = \"%s\"\n", cfg->keybindings.dashboard);
    fprintf(fp, "process_list = \"%s\"\n", cfg->keybindings.process_list);
    fprintf(fp, "connections = \"%s\"\n", cfg->keybindings.connections);
    fprintf(fp, "plugins = \"%s\"\n", cfg->keybindings.plugins);
    fprintf(fp, "help = \"%s\"\n\n", cfg->keybindings.help);

    fprintf(fp, "[alerts]\n");
    fprintf(fp, "enabled = %s\n", cfg->alerts.enabled ? "true" : "false");
    fprintf(fp, "cpu_threshold = %d\n", cfg->alerts.cpu_threshold);
    fprintf(fp, "memory_threshold = %d\n", cfg->alerts.memory_threshold);

    fclose(fp);
    return true;
}

void config_print(const SysmonConfig* cfg) {
    printf("[collection]\n");
    printf("interval_ms = %d\n", cfg->collection.interval_ms);
    printf("max_processes = %d\n\n", cfg->collection.max_processes);

    printf("[display]\n");
    printf("theme = \"%s\"\n", cfg->display.theme);
    printf("refresh_rate_hz = %d\n", cfg->display.refresh_rate_hz);
    printf("compact_mode = %s\n\n", cfg->display.compact_mode ? "true" : "false");

    printf("[sorting]\n");
    printf("process_default_column = \"%s\"\n", cfg->sorting.process_default_column);
    printf("process_default_order = \"%s\"\n\n", cfg->sorting.process_default_order);

    printf("[keybindings]\n");
    printf("quit = \"%s\"\n", cfg->keybindings.quit);
    printf("dashboard = \"%s\"\n", cfg->keybindings.dashboard);
    printf("process_list = \"%s\"\n", cfg->keybindings.process_list);
    printf("connections = \"%s\"\n", cfg->keybindings.connections);
    printf("plugins = \"%s\"\n", cfg->keybindings.plugins);
    printf("help = \"%s\"\n\n", cfg->keybindings.help);

    printf("[alerts]\n");
    printf("enabled = %s\n", cfg->alerts.enabled ? "true" : "false");
    printf("cpu_threshold = %d\n", cfg->alerts.cpu_threshold);
    printf("memory_threshold = %d\n", cfg->alerts.memory_threshold);
}
