#include "mem_stats.h"
#include <string.h>

int mem_stats_read(MemorySnapshot* snap) {
    (void)snap;
    memset(snap, 0, sizeof(*snap));
    return 0;
}
