#include "collection_engine.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "cpu_stats.h"
#include "mem_stats.h"
#include "net_stats.h"
#include "disk_stats.h"
#include "process_list.h"
#include "sysinfo.h"

static uint64_t get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + (uint64_t)ts.tv_nsec / 1000;
}

static void compute_cpu_percentages(SystemSnapshot* snap, CpuCoreSnapshot* prev_cpu, uint64_t prev_timestamp_us) {
    if (prev_timestamp_us == 0) return;
    
    for (int i = 0; i < snap->cpu.num_cores; i++) {
        CpuCoreSnapshot* curr = &snap->cpu.cores[i];
        CpuCoreSnapshot* prev = &prev_cpu[i];
        
        uint64_t total_delta = curr->total_ticks - prev->total_ticks;
        uint64_t active_delta = curr->active_ticks - prev->active_ticks;
        
        if (total_delta > 0) {
            curr->usage_percent = ((double)active_delta / total_delta) * 100.0;
            curr->user_percent = ((double)(curr->user + curr->nice - prev->user - prev->nice) / total_delta) * 100.0;
            curr->sys_percent = ((double)(curr->system + curr->irq + curr->softirq - prev->system - prev->irq - prev->softirq) / total_delta) * 100.0;
            curr->iowait_percent = ((double)(curr->iowait - prev->iowait) / total_delta) * 100.0;
            curr->steal_percent = ((double)(curr->steal - prev->steal) / total_delta) * 100.0;
        } else {
            curr->usage_percent = 0.0;
            curr->user_percent = 0.0;
            curr->sys_percent = 0.0;
            curr->iowait_percent = 0.0;
            curr->steal_percent = 0.0;
        }
    }
}

static void compute_global_rates(SystemSnapshot* snap, CollectionEngine* engine) {
    if (engine->prev_timestamp_us == 0) return;
    
    uint64_t delta_us = snap->collection_timestamp_us - engine->prev_timestamp_us;
    if (delta_us == 0) return;
    
    double delta_sec = delta_us / 1000000.0;
    
    if (snap->cpu.context_switches >= engine->prev_ctxt) {
        snap->cpu.ctxt_rate = (snap->cpu.context_switches - engine->prev_ctxt) / delta_sec;
    }
    
    if (snap->cpu.interrupts >= engine->prev_irq) {
        snap->cpu.irq_rate = (snap->cpu.interrupts - engine->prev_irq) / delta_sec;
    }
}

static void compute_network_rates(SystemSnapshot* snap, NetworkIfaceSnapshot* prev_net, uint64_t prev_timestamp_us) {
    if (prev_timestamp_us == 0) return;
    uint64_t current_time = snap->collection_timestamp_us;
    double delta_sec = (current_time - prev_timestamp_us) / 1000000.0;
    
    if (delta_sec <= 0.0) return;
    
    for (int i = 0; i < snap->network.num_interfaces; i++) {
        NetworkIfaceSnapshot* curr = &snap->network.interfaces[i];
        
        for (int j = 0; j < 32; j++) {
            NetworkIfaceSnapshot* prev = &prev_net[j];
            if (prev->name[0] != '\0' && strcmp(curr->name, prev->name) == 0) {
                if (curr->rx_bytes >= prev->rx_bytes)
                    curr->rx_bytes_per_sec = (curr->rx_bytes - prev->rx_bytes) / delta_sec;
                if (curr->tx_bytes >= prev->tx_bytes)
                    curr->tx_bytes_per_sec = (curr->tx_bytes - prev->tx_bytes) / delta_sec;
                if (curr->rx_packets >= prev->rx_packets)
                    curr->rx_packets_per_sec = (curr->rx_packets - prev->rx_packets) / delta_sec;
                if (curr->tx_packets >= prev->tx_packets)
                    curr->tx_packets_per_sec = (curr->tx_packets - prev->tx_packets) / delta_sec;
                break;
            }
        }
    }
}

static void compute_process_cpu_percentages(SystemSnapshot* snap, SnapshotManager* mgr) {
    const SystemSnapshot* prev_snap = snapshot_manager_get_current(mgr);
    if (!prev_snap) {
        for (int i = 0; i < snap->processes.num_processes; i++) {
            snap->processes.processes[i].cpu_percent = 0.0;
        }
        return;
    }
    
    uint64_t time_delta_us = snap->collection_timestamp_us - prev_snap->collection_timestamp_us;
    if (time_delta_us == 0) return;
    
    double time_delta_sec = time_delta_us / 1000000.0;
    if (snap->system_info && snap->system_info->clock_ticks_per_sec > 0) {
        double ticks_per_sec = (double)snap->system_info->clock_ticks_per_sec;
        
        for (int i = 0; i < snap->processes.num_processes; i++) {
            ProcessSnapshot* curr = &snap->processes.processes[i];
            
            ProcessSnapshot* prev = NULL;
            int left = 0, right = prev_snap->processes.num_processes - 1;
            while (left <= right) {
                int mid = left + (right - left) / 2;
                if (prev_snap->processes.processes[mid].pid == curr->pid) {
                    prev = &prev_snap->processes.processes[mid];
                    break;
                } else if (prev_snap->processes.processes[mid].pid < curr->pid) {
                    left = mid + 1;
                } else {
                    right = mid - 1;
                }
            }
            
            if (prev && curr->starttime_ticks == prev->starttime_ticks) {
                uint64_t curr_total = curr->utime_ticks + curr->stime_ticks;
                uint64_t prev_total = prev->utime_ticks + prev->stime_ticks;
                
                if (curr_total >= prev_total) {
                    double delta_ticks = (double)(curr_total - prev_total);
                    curr->cpu_percent = (delta_ticks / ticks_per_sec) / time_delta_sec * 100.0;
                } else {
                    curr->cpu_percent = 0.0;
                }
            } else {
                curr->cpu_percent = 0.0;
            }
        }
    }
}

void* collection_thread_func(void* arg) {
    CollectionEngine* engine = (CollectionEngine*)arg;
    bool first_run = true;
    
    while (engine->running) {
        uint64_t start_time = get_time_us();
        
        if (engine->on_collection_start) {
            engine->on_collection_start();
        }
        
        SystemSnapshot* snap = snapshot_manager_get_next(engine->snapshot_mgr);
        if (!snap) {
            usleep(100000);
            continue;
        }
        
        snap->collection_timestamp_us = start_time;
        
        if (first_run) {
            sysinfo_collect(snap->system_info);
            first_run = false;
        }
        
        cpu_stats_read(&snap->cpu);
        mem_stats_read(&snap->memory);
        net_stats_read(&snap->network);
        
        if (engine->collect_connections) {
            if (!snap->connections) {
                snap->connections = calloc(1, sizeof(ConnectionTableSnapshot));
                snap->connections->connections = calloc(4096, sizeof(ConnectionSnapshot));
                snap->connections->capacity = 4096;
            }
            net_connections_read(snap->connections);
        }
        
        if (engine->collect_disk_io) {
            if (!snap->disk) {
                snap->disk = calloc(1, sizeof(DiskSnapshot));
            }
            disk_stats_read(snap->disk);
        }
        
        process_list_read(&snap->processes, engine->process_max_count);
        
        compute_cpu_percentages(snap, engine->prev_cpu, engine->prev_timestamp_us);
        compute_network_rates(snap, engine->prev_net, engine->prev_timestamp_us);
        compute_process_cpu_percentages(snap, engine->snapshot_mgr);
        compute_global_rates(snap, engine);
        
        memcpy(engine->prev_cpu, snap->cpu.cores, sizeof(CpuCoreSnapshot) * snap->cpu.num_cores);
        memset(engine->prev_net, 0, sizeof(NetworkIfaceSnapshot) * 32);
        memcpy(engine->prev_net, snap->network.interfaces, sizeof(NetworkIfaceSnapshot) * snap->network.num_interfaces);
        engine->prev_timestamp_us = start_time;
        engine->prev_ctxt = snap->cpu.context_switches;
        engine->prev_irq = snap->cpu.interrupts;
        
        snap->collection_sequence++;
        snap->collection_duration_us = get_time_us() - start_time;
        
        plugin_manager_collect(&engine->plugin_mgr, &engine->plugin_data_buf);
        snap->plugin_data = &engine->plugin_data_buf;

        snapshot_manager_publish(engine->snapshot_mgr);
        
        if (engine->on_collection_complete) {
            engine->on_collection_complete(snap);
        }
        
        uint64_t elapsed_us = get_time_us() - start_time;
        uint64_t interval_us = (uint64_t)engine->collection_interval_ms * 1000;
        
        if (elapsed_us < interval_us) {
            uint64_t remaining_us = interval_us - elapsed_us;
            while (remaining_us > 0 && engine->running) {
                uint64_t sleep_time = remaining_us > 50000 ? 50000 : remaining_us;
                usleep(sleep_time);
                remaining_us -= sleep_time;
            }
        }
    }
    
    return NULL;
}

int collection_engine_init(CollectionEngine* engine, SnapshotManager* mgr) {
    if (!engine || !mgr) return -1;
    
    memset(engine, 0, sizeof(*engine));
    engine->snapshot_mgr = mgr;
    engine->collection_interval_ms = 1000;
    engine->process_max_count = 10000;
    engine->collect_disk_io = true;
    engine->collect_connections = true;
    engine->running = false;
    
    engine->prev_cpu = calloc(256, sizeof(CpuCoreSnapshot));
    engine->prev_net = calloc(32, sizeof(NetworkIfaceSnapshot));
    
    return 0;
}

int collection_engine_start(CollectionEngine* engine) {
    if (!engine || engine->running) return -1;
    
    plugin_manager_init(&engine->plugin_mgr, "scripts/plugins");
    plugin_manager_start_all(&engine->plugin_mgr);
    
    engine->running = true;
    if (pthread_create(&engine->thread, NULL, collection_thread_func, engine) != 0) {
        engine->running = false;
        return -1;
    }
    
    return 0;
}

int collection_engine_stop(CollectionEngine* engine) {
    if (!engine || !engine->running) return -1;
    
    engine->running = false;
    pthread_join(engine->thread, NULL);
    
    plugin_manager_stop_all(&engine->plugin_mgr);
    
    return 0;
}

void collection_engine_destroy(CollectionEngine* engine) {
    if (!engine) return;
    
    if (engine->running) {
        collection_engine_stop(engine);
    }
    
    if (engine->prev_cpu) {
        free(engine->prev_cpu);
        engine->prev_cpu = NULL;
    }
    if (engine->prev_net) {
        free(engine->prev_net);
        engine->prev_net = NULL;
    }
}
