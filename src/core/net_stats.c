#include "net_stats.h"
#include "proc_reader.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int net_stats_read(NetworkSnapshot* snap) {
    if (!snap) return -1;
    memset(snap, 0, sizeof(*snap));

    char buf[8192];
    if (read_file_to_buf("/proc/net/dev", buf, sizeof(buf)) != 0) return -1;

    char* saveptr;
    char* line = strtok_r(buf, "\n", &saveptr);
    /* skip first two lines */
    if (line) line = strtok_r(NULL, "\n", &saveptr);
    if (line) line = strtok_r(NULL, "\n", &saveptr);

    while (line && snap->num_interfaces < 32) {
        char name[64];
        unsigned long long rb, rp, re, rd, rfifo, rframe, rcomp, rmcast;
        unsigned long long tb, tp, te, td, tfifo, tcol, tcarrier, tcomp;
        
        if (sscanf(line, " %63[^:]: %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                   name,
                   &rb, &rp, &re, &rd, &rfifo, &rframe, &rcomp, &rmcast,
                   &tb, &tp, &te, &td, &tfifo, &tcol, &tcarrier, &tcomp) >= 17) {
            
            NetworkIfaceSnapshot* iface = &snap->interfaces[snap->num_interfaces];
            strncpy(iface->name, name, sizeof(iface->name) - 1);
            iface->name[sizeof(iface->name)-1] = '\0';
            
            iface->rx_bytes = rb;
            iface->rx_packets = rp;
            iface->rx_errors = re;
            iface->rx_dropped = rd;
            iface->tx_bytes = tb;
            iface->tx_packets = tp;
            iface->tx_errors = te;
            iface->tx_dropped = td;
            
            if (strncmp(iface->name, "lo", 2) == 0) {
                iface->is_loopback = true;
            } else {
                iface->is_loopback = false;
            }
            
            char sys_path[256];
            snprintf(sys_path, sizeof(sys_path), "/sys/class/net/%s/operstate", iface->name);
            char operstate[32];
            if (read_file_to_buf(sys_path, operstate, sizeof(operstate)) == 0) {
                if (strncmp(operstate, "up", 2) == 0 || strncmp(operstate, "unknown", 7) == 0) {
                    iface->is_up = true;
                } else {
                    iface->is_up = false;
                }
            } else {
                iface->is_up = true;
            }
            
            snap->num_interfaces++;
        }
        line = strtok_r(NULL, "\n", &saveptr);
    }

    return 0;
}
