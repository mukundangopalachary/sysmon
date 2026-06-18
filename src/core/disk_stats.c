#include "disk_stats.h"
#include <string.h>

int disk_stats_read(DiskSnapshot* snap) {
    (void)snap;
    memset(snap, 0, sizeof(*snap));
    return 0;
}
