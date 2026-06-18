#ifndef SYSMON_SNAPSHOT_MANAGER_H
#define SYSMON_SNAPSHOT_MANAGER_H

#include "types.h"

/*
 * SNAPSHOT MANAGER
 * 
 * Manages the double-buffer and atomic swapping.
 * 
 * Memory strategy:
 * - Two SystemSnapshot structs allocated at init
 * - Process arrays grow to max seen process count and stay there
 * - Connection arrays grow on demand
 * - SystemInfo allocated once, shared between both buffers
 * - Memory is never freed during operation (only at shutdown)
 * 
 * Thread safety:
 * - publish() uses atomic exchange
 * - get_current() is lock-free read
 * - Only collection thread modifies the "next" buffer
 */
typedef struct SnapshotManager SnapshotManager;

struct SnapshotManager {
    SystemSnapshot* buffers[2];     /* Double buffer */
    int current_index;              /* 0 or 1, which buffer is "current" */
    SystemInfo* shared_system_info; /* Shared between both buffers */
    
    /* Statistics */
    uint64_t total_swaps;
    uint64_t total_collections;
    uint64_t max_process_count;
    uint64_t max_connection_count;
    uint64_t max_collection_time_us;
};

/* Public API */
int snapshot_manager_init(SnapshotManager* mgr);
void snapshot_manager_destroy(SnapshotManager* mgr);

/*
 * Get current immutable snapshot for reading.
 * Returns NULL if no snapshot has been published yet.
 * Thread-safe: can be called from any thread without locks.
 */
const SystemSnapshot* snapshot_manager_get_current(SnapshotManager* mgr);

/*
 * Get the "next" buffer for the collection engine to fill.
 * Only call from collection thread.
 * Returns mutable pointer - this buffer is NOT visible to readers.
 */
SystemSnapshot* snapshot_manager_get_next(SnapshotManager* mgr);

/*
 * Publish the completed "next" buffer.
 * Atomic swap makes it the new "current".
 * Only call from collection thread.
 */
void snapshot_manager_publish(SnapshotManager* mgr);

#endif /* SYSMON_SNAPSHOT_MANAGER_H */
