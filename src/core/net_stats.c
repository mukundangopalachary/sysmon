#include "net_stats.h"
#include <string.h>

int net_stats_read(NetworkSnapshot* snap) {
    (void)snap;
    memset(snap, 0, sizeof(*snap));
    return 0;
}
