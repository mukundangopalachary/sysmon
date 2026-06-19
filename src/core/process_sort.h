#ifndef SYSMON_PROCESS_SORT_H
#define SYSMON_PROCESS_SORT_H

#include "types.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SORT_COL_PID,
    SORT_COL_USER,
    SORT_COL_CPU,
    SORT_COL_MEM,
    SORT_COL_STATE,
    SORT_COL_TIME,
    SORT_COL_CMD
} SortColumn;

typedef enum {
    SORT_ORDER_ASC,
    SORT_ORDER_DESC
} SortOrder;

// Sorts the given process table in place
void process_sort_table(ProcessTableSnapshot* table, SortColumn col, SortOrder order);

// Returns true if the process matches the filter string (case-insensitive on comm or user)
bool process_matches_filter(const ProcessSnapshot* process, const char* filter);

#ifdef __cplusplus
}
#endif

#endif // SYSMON_PROCESS_SORT_H
