#ifndef SYSMON_THEME_H
#define SYSMON_THEME_H
#include <string>

// Global Color Pair IDs
enum ThemeColor {
    THEME_DEFAULT = 1,
    THEME_BORDER,
    THEME_HEADER,
    THEME_HIGHLIGHT,
    THEME_CRITICAL,
    THEME_WARN,
    THEME_GOOD
};

class ThemeManager {
public:
    void init();
    void set_theme(const std::string& name);
private:
    std::string current_theme_ = "default";
};
#endif
