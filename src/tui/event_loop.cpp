#include "event_loop.h"
#include <ncurses.h>
#include <unistd.h>

void EventLoop::run(SnapshotManager* snap_mgr) {
    running_ = true;
    int counter = 0;
    while (running_) {
        int ch = getch();
        if (ch == 'q') {
            running_ = false;
            break;
        }
        if (ch != ERR) {
            bool handled = screen_mgr_->handle_input(ch);
            if (!handled) {
                InputResult result = input_handler_->handle(ch);
                if (result == InputResult::QUIT) {
                    running_ = false;
                    handled = true;
                } else if (result == InputResult::HANDLED) {
                    handled = true;
                }
                
                // Fallback hardcoded quit just in case
                if (!handled && (ch == 'q' || ch == 'Q')) {
                    running_ = false;
                    handled = true;
                }
            }
            // If we processed a key, force an immediate redraw
            if (handled) {
                counter = 1000;
            }
        }

        counter += 10;
        if (counter >= 1000) {
            const SystemSnapshot* snap = snapshot_manager_get_current(snap_mgr);
            if (snap != nullptr) {
                screen_mgr_->render(snap);
            }
            counter = 0;
        }
        usleep(10000); // 10ms sleep
    }
}

void EventLoop::stop() {
    running_ = false;
}
