#ifndef SYSMON_APP_H
#define SYSMON_APP_H
#include <string>
#include <memory>
#include "screens/screen.h"

class Application {
public:
    int run(int argc, char** argv);
    void register_screen(const std::string& name, std::unique_ptr<Screen> screen);
    void switch_screen(const std::string& name);
};
#endif
