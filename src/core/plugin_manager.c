#include "plugin_manager.h"
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

int plugin_manager_init(PluginManager* mgr, const char* plugin_dir) {
    memset(mgr, 0, sizeof(*mgr));
    DIR* dir = opendir(plugin_dir);
    if (!dir) return -1;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".sh") && mgr->num_plugins < 32) {
            PluginInstance* p = &mgr->plugins[mgr->num_plugins];
            snprintf(p->name, sizeof(p->name), "%.63s", entry->d_name);
            snprintf(p->path, sizeof(p->path), "%s/%s", plugin_dir, entry->d_name);
            mgr->num_plugins++;
        }
    }
    closedir(dir);
    return 0;
}

int plugin_manager_load_plugins(PluginManager* mgr) {
    (void)mgr;
    return 0;
}

int plugin_manager_start_all(PluginManager* mgr) {
    for (int i = 0; i < mgr->num_plugins; i++) {
        PluginInstance* p = &mgr->plugins[i];
        int fd[2];
        if (pipe(fd) == -1) continue;
        pid_t pid = fork();
        if (pid == 0) {
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
            char* args[] = {p->path, NULL};
            execvp(p->path, args);
            exit(1);
        } else if (pid > 0) {
            close(fd[1]);
            p->stdout_fd = fd[0];
            p->pid = pid;
            int flags = fcntl(p->stdout_fd, F_GETFL, 0);
            fcntl(p->stdout_fd, F_SETFL, flags | O_NONBLOCK);
        } else {
            close(fd[0]);
            close(fd[1]);
        }
    }
    return 0;
}

int plugin_manager_stop_all(PluginManager* mgr) {
    for (int i = 0; i < mgr->num_plugins; i++) {
        PluginInstance* p = &mgr->plugins[i];
        if (p->pid > 0) {
            kill(p->pid, SIGTERM);
            close(p->stdout_fd);
        }
    }
    return 0;
}

int plugin_manager_collect(PluginManager* mgr, PluginData* out_data) {
    out_data->num_metrics = 0;
    char buf[1024];
    for (int i = 0; i < mgr->num_plugins; i++) {
        PluginInstance* p = &mgr->plugins[i];
        if (p->pid <= 0) continue;
        int n = read(p->stdout_fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            char* line = strtok(buf, "\n");
            while (line && out_data->num_metrics < 16) {
                strncpy(out_data->metrics[out_data->num_metrics], line, 127);
                out_data->metrics[out_data->num_metrics][127] = '\0';
                out_data->num_metrics++;
                line = strtok(NULL, "\n");
            }
        }
    }
    return 0;
}
