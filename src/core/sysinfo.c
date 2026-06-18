#include "sysinfo.h"
#include <string.h>

int sysinfo_collect(SystemInfo* info) {
    (void)info;
    memset(info, 0, sizeof(*info));
    return 0;
}
