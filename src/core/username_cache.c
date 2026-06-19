#include "username_cache.h"
#include <pwd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define CACHE_SIZE 1024
#define TTL_SECONDS 300

typedef struct {
    uid_t uid;
    char username[32];
    time_t timestamp;
    int valid;
} CacheEntry;

static CacheEntry* user_cache = NULL;

int username_cache_init(void) {
    if (user_cache != NULL) return 0;
    user_cache = (CacheEntry*)calloc(CACHE_SIZE, sizeof(CacheEntry));
    if (!user_cache) return -1;
    return 0;
}

const char* username_cache_lookup(uid_t uid) {
    if (!user_cache) return NULL;
    
    time_t now = time(NULL);
    size_t idx = uid % CACHE_SIZE;
    size_t start_idx = idx;
    size_t empty_idx = CACHE_SIZE;
    
    do {
        if (user_cache[idx].valid && user_cache[idx].uid == uid) {
            if (now - user_cache[idx].timestamp < TTL_SECONDS) {
                return user_cache[idx].username;
            } else {
                empty_idx = idx;
                break;
            }
        }
        
        if (!user_cache[idx].valid && empty_idx == CACHE_SIZE) {
            empty_idx = idx;
        }
        
        idx = (idx + 1) % CACHE_SIZE;
    } while (idx != start_idx);
    
    struct passwd* pw = getpwuid(uid);
    if (!pw) {
        return NULL;
    }
    
    if (empty_idx == CACHE_SIZE) {
        empty_idx = start_idx;
    }
    
    user_cache[empty_idx].uid = uid;
    strncpy(user_cache[empty_idx].username, pw->pw_name, sizeof(user_cache[empty_idx].username) - 1);
    user_cache[empty_idx].username[sizeof(user_cache[empty_idx].username) - 1] = '\0';
    user_cache[empty_idx].timestamp = now;
    user_cache[empty_idx].valid = 1;
    
    return user_cache[empty_idx].username;
}

void username_cache_destroy(void) {
    if (user_cache) {
        free(user_cache);
        user_cache = NULL;
    }
}
