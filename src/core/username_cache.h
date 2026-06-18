#ifndef SYSMON_USERNAME_CACHE_H
#define SYSMON_USERNAME_CACHE_H

#include <sys/types.h>

int username_cache_init(void);
const char* username_cache_lookup(uid_t uid);
void username_cache_destroy(void);

#endif /* SYSMON_USERNAME_CACHE_H */
