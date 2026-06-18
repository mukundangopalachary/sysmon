#ifndef SYSMON_PROC_READER_H
#define SYSMON_PROC_READER_H

#include <stddef.h>
#include <stdint.h>

int read_file_to_buf(const char* path, char* buf, size_t bufsize);
uint64_t parse_uint64(const char* str);

#endif /* SYSMON_PROC_READER_H */
