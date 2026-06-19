#ifndef SYSMON_CLI_MAIN_H
#define SYSMON_CLI_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * Main entry point for the CLI mode.
 * Returns the exit code to be returned to the OS.
 */
int cli_main(int argc, char** argv);

#ifdef __cplusplus
}
#endif

#endif // SYSMON_CLI_MAIN_H
