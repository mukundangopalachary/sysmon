#include "components/header.h"
#include "theme.h"
#include <unistd.h>
#include <cstring>
HeaderPanel::HeaderPanel(int height, int width, int start_y, int start_x)
    : Panel(height, width, start_y, start_x) {}

HeaderPanel::~HeaderPanel() {}

void HeaderPanel::render(const SystemSnapshot* snapshot) {
    if (!window || !snapshot || !snapshot->system_info) return;
    
    clear_content();
    
    draw_border(false);
    
    // Check if we have enough height to draw the ASCII art
    if (height_ >= 6) {
        const char* art[] = {
            "   ______  ________  _______  _  __ ",
            "  / __/\\ \\/ / __/  |/  / __ \\/ |/ / ",
            " _\\ \\   \\  /\\ \\/ /|_/ / /_/ /    /  ",
            "/___/   /_/___/_/  /_/\\____/_/|_/   "
        };
        
        int art_width = 36; // Approx width of the art
        int start_x = (width_ - art_width) / 2;
        if (start_x < 1) start_x = 1;
        
        wattron(window, A_BOLD | COLOR_PAIR(THEME_HEADER));
        for (int i = 0; i < 4; i++) {
            mvwprintw(window, 1 + i, start_x, "%s", art[i]);
        }
        wattroff(window, A_BOLD | COLOR_PAIR(THEME_HEADER));
        
        // Draw the stats underneath the art
        mvwprintw(window, 5, 2, "Up: %s", format_duration_us(snapshot->collection_timestamp_us));
        mvwprintw(window, 5, 20, "Procs: %d", snapshot->processes.num_processes);
        mvwprintw(window, 5, 35, "Load: %.2f %.2f %.2f", 
                  snapshot->cpu.load_avg_1min, 
                  snapshot->cpu.load_avg_5min, 
                  snapshot->cpu.load_avg_15min);
                  
        char hostname[256] = "sysmon";
        gethostname(hostname, sizeof(hostname));
        mvwprintw(window, 5, width_ - strlen(hostname) - 10, "Host: %s", hostname);
    } else {
        // Fallback to normal 1-line header if terminal is small or height is low
        draw_title("Sysmon v2.0.1");
        mvwprintw(window, 1, 2, "Up: %s", format_duration_us(snapshot->collection_timestamp_us));
        mvwprintw(window, 1, 20, "Procs: %d", snapshot->processes.num_processes);
        mvwprintw(window, 1, 35, "Load: %.2f %.2f %.2f", 
                  snapshot->cpu.load_avg_1min, 
                  snapshot->cpu.load_avg_5min, 
                  snapshot->cpu.load_avg_15min);
                  
        char hostname[256] = "sysmon";
        gethostname(hostname, sizeof(hostname));
        mvwprintw(window, 1, width_ - strlen(hostname) - 10, "Host: %s", hostname);
    }
    
    wnoutrefresh(window); 
}

bool HeaderPanel::handle_input(int /*key*/) { return false; }
