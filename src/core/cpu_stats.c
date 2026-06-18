#include "cpu_stats.h"
#include <string.h>

int cpu_stats_read(CpuSnapshot* snap) {
    (void)snap;
    memset(snap, 0, sizeof(*snap));
    return 0;
}
