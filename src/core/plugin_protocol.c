#include "plugin_protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool plugin_protocol_execute(const char* executable_path, const char* command, char** output) {
    char cmd_line[1024];
    snprintf(cmd_line, sizeof(cmd_line), "%s %s 2>&1", executable_path, command);

    FILE* fp = popen(cmd_line, "r");
    if (!fp) return false;

    size_t size = 4096;
    size_t len = 0;
    char* buf = malloc(size);
    if (!buf) {
        pclose(fp);
        return false;
    }

    char temp[1024];
    while (fgets(temp, sizeof(temp), fp) != NULL) {
        size_t tlen = strlen(temp);
        if (len + tlen + 1 > size) {
            size *= 2;
            char* new_buf = realloc(buf, size);
            if (!new_buf) {
                free(buf);
                pclose(fp);
                return false;
            }
            buf = new_buf;
        }
        strcpy(buf + len, temp);
        len += tlen;
    }

    buf[len] = '\0';
    int status = pclose(fp);

    if (output) {
        *output = buf;
    } else {
        free(buf);
    }

    // Return true if the plugin exited with 0
    return (WEXITSTATUS(status) == 0);
}
