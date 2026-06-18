#ifndef SYSMON_THEME_H
#define SYSMON_THEME_H
#include <string>

class ThemeManager {
public:
    void init();
    void set_theme(const std::string& name);
};
#endif
