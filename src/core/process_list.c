#include "process_list.h"

int process_list_read(ProcessTableSnapshot* snap, int max_count) {
    (void)snap;
    (void)max_count;
    if (snap) {
        snap->num_processes = 0;
    }
    return 0;
}
