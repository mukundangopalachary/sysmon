#ifndef SYSMON_PLUGIN_PROTOCOL_H
#define SYSMON_PLUGIN_PROTOCOL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Execute a plugin executable with arguments (e.g. "collect", "check")
// Returns true on success, fills output buffer. output must be freed by caller.
bool plugin_protocol_execute(const char* executable_path, const char* command, char** output);

#ifdef __cplusplus
}
#endif

#endif // SYSMON_PLUGIN_PROTOCOL_H
