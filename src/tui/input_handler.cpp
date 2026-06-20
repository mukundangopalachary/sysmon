#include "input_handler.h"
#include "screen_manager.h"
#include <ncurses.h>
#include <string.h>

void InputHandler::init(SysmonConfig* cfg, ScreenManager* screen_mgr) {
    cfg_ = cfg;
    screen_mgr_ = screen_mgr;
}

int InputHandler::parse_key(const char* key_str) {
    if (!key_str || !key_str[0]) return -1;
    if (strlen(key_str) == 1) return key_str[0];
    if (strcmp(key_str, "F1") == 0) return KEY_F(1);
    if (strcmp(key_str, "F2") == 0) return KEY_F(2);
    if (strcmp(key_str, "F3") == 0) return KEY_F(3);
    if (strcmp(key_str, "F4") == 0) return KEY_F(4);
    if (strcmp(key_str, "F5") == 0) return KEY_F(5);
    if (strcmp(key_str, "F12") == 0) return KEY_F(12);
    if (strcmp(key_str, "ESC") == 0) return 27;
    return -1;
}

InputResult InputHandler::handle(int key) {
    if (!cfg_ || !screen_mgr_) return InputResult::NOT_HANDLED;
    
    int k_quit = parse_key(cfg_->keybindings.quit);
    int k_dash = parse_key(cfg_->keybindings.dashboard);
    int k_proc = parse_key(cfg_->keybindings.process_list);
    int k_conn = parse_key(cfg_->keybindings.connections);
    int k_plug = parse_key(cfg_->keybindings.plugins);
    int k_help = parse_key(cfg_->keybindings.help);
    
    if (key == k_quit || key == 'q' || key == 'Q') {
        return InputResult::QUIT;
    }
    
    if (key == k_dash) { screen_mgr_->switch_screen("dashboard"); return InputResult::HANDLED; }
    if (key == k_proc) { screen_mgr_->switch_screen("process_list"); return InputResult::HANDLED; }
    if (key == k_conn) { screen_mgr_->switch_screen("connection"); return InputResult::HANDLED; }
    if (key == k_plug) { screen_mgr_->switch_screen("plugin_page"); return InputResult::HANDLED; }
    if (key == k_help) { screen_mgr_->switch_screen("help"); return InputResult::HANDLED; }
    
    return InputResult::NOT_HANDLED;
}
