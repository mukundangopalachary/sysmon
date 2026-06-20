#ifndef SYSMON_INPUT_HANDLER_H
#define SYSMON_INPUT_HANDLER_H

#include "config_parser.h"

class ScreenManager;

enum class InputResult {
    HANDLED,
    NOT_HANDLED,
    QUIT
};

class InputHandler {
public:
    void init(SysmonConfig* cfg, ScreenManager* screen_mgr);
    InputResult handle(int key);
private:
    SysmonConfig* cfg_ = nullptr;
    ScreenManager* screen_mgr_ = nullptr;
    
    int parse_key(const char* key_str);
};
#endif
