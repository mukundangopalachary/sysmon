#include "snapshot_manager.h"
#include <string.h>
#include <stdlib.h>

int snapshot_manager_init(SnapshotManager* mgr) {
    if (!mgr) return -1;
    memset(mgr, 0, sizeof(*mgr));

    mgr->buffers[0] = calloc(1, sizeof(SystemSnapshot));
    mgr->buffers[1] = calloc(1, sizeof(SystemSnapshot));
    if (!mgr->buffers[0] || !mgr->buffers[1]) return -1;

    mgr->shared_system_info = calloc(1, sizeof(SystemInfo));
    if (!mgr->shared_system_info) return -1;

    mgr->buffers[0]->system_info = mgr->shared_system_info;
    mgr->buffers[1]->system_info = mgr->shared_system_info;

    mgr->buffers[0]->processes.capacity = 10000;
    mgr->buffers[0]->processes.processes = calloc(10000, sizeof(ProcessSnapshot));
    
    mgr->buffers[1]->processes.capacity = 10000;
    mgr->buffers[1]->processes.processes = calloc(10000, sizeof(ProcessSnapshot));

    if (!mgr->buffers[0]->processes.processes || !mgr->buffers[1]->processes.processes) {
        return -1;
    }

    mgr->current_index = 0;
    return 0;
}

void snapshot_manager_destroy(SnapshotManager* mgr) {
    if (!mgr) return;

    if (mgr->buffers[0]) {
        if (mgr->buffers[0]->processes.processes) {
            free(mgr->buffers[0]->processes.processes);
        }
        free(mgr->buffers[0]);
    }
    
    if (mgr->buffers[1]) {
        if (mgr->buffers[1]->processes.processes) {
            free(mgr->buffers[1]->processes.processes);
        }
        free(mgr->buffers[1]);
    }
    
    if (mgr->shared_system_info) {
        free(mgr->shared_system_info);
    }
    
    memset(mgr, 0, sizeof(*mgr));
}

const SystemSnapshot* snapshot_manager_get_current(SnapshotManager* mgr) {
    if (!mgr) return NULL;
    int idx = __atomic_load_n(&mgr->current_index, __ATOMIC_ACQUIRE);
    SystemSnapshot* snap = mgr->buffers[idx];
    if (snap && snap->collection_sequence == 0) {
        return NULL;
    }
    return snap;
}

SystemSnapshot* snapshot_manager_get_next(SnapshotManager* mgr) {
    if (!mgr) return NULL;
    int idx = __atomic_load_n(&mgr->current_index, __ATOMIC_RELAXED);
    return mgr->buffers[1 - idx];
}

void snapshot_manager_publish(SnapshotManager* mgr) {
    if (!mgr) return;
    int idx = __atomic_load_n(&mgr->current_index, __ATOMIC_RELAXED);
    int next_idx = 1 - idx;
    __atomic_store_n(&mgr->current_index, next_idx, __ATOMIC_RELEASE);
    mgr->total_swaps++;
}
