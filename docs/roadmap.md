# SYSMON — Build Roadmap

> Phased plan for building the system resource monitor from the ground up.
> Each phase produces a **testable, runnable** milestone.

---

## Phase 0: Project Scaffolding

**Goal:** Set up the directory structure, build system, and ensure a clean compile-link cycle.

### Tasks

- [ ] Create the full directory tree:
  ```
  src/
  ├── core/          # C — data types, /proc parsers, snapshot manager, collection engine
  ├── bridge/        # C header consumed by both C and C++ (sysmon_bridge.h)
  ├── tui/
  │   ├── components/   # C++ panels (cpu, memory, network, disk, process, connection, header)
  │   └── screens/      # C++ screens (dashboard, process list, connections, process detail, help)
  └── main.cpp
  config/
  └── default.conf
  scripts/
  ├── plugins/
  └── alerts/
  tests/
  docs/
  ```
- [ ] Write `CMakeLists.txt` (as specified in doc §8) with `sysmon_core` (static, C11) and `sysmon_tui` (static, C++17) libraries, and the `sysmon` executable.
- [ ] Write the convenience `Makefile` wrapper.
- [ ] Create `src/core/types.h` containing **all** C structs and enums from doc §2 (MetricClass, SystemInfo, CpuCoreSnapshot, CpuSnapshot, MemorySnapshot, NetworkIfaceSnapshot, NetworkSnapshot, ProcessSnapshot, ProcessTableSnapshot, ConnectionSnapshot, ConnectionTableSnapshot, DiskStatsSnapshot, DiskSnapshot, FilesystemSnapshot, FilesystemTableSnapshot, SystemSnapshot).
- [ ] Create `src/bridge/sysmon_bridge.h` — a thin `extern "C"` include guard that exposes the C API to C++ translation units.
- [ ] Add stub `.c` / `.cpp` files for every source listed in CMakeLists so the build succeeds (empty implementations, just enough to link).
- [ ] Create `src/main.cpp` with a trivial `main()` that returns 0.

### Verification

```bash
make build   # compiles and links with zero errors
./build/sysmon   # exits 0
```

---

## Phase 1: Core Data Collection (C)

**Goal:** Read real data from `/proc` and populate snapshot structs. No UI yet — validate via logging / a dump utility.

### Tasks

#### 1a — System Info (`src/core/sysinfo.c/.h`)
- [ ] Implement `sysinfo_collect(SystemInfo*)`.
- [ ] Parse hostname, kernel release, architecture, total memory, total swap, page size, clock ticks, CPU core count, boot time.
- [ ] Source: `/proc/meminfo`, `uname()`, `sysconf()`, `/proc/stat` (btime).

#### 1b — CPU Stats (`src/core/cpu_stats.c/.h`)
- [ ] Parse `/proc/stat` → fill `CpuCoreSnapshot[]` (per-core) and aggregate.
- [ ] Pre-compute `total_ticks`, `active_ticks`.
- [ ] Parse `/proc/loadavg` → `load_avg_1min/5min/15min`.

#### 1c — Memory Stats (`src/core/mem_stats.c/.h`)
- [ ] Parse `/proc/meminfo` → fill `MemorySnapshot`.
- [ ] Compute `used_bytes = total - available`, `usage_percent`.
- [ ] Handle swap, huge pages, slab, buffers, cached, shared, kernel stack, page tables.

#### 1d — Network Stats (`src/core/net_stats.c/.h`)
- [ ] Parse `/proc/net/dev` → fill `NetworkIfaceSnapshot[]`.
- [ ] Detect `is_loopback`, `is_up`.
- [ ] Rate fields (`rx_bytes_per_sec`, etc.) left at 0 for now — computed in Phase 2.

#### 1e — Process List (`src/core/process_list.c/.h`)
- [ ] Iterate `/proc/[pid]/stat`, `/proc/[pid]/status` → fill `ProcessSnapshot` array.
- [ ] Populate pid, ppid, comm, state, uid, utime, stime, rss, vsize, nice, num_threads.
- [ ] Detect `is_kernel_thread` (empty `/proc/[pid]/cmdline`), `is_zombie`.

#### 1f — Disk Stats (`src/core/disk_stats.c/.h`)
- [ ] Parse `/proc/diskstats` → fill `DiskStatsSnapshot[]`.
- [ ] Rate fields left at 0 for now.

#### 1g — `/proc` Reader Utilities (`src/core/proc_reader.c/.h`)
- [ ] Helpers: `read_file_to_buf()`, `parse_uint64()`, `parse_key_value_line()`, `iterate_proc_pids()`.
- [ ] Robust error handling per doc §4: zero-fill on missing, skip on permission denied, log malformed data.

#### 1h — Username Cache (`src/core/username_cache.c/.h`)
- [ ] Cache `uid → username` mappings via `getpwuid()`.
- [ ] TTL-based eviction (default 300s from config).

### Verification

```bash
# Write a small test main that collects once and prints all fields to stdout
make debug && ./build/sysmon   # dumps snapshot data
```

---

## Phase 2: Snapshot Manager & Collection Engine (C)

**Goal:** Double-buffered snapshot lifecycle with a background collection thread. Rates are now computed.

### Tasks

#### 2a — Snapshot Manager (`src/core/snapshot_manager.c/.h`)
- [ ] Allocate two `SystemSnapshot` buffers at init.
- [ ] Shared `SystemInfo*` between both buffers (collected once).
- [ ] Implement `snapshot_manager_init()`, `snapshot_manager_destroy()`.
- [ ] Implement `snapshot_manager_get_current()` — atomic load with acquire semantics.
- [ ] Implement `snapshot_manager_get_next()` — returns the non-current buffer.
- [ ] Implement `snapshot_manager_publish()` — atomic store with release semantics to swap.
- [ ] Dynamic growth for process and connection arrays (grow to max seen, never shrink during runtime).

#### 2b — Collection Engine (`src/core/collection_engine.c/.h`)
- [ ] Implement the four-phase collection cycle from doc §4:
  1. Raw data gathering (Phase 1 parsers).
  2. Expensive operations (processes, connections, disk).
  3. Compute derived values (CPU %, network rates, process CPU %, disk IOPS).
  4. Metadata + publish.
- [ ] `collection_engine_init()`, `collection_engine_start()` (spawns `pthread`), `collection_engine_stop()`, `collection_engine_destroy()`.
- [ ] Store previous CPU/network snapshots for delta-based rate computation.
- [ ] Respect `collection_interval_ms`, `process_max_count`, `collect_connections`, `collect_disk_io`.
- [ ] Callback hooks: `on_collection_start`, `on_collection_complete`, `on_collection_error`.

#### 2c — Rate Computations
- [ ] `compute_cpu_percentages()` — `delta(active_ticks) / delta(total_ticks) * 100` per core.
- [ ] `compute_network_rates()` — `delta(bytes) / delta(time)` per interface.
- [ ] `compute_process_cpu_percentages()` — `delta(utime+stime) / delta(total_cpu_ticks) * 100`.
- [ ] `compute_disk_rates()` — `delta(sectors * 512) / delta(time)`, IOPS.

### Verification

```bash
# Test program that starts the engine, sleeps 3s, reads current snapshot, prints stats with rates
make debug && ./build/sysmon
# Observe non-zero CPU%, network rates, process CPU%
```

---

## Phase 3: TUI Foundation (C++)

**Goal:** ncurses initialisation, Panel/Screen base classes, event loop, theme system. A blank terminal that handles resize and quit.

### Tasks

#### 3a — Application Shell (`src/tui/app.cpp/.h`)
- [ ] `Application::run()` — init ncurses, start collection engine, enter event loop, cleanup on exit.
- [ ] Signal handling: SIGINT, SIGTERM → graceful shutdown. SIGWINCH → resize.
- [ ] Set ncurses: `noecho()`, `cbreak()`, `curs_set(0)`, `keypad(stdscr, TRUE)`, `nodelay()` for non-blocking input, `start_color()`.

#### 3b — Panel Base Class (`src/tui/components/panel.h`)
- [ ] Implement abstract `Panel` per doc §3: constructor takes geometry, owns `WINDOW*`, RAII cleanup.
- [ ] `draw_border()`, `draw_title()`, `clear_content()`.
- [ ] Virtual: `render()`, `on_focus()`, `on_blur()`, `on_resize()`, `handle_input()`.

#### 3c — Screen Base Class (`src/tui/screens/screen.h`)
- [ ] Implement `Screen` per doc §3: owns `vector<unique_ptr<Panel>>`.
- [ ] Virtual: `render()`, `handle_input()`, `on_enter()`, `on_exit()`, `on_resize()`.

#### 3d — Screen Manager (`src/tui/screen_manager.cpp/.h`)
- [ ] Manages active screen. `switch_screen()`, `render()`, `handle_input()`, `on_resize()`.
- [ ] Dispatches input to the active screen, then to global handlers.

#### 3e — Event Loop (`src/tui/event_loop.cpp/.h`)
- [ ] `getch()` loop at configurable refresh rate (default 30 Hz).
- [ ] Dispatch to `InputHandler` → `ScreenManager` → active screen.
- [ ] Trigger re-render only when snapshot sequence changes or input received.

#### 3f — Input Handler (`src/tui/input_handler.cpp/.h`)
- [ ] Global key bindings from doc §6: `q`/Ctrl+C quit, F1 help, F5 process list, F6 connections, F7 dashboard, `r` refresh, `p` pause, `/` search, `:` command.

#### 3g — Theme Manager (`src/tui/theme.cpp/.h`)
- [ ] Define color pairs for: normal, header, bar-fill, bar-empty, highlight, focused-border, warning, error, info.
- [ ] Support themes: default, dark, light, solarized.
- [ ] Respect `color = 16 | 256 | truecolor` from config.

### Verification

```bash
make build && ./build/sysmon
# Black screen with no crash, q quits cleanly, resize is handled
```

---

## Phase 4: Dashboard Screen

**Goal:** The default view showing CPU, memory, network, and disk panels with live data.

### Tasks

#### 4a — Header Panel (`src/tui/components/header.cpp/.h`)
- [ ] Display: hostname, kernel, uptime, load averages, collection timing.
- [ ] Single line or two lines at the top of screen.

#### 4b — CPU Panel (`src/tui/components/cpu_panel.cpp/.h`)
- [ ] Aggregate CPU bar (overall usage %).
- [ ] Per-core bars (horizontal, labeled).
- [ ] Color gradient: green → yellow → red based on usage.
- [ ] Show `user` / `system` / `iowait` breakdown if space allows.

#### 4c — Memory Panel (`src/tui/components/memory_panel.cpp/.h`)
- [ ] Main memory gauge bar.
- [ ] Breakdown: used, buffers, cached, free.
- [ ] Swap bar (if swap exists).
- [ ] Human-readable byte formatting (`format_bytes()`).

#### 4d — Network Panel (`src/tui/components/network_panel.cpp/.h`)
- [ ] List active (non-loopback, up) interfaces.
- [ ] Show RX/TX rates in human-readable format (KB/s, MB/s).
- [ ] Totals across all interfaces.

#### 4e — Disk Panel (`src/tui/components/disk_panel.cpp/.h`)
- [ ] List disk devices with read/write rates (MB/s) and IOPS.
- [ ] Exclude loop devices and partitions (show whole disks only).

#### 4f — Dashboard Screen (`src/tui/screens/dashboard_screen.cpp/.h`)
- [ ] Compose: HeaderPanel + CpuPanel + MemoryPanel + NetworkPanel + DiskPanel.
- [ ] Responsive layout: adapt panel sizes to terminal dimensions.
- [ ] Register as the default screen on launch.

### Verification

```bash
make build && ./build/sysmon
# Dashboard with live-updating CPU bars, memory gauge, network rates, disk I/O
# Resize terminal → panels re-layout
```

---

## Phase 5: Process List & Detail Screens

**Goal:** Full interactive process table with sorting, filtering, tree view, and a deep-dive detail screen.

### Tasks

#### 5a — Process Table Panel (`src/tui/components/process_table.cpp/.h`)
- [ ] Columns: PID, USER, PRI, NI, VIRT, RES, S, CPU%, MEM%, TIME+, COMMAND.
- [ ] Scrollable list with selection highlight.
- [ ] Sorting by any column (`s` to cycle, `R` to reverse).
- [ ] Filter by name (`/`), filter by user (`u`).
- [ ] Tree view toggle (`t`) — reconstruct parent-child hierarchy from `ppid`.
- [ ] Vim-style navigation: `j/k`, `PgUp/PgDn`, `Home/End`.

#### 5b — Process List Screen (`src/tui/screens/process_list_screen.cpp/.h`)
- [ ] Compose: HeaderPanel + ProcessTablePanel.
- [ ] `Enter` on selection → switch to ProcessDetailScreen.
- [ ] `F9` → SIGKILL selected, `F10` → SIGTERM selected (with confirmation).

#### 5c — Process Detail Panel (`src/tui/components/process_detail.cpp/.h`)
- [ ] Show full details for a single PID: all fields from `ProcessSnapshot`.
- [ ] Read `/proc/[pid]/cmdline` on demand for full command line.
- [ ] Read `/proc/[pid]/environ` on demand (truncated).
- [ ] FD count from `/proc/[pid]/fd/`.
- [ ] I/O stats if available.

#### 5d — Process Detail Screen (`src/tui/screens/process_detail_screen.cpp/.h`)
- [ ] Single panel full-screen detail view.
- [ ] `Esc`/`Backspace` → back to process list.
- [ ] `k` → SIGTERM, `K` → SIGKILL.
- [ ] `c` → copy command line to clipboard (if possible).

### Verification

```bash
make build && ./build/sysmon
# F5 → process list, sort by CPU, filter by name, Enter → detail view, Esc → back
```

---

## Phase 6: Network Connections Screen

**Goal:** Live TCP/UDP/Unix socket table, parsed from `/proc/net/*`.

### Tasks

#### 6a — Connection Parsing (`src/core/proc_reader.c` additions)
- [ ] Parse `/proc/net/tcp`, `/proc/net/tcp6` → TCP connections.
- [ ] Parse `/proc/net/udp`, `/proc/net/udp6` → UDP connections.
- [ ] Parse `/proc/net/unix` → Unix sockets.
- [ ] Resolve hex addresses to dotted-quad strings.
- [ ] Map inode → PID (via `/proc/[pid]/fd/` → socket inode lookup).
- [ ] Fill `ConnectionSnapshot` structs.

#### 6b — Connection Table Panel (`src/tui/components/connection_table.cpp/.h`)
- [ ] Columns: PROTO, LOCAL ADDRESS, REMOTE ADDRESS, STATE, PID, PROGRAM.
- [ ] Sortable, filterable.
- [ ] Scrollable with selection.

#### 6c — Connection Screen (`src/tui/screens/connection_screen.cpp/.h`)
- [ ] Compose: HeaderPanel + ConnectionTablePanel.
- [ ] Lazy collection: set `collect_connections = true` on screen enter, `false` on exit.

#### 6d — Filesystem Table (bonus)
- [ ] Parse `/proc/mounts` + `statvfs()` → `FilesystemSnapshot`.
- [ ] Display mount point, type, size, used, available, usage bar.
- [ ] Collected every 30s (lower frequency).

### Verification

```bash
make build && ./build/sysmon
# F6 → connections view shows TCP/UDP sockets with PIDs
```

---

## Phase 7: Configuration & Help

**Goal:** Full config file parsing, command-line argument handling, and in-app help screen.

### Tasks

#### 7a — Config Parser
- [ ] Parse INI-style config from `~/.config/sysmon/config`, `/etc/sysmon/default.conf`, built-in defaults.
- [ ] Priority: CLI args > user config > system config > defaults (doc §9).
- [ ] Map config values to `CollectionEngine` and `Application` settings.

#### 7b — Command-Line Arguments
- [ ] `--help`, `--version`.
- [ ] `--interval <ms>`, `--theme <name>`, `--no-color`.
- [ ] `--debug`, `--log-file <path>`.

#### 7c — Help Screen (`src/tui/screens/help_screen.cpp/.h`)
- [ ] Display all key bindings from doc §6.
- [ ] Scrollable if content exceeds terminal height.
- [ ] F1 toggles help on/off.

#### 7d — Default Config File
- [ ] Write `config/default.conf` with all documented defaults.

### Verification

```bash
./build/sysmon --help        # prints usage
./build/sysmon --interval 500  # faster refresh
# F1 in app → help screen with key bindings
```

---

## Phase 8: Plugin System & Polish

**Goal:** Bash plugin lifecycle, final polish, exit codes, and testing.

### Tasks

#### 8a — Plugin Manager (`src/core/plugin_manager.c/.h`)
- [ ] `plugin_manager_init()` — scan plugin directories.
- [ ] `plugin_manager_load_plugins()` — detect scripts, parse header comments (PLUGIN, DESCRIPTION, INTERVAL, REQUIRES).
- [ ] `plugin_manager_start_all()` — fork/exec each plugin, set up stdout pipes, send config JSON on stdin.
- [ ] `plugin_manager_collect()` — non-blocking read from all plugin pipes, parse the line protocol (`type:key=value[:unit]`).
- [ ] `plugin_manager_stop_all()` — SIGTERM, then SIGKILL after timeout.
- [ ] Handle `table:` / `col:` / `row:` / `endtable` protocol.
- [ ] Respect `plugin_timeout` from config.

#### 8b — Plugin Panel (`src/tui/components/plugin_panel.cpp/.h`)
- [ ] Generic panel that renders plugin gauge/counter/table data.
- [ ] Dynamically added to dashboard when plugin data is available.

#### 8c — Example Plugins
- [ ] `scripts/plugins/gpu_monitor.sh` — NVIDIA GPU via `nvidia-smi`.
- [ ] `scripts/plugins/temperature.sh` — CPU temp from `/sys/class/thermal/`.
- [ ] `scripts/plugins/battery.sh` — Battery status from `/sys/class/power_supply/`.

#### 8d — Exit Codes & Error Handling
- [ ] Implement all exit codes from doc Appendix B (0–7, 130, 143).
- [ ] Check `/proc` mounted on startup.
- [ ] Check minimum terminal size (80×24).
- [ ] Single-instance lock file.

#### 8e — Polish
- [ ] Smooth transitions between screens.
- [ ] Pause/resume collection (`p` key).
- [ ] Force-refresh (`r` key).
- [ ] Command mode (`:`) — e.g., `:kill <pid>`, `:theme dark`.
- [ ] Search mode (`/`) global filter.

#### 8f — Testing
- [ ] Unit tests for all `/proc` parsers (mock data).
- [ ] Unit tests for snapshot manager (thread safety, publish/read).
- [ ] Unit tests for rate computations.
- [ ] Integration test: start engine → collect 3 cycles → verify snapshot fields.
- [ ] Wire up `tests/CMakeLists.txt` with CTest.

### Verification

```bash
make test    # all tests pass
make build && ./build/sysmon   # full-featured monitor
make install-plugins           # plugins installed and auto-started
```

---

## Phase Summary

| Phase | Name | Key Deliverable | Approx. Files |
|-------|------|-----------------|---------------|
| 0 | Scaffolding | Clean compile, directory tree, all types defined | ~25 stubs |
| 1 | Data Collection | Real `/proc` data in structs | 10 `.c/.h` |
| 2 | Snapshot & Engine | Threaded double-buffered collection with rates | 4 `.c/.h` |
| 3 | TUI Foundation | ncurses app shell, base classes, event loop | 8 `.cpp/.h` |
| 4 | Dashboard | Live CPU/Mem/Net/Disk panels | 6 `.cpp/.h` |
| 5 | Processes | Sortable process table + detail view | 4 `.cpp/.h` |
| 6 | Connections | Network socket viewer + filesystem table | 3 `.cpp/.h` + parser additions |
| 7 | Config & Help | Config parsing, CLI args, help screen | 3 `.cpp/.h` + config |
| 8 | Plugins & Polish | Bash plugin system, tests, final polish | 4 `.c/.h` + scripts + tests |

---

> **How to use this roadmap:** Work through phases in order. Each phase builds on the previous one. Within a phase, tasks can often be parallelised (e.g., all Phase 1 parsers are independent). Don't move to the next phase until the current phase's verification passes.
