#include "disk_stats.h"
#include "proc_reader.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int disk_stats_read(DiskSnapshot* snap) {
    if (!snap) return -1;
    memset(snap, 0, sizeof(*snap));

    char buf[16384];
    if (read_file_to_buf("/proc/diskstats", buf, sizeof(buf)) <= 0) return -1;

    char* saveptr;
    char* line = strtok_r(buf, "\n", &saveptr);
    while (line && snap->num_devices < 32) {
        int major, minor;
        char name[64];
        unsigned long long rio, rmerge, rsect, ruse, wio, wmerge, wsect, wuse, inprog, io_ticks, time_in_queue;
        
        int parsed = sscanf(line, " %d %d %63s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                            &major, &minor, name,
                            &rio, &rmerge, &rsect, &ruse,
                            &wio, &wmerge, &wsect, &wuse,
                            &inprog, &io_ticks, &time_in_queue);
        
        if (parsed >= 14) {
            if (strncmp(name, "loop", 4) == 0 || strncmp(name, "ram", 3) == 0) {
                line = strtok_r(NULL, "\n", &saveptr);
                continue;
            }
            
            DiskStatsSnapshot* disk = &snap->devices[snap->num_devices];
            strncpy(disk->device_name, name, sizeof(disk->device_name) - 1);
            disk->device_name[sizeof(disk->device_name)-1] = '\0';
            
            disk->read_ios = rio;
            disk->read_sectors = rsect;
            disk->read_ticks_ms = ruse;
            disk->write_ios = wio;
            disk->write_sectors = wsect;
            disk->write_ticks_ms = wuse;
            disk->io_ticks_ms = io_ticks;
            
            snap->num_devices++;
        }
        line = strtok_r(NULL, "\n", &saveptr);
    }

    return 0;
}
