#include "sysinfo.h"
#include "proc_reader.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <stdlib.h>

int sysinfo_collect(SystemInfo* info) {
    if (!info) return -1;
    memset(info, 0, sizeof(*info));
    
    struct utsname un;
    if (uname(&un) == 0) {
        size_t i;
        for (i = 0; i < sizeof(info->hostname) - 1 && un.nodename[i]; i++) info->hostname[i] = un.nodename[i];
        info->hostname[i] = '\0';
        for (i = 0; i < sizeof(info->kernel_release) - 1 && un.release[i]; i++) info->kernel_release[i] = un.release[i];
        info->kernel_release[i] = '\0';
        for (i = 0; i < sizeof(info->kernel_version) - 1 && un.version[i]; i++) info->kernel_version[i] = un.version[i];
        info->kernel_version[i] = '\0';
        for (i = 0; i < sizeof(info->architecture) - 1 && un.machine[i]; i++) info->architecture[i] = un.machine[i];
        info->architecture[i] = '\0';
    }
    
    info->page_size = sysconf(_SC_PAGE_SIZE);
    info->clock_ticks_per_sec = sysconf(_SC_CLK_TCK);
    info->num_cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);
    
    char buf[8192];
    if (read_file_to_buf("/proc/meminfo", buf, sizeof(buf)) > 0) {
        char* line = strtok(buf, "\n");
        while (line) {
            if (strncmp(line, "MemTotal:", 9) == 0) {
                uint64_t kb = parse_uint64(line + 9);
                info->total_memory_bytes = kb * 1024;
            } else if (strncmp(line, "SwapTotal:", 10) == 0) {
                uint64_t kb = parse_uint64(line + 10);
                info->total_swap_bytes = kb * 1024;
            }
            line = strtok(NULL, "\n");
        }
    }
    
    if (read_file_to_buf("/proc/stat", buf, sizeof(buf)) > 0) {
        char* line = strtok(buf, "\n");
        while (line) {
            if (strncmp(line, "btime ", 6) == 0) {
                info->boot_time_epoch = parse_uint64(line + 6);
                break;
            }
            line = strtok(NULL, "\n");
        }
    }
    
    return 0;
}
