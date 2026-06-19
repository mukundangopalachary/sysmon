#include "mem_stats.h"
#include "proc_reader.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int mem_stats_read(MemorySnapshot* snap) {
    if (!snap) return -1;
    memset(snap, 0, sizeof(*snap));

    char buf[8192];
    if (read_file_to_buf("/proc/meminfo", buf, sizeof(buf)) != 0) return -1;

    uint64_t hugepages_size = 0;
    uint64_t hugepages_total = 0;
    uint64_t hugepages_free = 0;

    char* saveptr;
    char* line = strtok_r(buf, "\n", &saveptr);
    while (line) {
        char key[64];
        unsigned long long val_kb;
        if (sscanf(line, "%63[^:]: %llu", key, &val_kb) == 2) {
            uint64_t val = val_kb * 1024;
            if (strcmp(key, "MemTotal") == 0) snap->total_bytes = val;
            else if (strcmp(key, "MemFree") == 0) snap->free_bytes = val;
            else if (strcmp(key, "MemAvailable") == 0) snap->available_bytes = val;
            else if (strcmp(key, "Buffers") == 0) snap->buffers_bytes = val;
            else if (strcmp(key, "Cached") == 0) snap->cached_bytes = val;
            else if (strcmp(key, "Shmem") == 0) snap->shared_bytes = val;
            else if (strcmp(key, "Slab") == 0) snap->slab_bytes = val;
            else if (strcmp(key, "KernelStack") == 0) snap->kernel_stack_bytes = val;
            else if (strcmp(key, "PageTables") == 0) snap->page_tables_bytes = val;
            else if (strcmp(key, "SwapTotal") == 0) snap->swap_total_bytes = val;
            else if (strcmp(key, "SwapFree") == 0) snap->swap_free_bytes = val;
            else if (strcmp(key, "HugePages_Total") == 0) hugepages_total = val_kb;
            else if (strcmp(key, "HugePages_Free") == 0) hugepages_free = val_kb;
            else if (strcmp(key, "Hugepagesize") == 0) hugepages_size = val;
        }
        line = strtok_r(NULL, "\n", &saveptr);
    }
    
    snap->hugepages_total_bytes = hugepages_total * hugepages_size;
    snap->hugepages_free_bytes = hugepages_free * hugepages_size;

    if (snap->available_bytes == 0 && snap->free_bytes > 0) {
        snap->available_bytes = snap->free_bytes + snap->cached_bytes + snap->buffers_bytes;
    }

    if (snap->total_bytes > snap->available_bytes) {
        snap->used_bytes = snap->total_bytes - snap->available_bytes;
    }
    
    if (snap->total_bytes > 0) {
        snap->usage_percent = (double)snap->used_bytes / snap->total_bytes * 100.0;
    }
    
    if (snap->swap_total_bytes > snap->swap_free_bytes) {
        snap->swap_used_bytes = snap->swap_total_bytes - snap->swap_free_bytes;
    }
    
    if (snap->swap_total_bytes > 0) {
        snap->swap_usage_percent = (double)snap->swap_used_bytes / snap->swap_total_bytes * 100.0;
    }

    return 0;
}
