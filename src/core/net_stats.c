#include "net_stats.h"
#include "proc_reader.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int net_stats_read(NetworkSnapshot* snap) {
    if (!snap) return -1;
    memset(snap, 0, sizeof(*snap));

    char buf[8192];
    if (read_file_to_buf("/proc/net/dev", buf, sizeof(buf)) <= 0) return -1;

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
            snprintf(iface->name, sizeof(iface->name), "%s", name);
            
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
            if (read_file_to_buf(sys_path, operstate, sizeof(operstate)) > 0) {
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

static void parse_hex_ip_port(const char* hex_str, char* out_buf, size_t out_size) {
    unsigned int ip, port;
    if (sscanf(hex_str, "%X:%X", &ip, &port) == 2) {
        snprintf(out_buf, out_size, "%u.%u.%u.%u:%u",
                 ip & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF, (ip >> 24) & 0xFF, port);
    } else {
        out_buf[0] = '\0';
    }
}

static void parse_proc_net_file(const char* filepath, ConnectionTableSnapshot* snap, ConnectionType type) {
    FILE* fp = fopen(filepath, "r");
    if (!fp) return;
    
    char line[1024];
    /* skip header */
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return;
    }
    
    while (fgets(line, sizeof(line), fp) && snap->num_connections < snap->capacity) {
        char local_hex[64], remote_hex[64];
        unsigned int state_hex, tx_q, rx_q, uid;
        unsigned long long inode;
        
        if (sscanf(line, "%*d: %63s %63s %X %X:%X %*X:%*X %*X %u %*d %llu",
                   local_hex, remote_hex, &state_hex, &tx_q, &rx_q, &uid, &inode) >= 7) {
            
            ConnectionSnapshot* conn = &snap->connections[snap->num_connections];
            conn->type = type;
            
            if (type == CONN_TCP) {
                switch(state_hex) {
                    case 0x01: conn->state = CONN_ESTABLISHED; break;
                    case 0x02: conn->state = CONN_SYN_SENT; break;
                    case 0x0A: conn->state = CONN_LISTEN; break;
                    case 0x08: conn->state = CONN_CLOSE_WAIT; break;
                    case 0x06: conn->state = CONN_TIME_WAIT; break;
                    default: conn->state = CONN_OTHER; break;
                }
            } else {
                conn->state = CONN_OTHER;
            }
            
            parse_hex_ip_port(local_hex, conn->local_addr, sizeof(conn->local_addr));
            parse_hex_ip_port(remote_hex, conn->remote_addr, sizeof(conn->remote_addr));
            
            conn->pid = -1;
            conn->uid = uid;
            conn->inode = inode;
            conn->tx_queue = tx_q;
            conn->rx_queue = rx_q;
            
            snap->num_connections++;
        }
    }
    fclose(fp);
}

int net_connections_read(ConnectionTableSnapshot* snap) {
    if (!snap) return -1;
    snap->num_connections = 0;
    
    parse_proc_net_file("/proc/net/tcp", snap, CONN_TCP);
    parse_proc_net_file("/proc/net/udp", snap, CONN_UDP);
    
    return 0;
}
