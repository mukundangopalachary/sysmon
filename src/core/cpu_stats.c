#include "cpu_stats.h"
#include "proc_reader.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int cpu_stats_read(CpuSnapshot* snap) {
    if (!snap) return -1;
    memset(snap, 0, sizeof(*snap));

    struct timeval tv;
    gettimeofday(&tv, NULL);
    snap->timestamp_us = (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;

    char buf[8192];
    if (read_file_to_buf("/proc/stat", buf, sizeof(buf)) > 0) {
        char* saveptr;
        char* line = strtok_r(buf, "\n", &saveptr);
        while (line) {
            if (strncmp(line, "cpu", 3) == 0 && line[3] >= '0' && line[3] <= '9') {
                if (snap->num_cores < 256) {
                    CpuCoreSnapshot* core = &snap->cores[snap->num_cores];
                    unsigned long long u=0, n=0, s=0, i=0, iw=0, irq=0, sirq=0, st=0, g=0, gn=0;
                    int parsed = sscanf(line + 4, "%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                           &u, &n, &s, &i, &iw, &irq, &sirq, &st, &g, &gn);
                    if (parsed >= 4) {
                        core->user = u; core->nice = n; core->system = s; core->idle = i;
                        core->iowait = iw; core->irq = irq; core->softirq = sirq; core->steal = st;
                        core->guest = g; core->guest_nice = gn;
                        
                        core->total_ticks = core->user + core->nice + core->system + core->idle +
                                            core->iowait + core->irq + core->softirq + core->steal +
                                            core->guest + core->guest_nice;
                        core->active_ticks = core->total_ticks - core->idle - core->iowait;
                        core->usage_percent = 0.0;
                        
                        snap->num_cores++;
                    }
                }
            } else if (strncmp(line, "ctxt", 4) == 0) {
                sscanf(line + 5, "%llu", (unsigned long long*)&snap->context_switches);
            } else if (strncmp(line, "intr", 4) == 0) {
                sscanf(line + 5, "%llu", (unsigned long long*)&snap->interrupts);
            }
            line = strtok_r(NULL, "\n", &saveptr);
        }
    }

    if (read_file_to_buf("/proc/loadavg", buf, sizeof(buf)) > 0) {
        sscanf(buf, "%lf %lf %lf", &snap->load_avg_1min, &snap->load_avg_5min, &snap->load_avg_15min);
    }

    return 0;
}
