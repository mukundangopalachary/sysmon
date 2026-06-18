#include "snapshot_manager.h"
#include <string.h>
#include <stdlib.h>

int snapshot_manager_init(SnapshotManager* mgr) {
    (void)mgr;
    memset(mgr, 0, sizeof(*mgr));
    return 0;
}

void snapshot_manager_destroy(SnapshotManager* mgr) {
    (void)mgr;
}

const SystemSnapshot* snapshot_manager_get_current(SnapshotManager* mgr) {
    (void)mgr;
    return NULL;
}

SystemSnapshot* snapshot_manager_get_next(SnapshotManager* mgr) {
    (void)mgr;
    return NULL;
}

void snapshot_manager_publish(SnapshotManager* mgr) {
    (void)mgr;
}
