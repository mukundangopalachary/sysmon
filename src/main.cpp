#include "app.h"
#include "sysmon_bridge.h"
#include "collection_engine.h"
#include "snapshot_manager.h"

int main(int argc, char** argv) {
    SnapshotManager snap_mgr;
    snapshot_manager_init(&snap_mgr);

    CollectionEngine engine;
    collection_engine_init(&engine, &snap_mgr);
    collection_engine_start(&engine);

    Application app(&snap_mgr);
    int result = app.run(argc, argv);

    collection_engine_stop(&engine);
    collection_engine_destroy(&engine);
    snapshot_manager_destroy(&snap_mgr);

    return result;
}
