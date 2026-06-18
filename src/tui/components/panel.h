#ifndef SYSMON_PANEL_H
#define SYSMON_PANEL_H
#include <ncurses.h>
#include "sysmon_bridge.h"

class Panel {
public:
    Panel(int height, int width, int start_y, int start_x);
    virtual ~Panel();
    virtual void render(const SystemSnapshot* snapshot) = 0;
    virtual void on_focus();
    virtual void on_blur();
    virtual void on_resize(int height, int width, int start_y, int start_x);
    virtual bool handle_input(int key);
    void draw_border(bool focused = false);
    void draw_title(const char* title);
    void clear_content();
protected:
    WINDOW* window;
    int height_, width_, start_y_, start_x_;
    bool has_focus_;
};
#endif
