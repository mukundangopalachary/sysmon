#ifndef SYSMON_PROCESS_LIST_H
#define SYSMON_PROCESS_LIST_H

#include "types.h"

int process_list_read(ProcessTableSnapshot* snap, int max_count);

#endif /* SYSMON_PROCESS_LIST_H */
