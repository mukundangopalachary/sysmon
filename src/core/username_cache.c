#include "username_cache.h"
#include <stddef.h>

int username_cache_init(void) {
    return 0;
}

const char* username_cache_lookup(uid_t uid) {
    (void)uid;
    return NULL;
}

void username_cache_destroy(void) {
}
