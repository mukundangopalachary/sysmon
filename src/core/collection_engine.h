#ifndef SYSMON_COLLECTION_ENGINE_H
#define SYSMON_COLLECTION_ENGINE_H

#include <pthread.h>
#include "types.h"
#include "snapshot_manager.h"
#include "plugin_manager.h"

/*
 * COLLECTION ENGINE
 * 
 * Single-threaded, runs in a loop with configurable interval.
 * Each iteration produces one complete SystemSnapshot.
 * 
 * Collection order matters:
 * 1. Read all /proc files (minimize time between first and last read)
 * 2. Parse and compute derived values
 * 3. Publish snapshot atomically
 * 
 * Error handling:
 * - Missing /proc entries -> zero-fill, log warning
 * - Permission denied -> skip, leave blank
 * - Malformed data -> skip entry, log error
 * - /proc not mounted -> fatal, exit with message
 */
typedef struct CollectionEngine CollectionEngine;

struct CollectionEngine {
    /* Configuration */
    int collection_interval_ms;     /* Default: 1000 */
    int process_max_count;          /* Default: 10000 */
    bool collect_connections;       /* Default: false (lazy) */
    bool collect_disk_io;           /* Default: true */
    
    /* State */
    bool running;
    pthread_t thread;
    SnapshotManager* snapshot_mgr;
    
    /* Previous snapshot for rate calculations */
    CpuCoreSnapshot* prev_cpu;      /* Per-core previous times */
    NetworkIfaceSnapshot* prev_net; /* Per-interface previous bytes */
    uint64_t prev_timestamp_us;
    
    /* Plugins */
    PluginManager plugin_mgr;
    PluginData plugin_data_buf;

    /* Callbacks */
    void (*on_collection_start)(void);
    void (*on_collection_complete)(SystemSnapshot* snapshot);
    void (*on_collection_error)(const char* error_msg);
};

/* Public API */
int collection_engine_init(CollectionEngine* engine, SnapshotManager* mgr);
int collection_engine_start(CollectionEngine* engine);
int collection_engine_stop(CollectionEngine* engine);
void collection_engine_destroy(CollectionEngine* engine);

#endif /* SYSMON_COLLECTION_ENGINE_H */
