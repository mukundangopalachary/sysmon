#include "proc_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int read_file_to_buf(const char* path, char* buf, size_t bufsize) {
    if (!path || !buf || bufsize == 0) {
        return -1;
    }
    
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    
    ssize_t bytes_read = read(fd, buf, bufsize - 1);
    close(fd);
    
    if (bytes_read < 0) {
        return -1;
    }
    
    buf[bytes_read] = '\0';
    return (int)bytes_read;
}

uint64_t parse_uint64(const char* str) {
    if (!str) {
        return 0;
    }
    return strtoull(str, NULL, 10);
}
