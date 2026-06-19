================================================================================
SYSMON V2 - ARCHITECTURE DOCUMENTATION
================================================================================

VERSION:        2.0.0
BASED ON:       V1 project structure (13 dirs, 84 files)
STATUS:         Design Phase
LAST UPDATED:   2026-06-20

================================================================================
1. WHAT CHANGES FROM V1
================================================================================

V1 PROBLEM                          V2 SOLUTION
────────────────────────────────────────────────────────────────────────────
Plugins embedded in scripts/        Proper CLI plugin management
  plugins/ as .sh files               sysmon plugin install/uninstall/list

Config manual editing only          CLI-driven config + TOML format
                                      sysmon config set/get/edit

CPU pane shows only bars            Breakdown by type, top consumers
                                      inline, thermal context

Process table no sort/filter        Column sorting, regex/text filter,
                                      state/user filter modes

Connection table no sort/filter     Sort by state/port/process, filter
                                      by port number or state

Single plugin pane with             Dedicated Plugin Page (F4)
  limited info                       Each plugin gets full pane(s)

No plugin versioning                Manifests, version locks, updates

Configuration in default.conf       ~/.config/sysmon/config.toml
                                      XDG spec compliant

No CLI mode                         Dual binary: sysmon (TUI) + 
                                      sysmon-cli (management)

No plugin registry                  Local + remote registry with
                                      index.toml, search capability

================================================================================
2. DUAL BINARY ARCHITECTURE
================================================================================

sysmon is now TWO executables built from the same codebase:

┌─────────────────────────────────────────────────────────────────┐
│                         BUILD OUTPUTS                            │
├───────────────────┬─────────────────────────────────────────────┤
│ sysmon-tui        │ Interactive terminal monitor                │
│ (binary: sysmon)  │ - Dashboard, Processes, Connections,        │
│                   │   Plugins pages                             │
│                   │ - Real-time metrics collection               │
│                   │ - Keyboard-driven navigation                 │
│                   │ - Links to libsysmon_core + libsysmon_tui    │
├───────────────────┼─────────────────────────────────────────────┤
│ sysmon-cli        │ Command-line management tool                │
│ (binary: sysmon-cli)│ - Plugin install/uninstall/list/search    │
│                   │ - Config get/set/edit/reload                 │
│                   │ - Theme set/list/preview                     │
│                   │ - Registry sync                             │
│                   │ - Links to libsysmon_core only               │
└───────────────────┴─────────────────────────────────────────────┘

Both link against the same libsysmon_core (C).
sysmon-tui additionally links libsysmon_tui (C++).
sysmon-cli is pure C, no curses dependency.

Entry point:
  src/main.cpp -> detects argv[0] or first argument
    if TUI mode:   launches app.cpp event loop
    if CLI mode:   dispatches to cli/cli_main.c

Shell experience:
  $ sysmon                  # Launches TUI
  $ sysmon plugin install   # CLI mode (argument detected)

================================================================================
3. DIRECTORY ARCHITECTURE - WHAT EACH DIRECTORY MEANS
================================================================================

src/
├── core/          C library - no UI, no curses, pure data
│                  - Collection engine, parsers, snapshot manager
│                  - Plugin protocol, registry, config parsing
│                  - Compiles to libsysmon_core.a
│                  - Used by BOTH sysmon-tui AND sysmon-cli
│
├── tui/           C++ library - ncurses UI components
│                  - Screens, panels, event loop, input handling
│                  - Compiles to libsysmon_tui.a
│                  - Used ONLY by sysmon-tui
│
├── cli/           C module - CLI command implementations
│                  - plugin commands, config commands, theme commands
│                  - Compiles into sysmon-cli binary
│                  - Thin layer over libsysmon_core
│
├── bridge/        C/C++ interface layer
│                  - extern "C" wrappers for C++ -> C calls
│                  - Shared type definitions
│                  - Compiles into libsysmon_core
│
└── main.cpp       Entry point - mode detection and dispatch

scripts/
├── plugins/
│   ├── builtin/   Shipped plugins (installed to /usr/share/sysmon/)
│   │              Each is a directory with plugin.sh + manifest.toml
│   │              Installed system-wide, user enables them via CLI
│   │
│   └── registry/  Local index of available plugins
│                  index.toml downloaded from GitHub releases
│                  Updated via: sysmon-cli registry update
│
├── alerts/        User-defined alert scripts (unchanged from V1)
│
└── install.sh     System dependency installer

config/
└── default.conf   System-wide defaults
                   User overrides: ~/.config/sysmon/config.toml
                   Plugin state:    ~/.config/sysmon/plugins.toml

~/.local/share/sysmon/           (Created at runtime)
├── plugins/       User-installed plugins live here
│   ├── gpu-nvidia/
│   │   ├── plugin.sh
│   │   ├── manifest.toml
│   │   └── version.lock
│   └── docker/
│       └── ...
└── plugin-cache/  Downloaded tarballs (safe to delete)

================================================================================
4. PLUGIN SYSTEM V2 - COMPLETE LIFECYCLE
================================================================================

4.1 Plugin Locations (search order)

When sysmon looks for a plugin named "gpu-nvidia":

  Priority 1: ~/.local/share/sysmon/plugins/gpu-nvidia/
              User-installed, possibly newer version

  Priority 2: /usr/share/sysmon/plugins/builtin/gpu-nvidia/
              System-installed, shipped with package

  Priority 3: /usr/local/share/sysmon/plugins/builtin/gpu-nvidia/
              Alternative system location

  Priority 4: Remote registry (only for install/search, not runtime)
              https://raw.githubusercontent.com/sysmon-plugins/registry/main/index.toml

4.2 Plugin Manifest Format

Each plugin ships a manifest.toml that describes:

  [plugin]
  name, version, description, author, license, homepage

  [requirements]
  executables      - Required binaries (checked before running)
  kernel_modules   - Required kernel modules
  sysfs_paths      - Required sysfs entries

  [plugin.panes]   - What UI panes this plugin provides
  Each pane has: name, description, type (gauge/table/text), refresh_ms

  [plugin.alerts]  - Alert conditions user can enable
  Each alert has: name, description, default_threshold, unit

4.3 Plugin Protocol V2

Plugin executables receive a command as first argument:

  COMMAND         BEHAVIOR
  ─────────────────────────────────────────────────────────────
  metadata        Output full manifest as JSON to stdout
                  Cached by sysmon, only called on install/update

  panes           List pane names, one per line
                  Used to build plugin page layout

  collect         Collect ALL metrics for ALL panes at once
                  Output format uses PANE: markers for grouping

  pane <name>     Collect metrics for a specific pane only
                  Used when only one pane is visible

  check           Verify plugin works (check requirements)
                  Return 0 if OK, non-zero with error message

Output format for 'collect' and 'pane':

  KEY=VALUE:TYPE:UNIT
  TABLE_START
  COLUMNS:col1,col2,col3
  row1val1,row1val2,row1val3
  row2val1,row2val2,row2val3
  TABLE_END

  PANE:pane_name
  (metrics for that pane)
  ENDPANE

4.4 CLI Plugin Commands

  sysmon plugin list              Show installed (name, version, enabled, desc)
  sysmon plugin search <query>    Search local + remote registry
  sysmon plugin info <name>       Show manifest details + config
  sysmon plugin install <name>    Download from registry, place in ~/.local/...
  sysmon plugin uninstall <name>  Remove from ~/.local/..., confirm prompt
  sysmon plugin enable <name>     Add to ~/.config/sysmon/plugins.toml
  sysmon plugin disable <name>    Remove from enabled list, keep files
  sysmon plugin update <name>     Check registry, download newer version
  sysmon plugin update --all      Update all installed plugins
  sysmon plugin check <name>      Run plugin's 'check' command, report status

4.5 Plugin States

  NOT INSTALLED   Not in any local directory
  INSTALLED       Present in ~/.local/... or /usr/...
  ENABLED         Listed in plugins.toml, actively runs
  DISABLED        Installed but not in plugins.toml, dormant
  BROKEN          check command failed (missing dependency)

4.6 Plugin Page (F4 in TUI)

  ┌─────────────────────────────────────────────────────────────┐
  │ PLUGINS                                   Page 1/2          │
  ├─────────────────────────────────────────────────────────────┤
  │                                                             │
  │ ┌─ gpu-nvidia v1.2.0 [ENABLED] ──────────────────────────┐ │
  │ │ PANE: GPU Overview                           [auto: 1s] │ │
  │ │ Temperature:  67°C  [████████░░░░░░░░] 67% of max       │ │
  │ │ Utilization:  82%   [████████████░░░░]                  │ │
  │ │ Power:        185W / 250W limit                         │ │
  │ │ Fan Speed:    45%                                       │ │
  │ ├─────────────────────────────────────────────────────────┤ │
  │ │ PANE: GPU Memory                            [auto: 2s] │ │
  │ │ VRAM:  8.0 GB / 12.0 GB  [████████████░░░░] 66%        │ │
  │ ├─────────────────────────────────────────────────────────┤ │
  │ │ PANE: GPU Processes                         [auto: 5s] │ │
  │ │ PID     Process              VRAM    GPU%               │ │
  │ │ 3241    python3 (training)   6.0 GB  95%                │ │
  │ │ 5678    firefox              1.0 GB  12%                │ │
  │ └─────────────────────────────────────────────────────────┘ │
  │                                                             │
  │ ┌─ docker v0.9.0 [ENABLED] ──────────────────────────────┐ │
  │ │ PANE: Containers                           [auto: 2s]  │ │
  │ │ ...                                                      │ │
  │ └─────────────────────────────────────────────────────────┘ │
  │                                                             │
  │ F1 Dash  F2 Proc  F3 Conn  F4 Plug  F5 Help                │
  │ e/d Toggle plugin    r Refresh plugin    c Configure plugin │
  └─────────────────────────────────────────────────────────────┘

  Plugin page scrolls vertically. Each plugin gets its own section.
  Plugin sections are collapsible (toggle with Enter).
  Each pane within a plugin refreshes at its own interval.
  'e' enables/disables the focused plugin.
  'c' opens plugin-specific config in $EDITOR.

================================================================================
5. ENHANCED CPU PANE
================================================================================

5.1 What's Wrong in V1

V1 CPU pane shows only:
  - Overall percentage bar
  - Per-core percentage bars

This answers "how busy?" but NOT:
  - Busy doing WHAT? (user code? kernel? waiting for I/O?)
  - WHO is causing it? (which processes?)
  - Is the system healthy? (thermal? load relative to cores?)

5.2 V2 CPU Pane Layout

  ┌─────────────────────────────────────────────────────────────┐
  │ CPU                                          [hist] [cores] │
  ├─────────────────────────────────────────────────────────────┤
  │                                                             │
  │  OVERALL: [████████░░░░░░░░░░░░░░░░░░░░░░░░] 42%            │
  │                                                             │
  │  ▸ User:    [████████░░░░░░░░░░] 28%    System: [████░░] 12%│
  │    IOWait:  [█░░░░░░░░░░░░░░░░░]  2%    Steal:  [░░░░░]  0%│
  │                                                             │
  │  ▸ TOP CPU CONSUMERS:                                       │
  │    firefox        [████████████░░░░] 56.1%  T:4  PID:1289   │
  │    python3        [██████░░░░░░░░░░] 22.3%  T:1  PID:3241   │
  │    node           [██░░░░░░░░░░░░░░]  8.7%  T:8  PID:892    │
  │                                                             │
  │  ▸ LOAD:  1m:2.1  5m:1.8  15m:1.5   (cores: 16)            │
  │    CTX SW: 42,318/s    INTERRUPTS: 18,492/s                 │
  │                                                             │
  │  ▸ THERMAL: Package 67°C / max 100°C  [███████░░░] 67%     │
  │                                                             │
  │  (press 'c' for per-core view, 'h' for history graph)       │
  └─────────────────────────────────────────────────────────────┘

5.3 Data Sources for Enhanced CPU

  METRIC              SOURCE              FREQUENCY
  ────────────────────────────────────────────────────────────
  User/System/IOWait  /proc/stat          Every collection
  Top consumers       Process table       Already collected
  Load averages       /proc/loadavg       Every collection
  Context switches    /proc/stat (ctxt)   Every collection
  Interrupts          /proc/stat (intr)   Every collection
  Thermal             Plugin or /sys       Every 2s (plugin)

5.4 Thermal Data Strategy

  V2 does NOT read thermal directly from /sys in core.
  Thermal is provided by the 'temperature' plugin (builtin).
  If plugin is enabled, CPU pane shows thermal section.
  If not, thermal section is hidden.
  This keeps core lean and makes thermal optional.

================================================================================
6. SORTING AND FILTERING
================================================================================

6.1 Generic Table Component

  src/tui/components/table_view.cpp provides:

  - Column definition (name, width, alignment, sortable)
  - Sort state (column index, ascending/descending)
  - Filter state (text filter, regex mode)
  - Scroll state (offset, page size)
  - Render with headers, sort indicators, filter bar
  - Keyboard handlers for sort/filter/scroll

  Used by:
  - Process Table (process_table.cpp)
  - Connection Table (connection_table.cpp)
  - Plugin tables (plugin_pane.cpp, for TABLE type panes)

6.2 Sort/Filter Component

  src/tui/components/sort_filter.cpp provides:

  sort_state_t:
    int sort_column
    bool descending
    int (*compare)(const void*, const void*, int column)

  filter_state_t:
    char filter_text[256]
    int filter_mode    // FILTER_TEXT, FILTER_REGEX, FILTER_STATE, FILTER_USER
    bool (*match)(const void*, const char* filter, int mode)

  filter_modes:
    TEXT     - Case-insensitive substring match on any visible column
    REGEX    - POSIX extended regex match
    STATE    - For connections: ESTABLISHED, LISTEN, TIME_WAIT etc.
    USER     - For processes: filter by username
    PID      - Exact PID match

6.3 Process Table Sort/Filter UX

  Keys:
    c       Sort by CPU%
    m       Sort by Memory%
    p       Sort by PID
    u       Sort by User
    s       Sort by State
    R       Reverse current sort
    /       Enter filter text
    Esc     Clear filter
    u       (in filter mode) Filter by user
    s       (in filter mode) Filter by process state

  Filter bar appearance:
    ┌─────────────────────────────────────────────────────────┐
    │ PROCESSES   Sort: CPU% ▼ | Filter: fire*_ [x] clear   │
    │             [c]pu [m]em [p]id [u]ser [s]tate [/]regex │
    ├─────────────────────────────────────────────────────────┤

6.4 Connection Table Sort/Filter UX

  Keys:
    s       Sort by State
    p       Sort by Port (local)
    a       Sort by Application/Process name
    i       Sort by Remote IP
    /       Enter filter (port number or state)
    p       (in filter mode) Filter by port number
    s       (in filter mode) Filter by state (ESTAB, LISTEN, etc.)

  Filter bar:
    ┌─────────────────────────────────────────────────────────┐
    │ CONNECTIONS   Sort: STATE ▼ | Filter: :443_ [x] clear  │
    │               [s]tate [p]ort [a]pp [i]p [/]regex       │
    └─────────────────────────────────────────────────────────┘

================================================================================
7. CONFIGURATION ARCHITECTURE
================================================================================

7.1 File Locations and Purposes

  FILE                                PURPOSE
  ────────────────────────────────────────────────────────────────
  /etc/sysmon/default.conf           System-wide defaults (shipped)
  ~/.config/sysmon/config.toml       User preferences (theme, keys, layout)
  ~/.config/sysmon/plugins.toml      Enabled plugins + plugin-specific settings
  ~/.local/share/sysmon/plugins/     User-installed plugin files
  ~/.local/share/sysmon/registry/    Cached plugin index
  ~/.cache/sysmon/                   Temporary data, safe to delete

7.2 Config File Content (config.toml)

  [collection]
  interval_ms = 1000
  max_processes = 10000

  [display]
  theme = "tokyo-night"
  refresh_rate_hz = 30
  compact_mode = false

  [display.panels]
  dashboard_layout = ["cpu", "memory", "network", "disk"]
  process_columns = ["pid", "cpu", "mem", "rss", "state", "user", "command"]

  [sorting]
  process_default_column = "cpu"
  process_default_order = "descending"

  [keybindings]
  quit = "q"
  dashboard = "F1"
  process_list = "F2"
  connections = "F3"
  plugins = "F4"
  help = "?"

  [alerts]
  enabled = false
  cpu_threshold = 90
  memory_threshold = 95

7.3 Plugin State File (plugins.toml)

  [plugins]
  enabled = ["gpu-nvidia", "docker", "temperature"]

  [plugins.config.gpu-nvidia]
  temperature_warn = 80
  temperature_crit = 95

  [plugins.config.docker]
  show_stopped = false

7.4 Config Loading Priority

  1. Command-line flags (--theme, --interval, etc.)
  2. ~/.config/sysmon/config.toml
  3. $XDG_CONFIG_HOME/sysmon/config.toml
  4. /etc/sysmon/default.conf
  5. Hardcoded defaults in theme_engine.c

7.5 CLI Config Commands

  sysmon config show              Print current effective config
  sysmon config show --defaults   Print built-in defaults
  sysmon config get <key>         Get specific value (e.g., display.theme)
  sysmon config set <key> <value> Set value, auto-saves
  sysmon config edit              Open config in $EDITOR
  sysmon config validate          Check syntax, report errors
  sysmon config reset             Restore to defaults
  sysmon reload                   Re-read config (also: SIGHUP)

================================================================================
8. THEME SYSTEM
================================================================================

8.1 Built-in Themes (defined in src/tui/theme.cpp)

  default         Clean, neutral
  dark            High contrast dark
  tokyo-night     Popular community theme
  nord            Arctic, bluish
  solarized-dark  Ethan Schoonover's classic
  dracula         Purple-tinted dark
  monokai         Warm contrast

8.2 Theme Structure

  typedef struct {
      char name[32];
      
      // Core palette (16 colors)
      int background;
      int foreground;
      int accent;
      int warning;
      int danger;
      int success;
      int info;
      
      // Semantic colors
      int cpu_color;
      int memory_color;
      int network_color;
      int disk_color;
      int header_bg;
      int header_fg;
      int border_focused;
      int border_unfocused;
      int table_header_bg;
      int table_row_even;
      int table_row_odd;
      int filter_bar_bg;
      int sort_indicator;
      
      // Bar styles
      char bar_filled[8];     // Character for filled portion
      char bar_empty[8];      // Character for empty portion
      int bar_width;          // Default bar width
  } Theme;

8.3 CLI Theme Commands

  sysmon theme list                 List available themes
  sysmon theme preview <name>       Show color swatches in terminal
  sysmon theme set <name>           Apply and save to config
  sysmon theme export <name>        Output theme as TOML to stdout
  sysmon theme import <file.toml>   Load custom theme from file

================================================================================
9. KEYBINDING ARCHITECTURE
================================================================================

9.1 Keybinding System

Keybindings are hierarchical:
  - Global keys (work everywhere)
  - Screen keys (work within a screen)
  - Panel keys (work when panel focused)
  - Modal keys (work in filter/search mode)

9.2 Keybinding Definition

  typedef struct {
      char action[64];          // "quit", "process_list", etc.
      int default_key;          // Default ncurses key code
      int user_key;             // User override (0 = use default)
      char description[128];    // For help screen
  } KeyBinding;

  Global keybindings array in input_handler.cpp
  Config file can override any binding by action name.

9.3 Default Key Map

  GLOBAL:
    q, Ctrl+C       quit
    F1              dashboard
    F2              process_list
    F3              connections
    F4              plugins
    F5, ?           help
    :               command_mode
    /               search_mode (context-dependent)

  PROCESS LIST:
    ↑↓, j/k         navigate
    Enter            process_detail
    F9               kill_process (SIGKILL)
    F10              term_process (SIGTERM)
    c                sort_by_cpu
    m                sort_by_memory
    p                sort_by_pid
    u                sort_by_user / filter_by_user
    s                sort_by_state / filter_by_state
    R                reverse_sort
    /                filter_text
    t                toggle_tree_view
    Esc              clear_filter / back

  CONNECTIONS:
    ↑↓, j/k          navigate
    s                sort_by_state / filter_by_state
    p                sort_by_port / filter_by_port
    a                sort_by_application
    i                sort_by_ip
    /                filter_text
    Esc              clear_filter / back

  PLUGINS PAGE:
    ↑↓, j/k          navigate plugins
    ←→               switch plugin pane
    e                toggle_plugin_enabled
    d                disable_plugin
    c                configure_plugin
    r                refresh_plugin
    Enter            expand/collapse plugin section

================================================================================
10. DATA FLOW V2
================================================================================

10.1 Collection Cycle

  Collection Engine (collection_engine.c)
  │
  ├─ Read /proc (proc_reader.c)
  │   ├─ cpu_stats.c      → CpuSnapshot
  │   ├─ mem_stats.c      → MemorySnapshot
  │   ├─ net_stats.c      → NetworkSnapshot
  │   ├─ disk_stats.c     → DiskSnapshot
  │   ├─ process_list.c   → ProcessTableSnapshot
  │   └─ sysinfo.c        → SystemInfo (cached)
  │
  ├─ Plugin Collection (plugin_manager.c)
  │   For each ENABLED plugin:
  │   ├─ Fork/exec plugin with "collect" argument
  │   ├─ Read stdout via pipe
  │   ├─ Parse protocol v2 (plugin_protocol.c)
  │   └─ Store in PluginData structs
  │
  ├─ Compute Derived Values
  │   ├─ CPU percentages (delta from previous)
  │   ├─ Network rates (delta from previous)
  │   ├─ Process CPU% (per-process delta)
  │   └─ Username resolution (username_cache.c, cached)
  │
  └─ Publish (snapshot_manager.c)
      └─ Atomic swap → UI can now read

10.2 UI Render Cycle

  Event Loop (event_loop.cpp)
  │
  ├─ Check for input (non-blocking)
  │   ├─ Keyboard → input_handler.cpp
  │   │   ├─ Global key? → screen_manager switch
  │   │   ├─ Screen key? → current screen handle
  │   │   ├─ Panel key? → focused panel handle
  │   │   └─ Modal key? → filter/search mode
  │   │
  │   └─ Resize? → notify all screens/panels
  │
  ├─ Get current snapshot
  │   └─ snapshot_manager_get_current() → const SystemSnapshot*
  │       (lock-free, always returns latest complete snapshot)
  │
  └─ Render active screen
      └─ screen->render(snapshot)
          └─ Each panel renders its portion
              ├─ cpu_panel reads snapshot->cpu
              ├─ process_table reads snapshot->processes
              │   └─ Applies sort_filter to create display list
              ├─ plugin_panel reads snapshot->plugin_data
              │   └─ For each plugin, renders its panes
              └─ etc.

10.3 Plugin Collection Data Flow

  plugin_manager.c (collection thread)
  │
  ├─ For each enabled plugin:
  │   ├─ Check if plugin exists in search paths
  │   ├─ Check if manifest is cached
  │   ├─ Run plugin process: plugin.sh collect
  │   ├─ Parse output via plugin_protocol.c
  │   │
  │   └─ Store in PluginData:
  │       typedef struct {
  │           char plugin_name[64];
  │           int num_panes;
  │           PluginPaneData panes[16];
  │       } PluginData;
  │
  │       typedef struct {
  │           char pane_name[64];
  │           int type;  // GAUGE, TABLE, TEXT
  │           
  │           // For GAUGE type
  │           int num_metrics;
  │           PluginMetric metrics[32];  // key, value, unit
  │           
  │           // For TABLE type
  │           int num_columns;
  │           char columns[16][32];
  │           int num_rows;
  │           char rows[256][16][128];  // [row][col]
  │       } PluginPaneData;
  │
  └─ All plugin data attached to snapshot->plugin_data

================================================================================
11. BUILD SYSTEM ARCHITECTURE
================================================================================

11.1 Library Dependency Graph

  sysmon-tui ──────┐
                   ├── libsysmon_tui.a (C++)
                   │       │
                   │       ├── libsysmon_core.a (C)
                   │       ├── libncurses
                   │       └── libstdc++
                   │
  sysmon-cli ──────┤
                   ├── libsysmon_core.a (C)
                   │
                   └── libsysmon_core.a dependencies:
                       ├── libpthread
                       ├── tomlplusplus (bundled header-only)
                       └── libcurl (optional, for registry fetch)

11.2 Build Targets

  make all            Build both sysmon-tui and sysmon-cli
  make tui            Build only TUI
  make cli            Build only CLI
  make debug          Debug build with sanitizers
  make release        Optimized build with LTO
  make install        Install binaries, plugins, completions
  make test           Run test suite
  make format         clang-format all source files
  make analyze        Static analysis with scan-build
  make registry       Update local plugin registry index

11.3 Installation Layout

  /usr/local/bin/sysmon              → symlink to sysmon-tui
  /usr/local/bin/sysmon-tui          TUI binary
  /usr/local/bin/sysmon-cli          CLI binary
  /usr/local/share/sysmon/plugins/   Builtin plugins
  /usr/local/share/sysmon/registry/  Initial registry index
  /etc/sysmon/default.conf           System defaults
  /usr/local/share/man/man1/         Man pages
  /usr/local/share/bash-completion/  Shell completions

================================================================================
12. KEY ARCHITECTURAL DECISIONS
================================================================================

12.1 Why Separate CLI Binary?

  - sysmon-cli can be used without ncurses (SSH, headless, scripting)
  - Smaller dependency footprint for management operations
  - Can run while TUI is active (e.g., install plugin from another terminal)
  - Cleaner code separation - CLI never touches UI code

12.2 Why TOML for Config?

  - Human-readable, well-specified
  - Comments supported (unlike JSON)
  - Good library support (toml++)
  - Familiar to users (Cargo, Rust ecosystem)
  - Sections map naturally to our config structure

12.3 Why Manifest per Plugin (not just .sh file)?

  - Plugin metadata without executing the script
  - Dependency checking before runtime
  - Version tracking for updates
  - Pane declarations for UI layout planning
  - Security: inspect manifest without running code

12.4 Why Plugin Protocol v2 (commands via argv)?

  - Plugin can be tested standalone: ./plugin.sh collect
  - Single execution = complete data (no long-running daemon)
  - No shared memory or IPC complexity
  - Plugin crashes don't affect sysmon
  - Timeouts easy to enforce (kill after N seconds)

12.5 Why Per-Pane Refresh Intervals?

  - GPU overview updates fast (1s), processes slow (5s)
  - Reduces plugin execution overhead
  - GPU memory data is cheaper to fetch than process list
  - Pane-level scheduling in plugin_manager.c

12.6 Why Plugin Page Instead of Plugin Panel?

  - V1 plugin_panel tried to squeeze all plugins into one panel
  - V2 plugin_page gives each plugin full width
  - Plugin can have multiple panes, each with full detail
  - Scrollable, collapsible, navigable
  - Matches user expectation (F4 = "see all my plugins")

================================================================================
13. MIGRATION FROM V1 TO V2
================================================================================

13.1 What Users Must Do

  - V1 plugins in scripts/plugins/*.sh:
    These become builtin plugins with manifest.toml
    User runs: sysmon plugin install gpu_monitor
    (one-time, plugin state saved)

  - V1 config in config/default.conf:
    User runs: sysmon config migrate
    (converts old format to ~/.config/sysmon/config.toml)

  - Keybindings:
    User runs: sysmon config export-keys > my-keys.toml
    (review, then sysmon config import-keys my-keys.toml)

13.2 What Developers Must Do

  - Add manifest.toml to each existing .sh plugin
  - Implement protocol v2 commands in plugins
  - Build cli/ subsystem
  - Add sort_filter to process_table and connection_table
  - Enhance cpu_panel with breakdown
  - Create plugin_page screen
  - Wire F4 to plugin_page
  - Implement config_parser.c (TOML)
  - Implement plugin_registry.c
  - Update bridge for new types

================================================================================
14. TESTING STRATEGY
================================================================================

  tests/
  ├── test_plugin_protocol.c    Protocol parsing (unit tests)
  ├── test_sort_filter.cpp      Sort/filter logic (unit tests)
  ├── test_config_parser.c      TOML parsing (unit tests)
  ├── test_registry.c           Registry operations (unit tests)
  ├── test_collection.c         Mock /proc, verify parsing
  ├── test_snapshot.c           Double-buffer atomicity
  └── test_plugin_mock.sh       Test plugin that outputs known data

  Integration tests:
  - test_plugin_lifecycle.sh    Install, enable, disable, uninstall flow
  - test_config_roundtrip.sh    Set config, read back, verify
  - test_theme_apply.sh         Apply theme, verify output colors

================================================================================
END OF V2 ARCHITECTURE DOCUMENTATION
================================================================================
