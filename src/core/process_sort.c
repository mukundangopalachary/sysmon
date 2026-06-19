#include "process_sort.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static SortColumn current_sort_col;
static SortOrder current_sort_order;

static int compare_processes(const void* a, const void* b) {
    const ProcessSnapshot* pa = (const ProcessSnapshot*)a;
    const ProcessSnapshot* pb = (const ProcessSnapshot*)b;
    
    int result = 0;
    
    switch (current_sort_col) {
        case SORT_COL_PID:
            result = (pa->pid < pb->pid) ? -1 : ((pa->pid > pb->pid) ? 1 : 0);
            break;
        case SORT_COL_USER:
            result = strcmp(pa->username, pb->username);
            break;
        case SORT_COL_CPU:
            if (pa->cpu_percent < pb->cpu_percent) result = -1;
            else if (pa->cpu_percent > pb->cpu_percent) result = 1;
            break;
        case SORT_COL_MEM:
            if (pa->rss_bytes < pb->rss_bytes) result = -1;
            else if (pa->rss_bytes > pb->rss_bytes) result = 1;
            break;
        case SORT_COL_STATE:
            if (pa->state < pb->state) result = -1;
            else if (pa->state > pb->state) result = 1;
            break;
        case SORT_COL_TIME: {
            uint64_t time_a = pa->utime_ticks + pa->stime_ticks;
            uint64_t time_b = pb->utime_ticks + pb->stime_ticks;
            if (time_a < time_b) result = -1;
            else if (time_a > time_b) result = 1;
            break;
        }
        case SORT_COL_CMD:
            result = strcmp(pa->comm, pb->comm);
            break;
    }
    
    if (result == 0) {
        // Fallback to PID ascending for stability
        result = (pa->pid < pb->pid) ? -1 : ((pa->pid > pb->pid) ? 1 : 0);
        return result; 
    }
    
    return (current_sort_order == SORT_ORDER_ASC) ? result : -result;
}

void process_sort_table(ProcessTableSnapshot* table, SortColumn col, SortOrder order) {
    if (!table || table->num_processes == 0) return;
    
    current_sort_col = col;
    current_sort_order = order;
    
    qsort(table->processes, table->num_processes, sizeof(ProcessSnapshot), compare_processes);
}

// Helper for case-insensitive substring search
static const char* stristr(const char* haystack, const char* needle) {
    if (!*needle) return haystack;
    for (; *haystack; ++haystack) {
        if (toupper(*haystack) == toupper(*needle)) {
            const char *h, *n;
            for (h = haystack, n = needle; *h && *n; ++h, ++n) {
                if (toupper(*h) != toupper(*n)) break;
            }
            if (!*n) return haystack;
        }
    }
    return NULL;
}

bool process_matches_filter(const ProcessSnapshot* process, const char* filter) {
    if (!filter || filter[0] == '\0') return true;
    
    if (stristr(process->comm, filter) != NULL) return true;
    if (stristr(process->username, filter) != NULL) return true;
    
    return false;
}
