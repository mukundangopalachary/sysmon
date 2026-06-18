#include "proc_reader.h"

int read_file_to_buf(const char* path, char* buf, size_t bufsize) {
    (void)path;
    if (buf && bufsize > 0) {
        buf[0] = '\0';
    }
    return 0;
}

uint64_t parse_uint64(const char* str) {
    (void)str;
    return 0;
}
