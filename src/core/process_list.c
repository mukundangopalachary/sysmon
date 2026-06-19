#include "process_list.h"
#include "proc_reader.h"
#include "username_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>

int process_list_read(ProcessTableSnapshot* snap, int max_count) {
    if (!snap || !snap->processes) {
        return -1;
    }
    if (max_count > snap->capacity) {
        max_count = snap->capacity;
    }

    DIR* dir = opendir("/proc");
    if (!dir) {
        return -1;
    }

    int count = 0;
    struct dirent* ent;
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size < 0) {
        page_size = 4096;
    }

    while ((ent = readdir(dir)) != NULL && count < max_count) {
        // Check if directory name is all digits
        int is_pid = 1;
        for (int i = 0; ent->d_name[i]; i++) {
            if (!isdigit((unsigned char)ent->d_name[i])) {
                is_pid = 0;
                break;
            }
        }
        if (!is_pid) continue;

        int pid = atoi(ent->d_name);
        ProcessSnapshot* ps = &snap->processes[count];
        memset(ps, 0, sizeof(ProcessSnapshot));
        ps->pid = pid;

        char path[256];
        char buf[8192];

        // 1. Read /proc/[pid]/stat
        snprintf(path, sizeof(path), "/proc/%d/stat", pid);
        if (read_file_to_buf(path, buf, sizeof(buf)) > 0) {
            char* lp = strchr(buf, '(');
            char* rp = strrchr(buf, ')');
            if (lp && rp && rp > lp) {
                size_t comm_len = rp - lp - 1;
                if (comm_len >= sizeof(ps->comm)) comm_len = sizeof(ps->comm) - 1;
                strncpy(ps->comm, lp + 1, comm_len);
                ps->comm[comm_len] = '\0';

                char* p = rp + 2;
                if (*p) {
                    ps->state = *p;
                    ps->is_zombie = (ps->state == 'Z');
                    p += 2;

                    char* saveptr;
                    char* token = strtok_r(p, " ", &saveptr);
                    int tok_idx = 1; // ppid is index 1
                    while (token) {
                        switch (tok_idx) {
                            case 1: ps->ppid = atoi(token); break;
                            case 2: ps->pgrp = atoi(token); break;
                            case 3: ps->session = atoi(token); break;
                            case 4: ps->tty_nr = atoi(token); break;
                            case 11: ps->utime_ticks = parse_uint64(token); break;
                            case 12: ps->stime_ticks = parse_uint64(token); break;
                            case 16: ps->nice = atoi(token); break;
                            case 17: ps->num_threads = atoi(token); break;
                            case 19: ps->starttime_ticks = parse_uint64(token); break;
                            case 20: ps->vsize_bytes = parse_uint64(token); break;
                            case 21: ps->rss_bytes = parse_uint64(token) * page_size; break;
                            case 32: ps->wchan = parse_uint64(token); break;
                        }
                        tok_idx++;
                        token = strtok_r(NULL, " ", &saveptr);
                    }
                }
            }
        } else {
            // Process might have died, skip it
            continue;
        }

        // 2. Read /proc/[pid]/status
        snprintf(path, sizeof(path), "/proc/%d/status", pid);
        if (read_file_to_buf(path, buf, sizeof(buf)) > 0) {
            char* saveptr;
            char* line = strtok_r(buf, "\n", &saveptr);
            while (line) {
                if (strncmp(line, "Uid:", 4) == 0) {
                    int uid;
                    if (sscanf(line + 4, "%d", &uid) == 1) {
                        ps->uid = (uid_t)uid;
                        const char* un = username_cache_lookup(ps->uid);
                        if (un) {
                            strncpy(ps->username, un, sizeof(ps->username) - 1);
                            ps->username[sizeof(ps->username) - 1] = '\0';
                        }
                    }
                    break;
                }
                line = strtok_r(NULL, "\n", &saveptr);
            }
        }

        // 3. Read /proc/[pid]/cmdline
        snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
        char cmdline_buf[8];
        if (read_file_to_buf(path, cmdline_buf, sizeof(cmdline_buf)) <= 0) {
            ps->is_kernel_thread = true;
        } else {
            ps->is_kernel_thread = false;
        }

        count++;
    }

    closedir(dir);
    snap->num_processes = count;
    return 0;
}
