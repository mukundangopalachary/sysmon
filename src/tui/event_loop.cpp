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
                if (ch == 'p') screen_mgr_->switch_screen("process_list");
                else if (ch == 'n') screen_mgr_->switch_screen("connection");
                else if (ch == KEY_F(4) || ch == 'L' || ch == 'l' || ch == 16) screen_mgr_->switch_screen("plugin_page");
                else if (ch == 'd' || ch == 27) screen_mgr_->switch_screen("dashboard"); // 27 = ESC
                else handled = input_handler_->handle(ch);
            }
            // If we processed a key, force an immediate redraw
            if (handled || ch == 'p' || ch == 'n' || ch == KEY_F(4) || ch == 'L' || ch == 'l' || ch == 16 || ch == 'd' || ch == 27) {
                counter = 100;
            }
        }

        counter += 10;
        if (counter >= 100) {
            const SystemSnapshot* snap = snapshot_manager_get_current(snap_mgr);
            if (snap != nullptr) {
                screen_mgr_->render(snap);
                refresh();
            }
            counter = 0;
        }
        usleep(10000); // 10ms sleep
    }
}

void EventLoop::stop() {
    running_ = false;
}
