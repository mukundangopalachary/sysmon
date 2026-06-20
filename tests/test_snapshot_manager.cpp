#include "test_utils.h"

extern "C" {
#include "snapshot_manager.h"
}

TEST(test_snapshot_lifecycle) {
    SnapshotManager mgr;
    int ret = snapshot_manager_init(&mgr);
    EXPECT_EQ(0, ret);

    // Initial state: current is NULL because sequence is 0
    const SystemSnapshot* curr = snapshot_manager_get_current(&mgr);
    EXPECT_TRUE(curr == NULL);

    SystemSnapshot* next = snapshot_manager_get_next(&mgr);
    EXPECT_TRUE(next != NULL);
    EXPECT_TRUE(curr != next);

    // Write to next and publish
    next->collection_timestamp_us = 1000;
    next->collection_sequence = 1;
    snapshot_manager_publish(&mgr);

    // Now current should be the one we just published
    const SystemSnapshot* new_curr = snapshot_manager_get_current(&mgr);
    EXPECT_EQ(1000, new_curr->collection_timestamp_us);
    EXPECT_TRUE(new_curr == next); // The pointers swapped

    // Next should now be the other buffer
    SystemSnapshot* new_next = snapshot_manager_get_next(&mgr);
    EXPECT_TRUE(new_next != NULL);
    EXPECT_TRUE(new_curr != new_next);

    snapshot_manager_destroy(&mgr);
}

int main() {
    RUN_TEST(test_snapshot_lifecycle);
    return 0;
}
