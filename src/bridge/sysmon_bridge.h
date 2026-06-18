#ifndef SYSMON_BRIDGE_H
#define SYSMON_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/* Forward declare opaque structs for C++ consumers */
typedef struct SnapshotManager SnapshotManager;
typedef struct CollectionEngine CollectionEngine;

/* Snapshot Manager API */
int snapshot_manager_init(SnapshotManager* mgr);
void snapshot_manager_destroy(SnapshotManager* mgr);
const SystemSnapshot* snapshot_manager_get_current(SnapshotManager* mgr);
SystemSnapshot* snapshot_manager_get_next(SnapshotManager* mgr);
void snapshot_manager_publish(SnapshotManager* mgr);

/* Collection Engine API */
int collection_engine_init(CollectionEngine* engine, SnapshotManager* mgr);
int collection_engine_start(CollectionEngine* engine);
int collection_engine_stop(CollectionEngine* engine);
void collection_engine_destroy(CollectionEngine* engine);

/* Process Table Helpers */
const ProcessSnapshot* process_find_by_pid(const SystemSnapshot* snap, int pid);
int process_get_children(const SystemSnapshot* snap, int ppid, const ProcessSnapshot** children, int max_children);

/* Utility */
const char* format_bytes(uint64_t bytes);
const char* format_duration_us(uint64_t us);
uint64_t get_time_us(void);

#ifdef __cplusplus
}
#endif

#endif /* SYSMON_BRIDGE_H */
