#ifndef SYSMON_SCREEN_H
#define SYSMON_SCREEN_H
#include <vector>
#include <memory>
#include "components/panel.h"

class Screen {
public:
    virtual ~Screen();
    virtual void on_enter();
    virtual void on_exit();
    virtual void render(const SystemSnapshot* snapshot) = 0;
    virtual bool handle_input(int key) = 0;
    virtual void on_resize();
    void set_visible(bool visible);
    bool is_visible() const;
protected:
    std::vector<std::unique_ptr<Panel>> panels_;
    bool visible_ = false;
};
#endif
