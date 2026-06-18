#include "collection_engine.h"
#include <string.h>

int collection_engine_init(CollectionEngine* engine, SnapshotManager* mgr) {
    (void)engine;
    (void)mgr;
    memset(engine, 0, sizeof(*engine));
    engine->snapshot_mgr = mgr;
    engine->collection_interval_ms = 1000;
    engine->process_max_count = 10000;
    engine->collect_disk_io = true;
    return 0;
}

int collection_engine_start(CollectionEngine* engine) {
    (void)engine;
    return 0;
}

int collection_engine_stop(CollectionEngine* engine) {
    (void)engine;
    engine->running = false;
    return 0;
}

void collection_engine_destroy(CollectionEngine* engine) {
    (void)engine;
}
