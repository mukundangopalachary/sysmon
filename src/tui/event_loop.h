#ifndef SYSMON_EVENT_LOOP_H
#define SYSMON_EVENT_LOOP_H

#include "screen_manager.h"
#include "input_handler.h"

extern "C" {
#include "../core/snapshot_manager.h"
}

class EventLoop {
public:
    EventLoop(ScreenManager* sm, InputHandler* ih) : screen_mgr_(sm), input_handler_(ih), running_(false) {}
    void run(SnapshotManager* snap_mgr);
    void stop();

private:
    ScreenManager* screen_mgr_;
    InputHandler* input_handler_;
    bool running_;
};
#endif
