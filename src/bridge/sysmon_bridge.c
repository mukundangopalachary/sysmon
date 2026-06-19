#include "sysmon_bridge.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

const char* format_bytes(uint64_t bytes) {
    static char buf[64];
    const char* suffixes[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double dblBytes = (double)bytes;
    while (dblBytes >= 1024.0 && i < 4) {
        dblBytes /= 1024.0;
        i++;
    }
    if (i == 0) {
        snprintf(buf, sizeof(buf), "%llu %s", (unsigned long long)bytes, suffixes[i]);
    } else {
        snprintf(buf, sizeof(buf), "%.2f %s", dblBytes, suffixes[i]);
    }
    return buf;
}

const char* format_duration_us(uint64_t us) {
    static char buf[64];
    uint64_t ms = us / 1000;
    if (ms < 1000) {
        snprintf(buf, sizeof(buf), "%llu ms", (unsigned long long)ms);
    } else {
        uint64_t s = ms / 1000;
        if (s < 60) {
            snprintf(buf, sizeof(buf), "%llu s", (unsigned long long)s);
        } else {
            uint64_t m = s / 60;
            s = s % 60;
            if (m < 60) {
                snprintf(buf, sizeof(buf), "%llu m %llu s", (unsigned long long)m, (unsigned long long)s);
            } else {
                uint64_t h = m / 60;
                m = m % 60;
                snprintf(buf, sizeof(buf), "%llu h %llu m", (unsigned long long)h, (unsigned long long)m);
            }
        }
    }
    return buf;
}

uint64_t get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

const ProcessSnapshot* process_find_by_pid(const SystemSnapshot* snap, int pid) {
    if (!snap) return NULL;
    for (int i = 0; i < snap->processes.num_processes; ++i) {
        if (snap->processes.processes[i].pid == pid) {
            return &snap->processes.processes[i];
        }
    }
    return NULL;
}

int process_get_children(const SystemSnapshot* snap, int ppid, const ProcessSnapshot** children, int max_children) {
    if (!snap || !children || max_children <= 0) return 0;
    int count = 0;
    for (int i = 0; i < snap->processes.num_processes; ++i) {
        if (snap->processes.processes[i].ppid == ppid) {
            if (count < max_children) {
                children[count] = &snap->processes.processes[i];
                count++;
            }
        }
    }
    return count;
}
