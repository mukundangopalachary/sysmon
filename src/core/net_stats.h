#ifndef SYSMON_NET_STATS_H
#define SYSMON_NET_STATS_H

#include "types.h"

int net_stats_read(NetworkSnapshot* snap);
int net_connections_read(ConnectionTableSnapshot* snap);

#endif /* SYSMON_NET_STATS_H */
