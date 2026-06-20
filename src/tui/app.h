#ifndef SYSMON_APP_H
#define SYSMON_APP_H

#include <string>
#include <memory>
#include "screens/screen.h"
#include "event_loop.h"
#include "theme.h"
#include "input_handler.h"
#include "screen_manager.h"

extern "C" {
#include "../core/snapshot_manager.h"
#include "config_parser.h"
#include "config_paths.h"
}

#include "theme.h"
#include <string>

class Application {
public:
    Application(SnapshotManager* snap_mgr, CollectionEngine* engine);
    int run(int argc, char** argv);

private:
    void register_screen(const std::string& name, std::unique_ptr<Screen> screen);
    void switch_screen(const std::string& name);

    SnapshotManager* snap_mgr_;
    CollectionEngine* engine_;
    SysmonConfig cfg_;
    ScreenManager screen_mgr_;
    InputHandler input_handler_;
    ThemeManager theme_mgr_;
    EventLoop event_loop_;
};
#endif
