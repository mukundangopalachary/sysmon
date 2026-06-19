# SYSMON V2 — Build Roadmap

> Phased plan for evolving sysmon from a single-binary monitor into a dual-binary platform with CLI plugin management, TOML configuration, enhanced UI, and a plugin registry.
> Each phase produces a **testable, runnable** milestone.

---

## Phase 0: Codebase Reorganization & CLI Scaffolding

**Goal:** Restructure the repository for the dual-binary architecture. Introduce the `src/cli/` module and the `sysmon-cli` build target. Everything still compiles and runs identically to V1.

### Tasks

- [ ] Create `src/cli/` directory with `cli_main.c` and `cli_main.h`.
- [ ] Update `src/main.cpp` to detect `argv[0]` or first argument — dispatch to TUI or CLI mode.
- [ ] Update `CMakeLists.txt`:
  - Add `sysmon-cli` executable target linking only `libsysmon_core`.
  - Add `sysmon-tui` as an alias for the existing `sysmon` binary.
  - Create a `sysmon` symlink target pointing to `sysmon-tui`.
- [ ] Update `Makefile` with new targets: `make all`, `make tui`, `make cli`.
- [ ] Stub out CLI dispatch: `sysmon plugin`, `sysmon config`, `sysmon theme` print "not yet implemented".
- [ ] Reorganize `docs/` directory with proper naming conventions (already done in this commit).

### Verification

```bash
make all                    # builds both sysmon-tui and sysmon-cli
./build/sysmon              # launches TUI as before
./build/sysmon-cli plugin list  # prints stub message
```

---

## Phase 1: TOML Configuration System

**Goal:** Replace the INI-style `default.conf` with a proper TOML-based configuration engine. Implement `config_parser.c` in the core library, XDG-compliant paths, and the `sysmon config` CLI commands.

### Tasks

#### 1a — TOML Parser (`src/core/config_parser.c/.h`)
- [ ] Integrate a lightweight C TOML parser (either bundled or header-only).
- [ ] Implement `config_load()`, `config_get_string()`, `config_get_int()`, `config_get_bool()`.
- [ ] Support the full `config.toml` schema from V2 architecture doc §7.2.
- [ ] Implement config file priority chain: CLI flags → user config → system config → hardcoded defaults.

#### 1b — XDG Path Resolution (`src/core/config_paths.c/.h`)
- [ ] Implement `config_get_user_config_path()` → `~/.config/sysmon/config.toml`.
- [ ] Implement `config_get_user_data_path()` → `~/.local/share/sysmon/`.
- [ ] Implement `config_get_user_cache_path()` → `~/.cache/sysmon/`.
- [ ] Auto-create directories on first use.

#### 1c — CLI Config Commands (`src/cli/cli_config.c`)
- [ ] `sysmon config show` — print effective merged config.
- [ ] `sysmon config get <key>` — print single value.
- [ ] `sysmon config set <key> <value>` — write to user config file.
- [ ] `sysmon config edit` — open user config in `$EDITOR`.
- [ ] `sysmon config validate` — check syntax and report errors.
- [ ] `sysmon config reset` — remove user config (restore defaults).

#### 1d — Default Config Migration
- [ ] Write `config/default.toml` with all default values.
- [ ] Implement `sysmon config migrate` — convert V1 `default.conf` to `config.toml`.

### Verification

```bash
sysmon config show           # prints merged config
sysmon config set display.theme tokyo-night
sysmon config get display.theme  # prints "tokyo-night"
cat ~/.config/sysmon/config.toml  # file exists and is valid TOML
```

---

## Phase 2: Plugin System V2 — Manifests & Lifecycle

**Goal:** Upgrade the plugin system from raw `.sh` files to structured plugin directories with `manifest.toml` and protocol V2 command support. Implement the CLI plugin management commands.

### Tasks

#### 2a — Plugin Manifest Parser (`src/core/plugin_manifest.c/.h`)
- [ ] Parse `manifest.toml` from plugin directories.
- [ ] Extract: name, version, description, author, license, requirements, panes, alerts.
- [ ] Validate required fields and dependency executables.
- [ ] Cache parsed manifests to avoid re-reading on every collection cycle.

#### 2b — Plugin State Manager (`src/core/plugin_state.c/.h`)
- [ ] Read/write `~/.config/sysmon/plugins.toml` — tracks enabled plugins and per-plugin config.
- [ ] Implement plugin state machine: NOT_INSTALLED → INSTALLED → ENABLED/DISABLED → BROKEN.
- [ ] Search path resolution: user dir → system dir → system-alt dir (§4.1).

#### 2c — Plugin Protocol V2 (`src/core/plugin_protocol.c/.h`)
- [ ] Invoke plugins with command arguments: `metadata`, `panes`, `collect`, `pane <name>`, `check`.
- [ ] Parse `KEY=VALUE:TYPE:UNIT` output format.
- [ ] Parse `TABLE_START` / `COLUMNS:` / `TABLE_END` table protocol.
- [ ] Parse `PANE:name` / `ENDPANE` pane grouping markers.
- [ ] Timeout enforcement per plugin execution.

#### 2d — CLI Plugin Commands (`src/cli/cli_plugin.c`)
- [ ] `sysmon plugin list` — show installed plugins with name, version, enabled status, description.
- [ ] `sysmon plugin info <name>` — show full manifest details.
- [ ] `sysmon plugin install <name>` — download from registry, place in `~/.local/share/sysmon/plugins/`.
- [ ] `sysmon plugin uninstall <name>` — remove with confirmation prompt.
- [ ] `sysmon plugin enable <name>` / `disable <name>` — toggle in `plugins.toml`.
- [ ] `sysmon plugin update <name>` / `--all` — check registry for newer versions.
- [ ] `sysmon plugin check <name>` — run plugin's `check` command and report status.

#### 2e — Migrate Builtin Plugins
- [ ] Convert `scripts/plugins/gpu_monitor.sh` → `scripts/plugins/builtin/gpu-monitor/plugin.sh` + `manifest.toml`.
- [ ] Convert `scripts/plugins/temperature.sh` → `scripts/plugins/builtin/temperature/plugin.sh` + `manifest.toml`.
- [ ] Implement protocol V2 commands in both plugins.

### Verification

```bash
sysmon plugin list                # shows builtin plugins
sysmon plugin check temperature   # verifies dependencies
sysmon plugin enable gpu-monitor  # adds to plugins.toml
sysmon plugin info gpu-monitor    # shows manifest details
```

---

## Phase 3: Plugin Registry

**Goal:** Implement local and remote plugin registry support. Allow discovery and installation of community plugins via `sysmon plugin search` and `sysmon plugin install`.

### Tasks

#### 3a — Registry Index (`src/core/plugin_registry.c/.h`)
- [ ] Define `index.toml` format: list of plugins with name, version, description, download URL, checksum.
- [ ] Implement `registry_load_local()` — read cached index from `~/.local/share/sysmon/registry/`.
- [ ] Implement `registry_search()` — substring match on name and description.

#### 3b — Registry Sync
- [ ] Implement `registry_update()` — fetch remote `index.toml` via HTTP (optional `libcurl` dependency).
- [ ] Fallback: manual download and placement of `index.toml`.
- [ ] `sysmon registry update` CLI command.

#### 3c — Plugin Download & Install
- [ ] Download plugin tarball from registry URL.
- [ ] Verify checksum before extraction.
- [ ] Extract to `~/.local/share/sysmon/plugins/<name>/`.
- [ ] Create `version.lock` file to track installed version.

### Verification

```bash
sysmon registry update           # fetches latest index
sysmon plugin search docker      # finds matching plugins
sysmon plugin install docker     # downloads and installs
sysmon plugin list               # shows newly installed plugin
```

---

## Phase 4: Enhanced CPU Pane

**Goal:** Upgrade the CPU panel from simple percentage bars to a comprehensive breakdown with usage types, top consumers, load context, and thermal data (via plugin).

### Tasks

#### 4a — CPU Usage Breakdown
- [ ] Parse additional `/proc/stat` fields: user, nice, system, idle, iowait, irq, softirq, steal.
- [ ] Display stacked breakdown bars: User | System | IOWait | Steal.
- [ ] Add context switches (`/proc/stat` ctxt line) and interrupt rate.

#### 4b — Top CPU Consumers (inline)
- [ ] Sort processes by `cpu_percent` in the snapshot.
- [ ] Display top 3 consumers inline within the CPU pane: name, CPU%, thread count, PID.
- [ ] Update per collection cycle (no extra overhead, data already collected).

#### 4c — Load Context
- [ ] Display load averages relative to core count (e.g., `Load: 2.1 / 16 cores`).
- [ ] Color-code load: green (< cores), yellow (≈ cores), red (> cores).

#### 4d — Thermal Integration
- [ ] If the `temperature` plugin is enabled, read its data from `snapshot->plugin_data`.
- [ ] Display thermal gauge inline: `Package 67°C / max 100°C [████████░░░] 67%`.
- [ ] If plugin is disabled, hide the thermal section entirely.

#### 4e — Per-Core Toggle
- [ ] Press `c` on the dashboard to toggle between overview mode and per-core bar view.

### Verification

```bash
sysmon   # CPU pane shows breakdown, top consumers, load, thermal (if plugin enabled)
# Press 'c' to toggle per-core view
```

---

## Phase 5: Sorting & Filtering Framework

**Goal:** Build a generic, reusable sort/filter engine and integrate it into the Process Table and Connection Table.

### Tasks

#### 5a — Sort/Filter Component (`src/tui/components/sort_filter.cpp/.h`)
- [ ] Implement `sort_state_t` with column index, direction, and comparator function pointer.
- [ ] Implement `filter_state_t` with filter text, mode (TEXT, REGEX, STATE, USER, PID).
- [ ] Implement `filter_match()` — case-insensitive substring or POSIX regex matching.

#### 5b — Generic Table View (`src/tui/components/table_view.cpp/.h`)
- [ ] Column definition: name, width, alignment, sortable flag.
- [ ] Render with sort indicators (▲/▼) in column headers.
- [ ] Filter bar rendering at the top of the table.
- [ ] Keyboard dispatch for sort/filter/scroll actions.

#### 5c — Process Table Integration
- [ ] Integrate `sort_filter` into `ProcessTablePanel`.
- [ ] Keybindings: `c` (CPU), `m` (MEM), `p` (PID), `u` (User), `s` (State), `R` (Reverse), `/` (Filter).
- [ ] Display active sort column and filter text in the header bar.

#### 5d — Connection Table Integration
- [ ] Integrate `sort_filter` into `ConnectionTablePanel`.
- [ ] Keybindings: `s` (State), `p` (Port), `a` (Application), `i` (IP), `/` (Filter).
- [ ] Display active sort column and filter text in the header bar.

### Verification

```bash
sysmon                       # launch TUI
# Press 'p' for process list → press 'c' to sort by CPU, '/' to filter
# Press 'c' for connections → press 's' to sort by state, '/' to filter by port
```

---

## Phase 6: Plugin Page (F4)

**Goal:** Create a dedicated full-screen Plugin Page accessible via `F4`. Each enabled plugin gets its own section with all its panes rendered.

### Tasks

#### 6a — Plugin Page Screen (`src/tui/screens/plugin_page_screen.cpp/.h`)
- [ ] New screen registered as `F4` in the screen manager.
- [ ] Query `snapshot->plugin_data` for all enabled plugins.
- [ ] Render each plugin as a collapsible section (title bar + panes).
- [ ] Vertically scrollable when content exceeds terminal height.

#### 6b — Plugin Pane Renderer (`src/tui/components/plugin_pane.cpp/.h`)
- [ ] Render GAUGE-type panes: key-value pairs with optional bar visualization.
- [ ] Render TABLE-type panes: column headers + rows using `table_view`.
- [ ] Render TEXT-type panes: raw text output.
- [ ] Per-pane refresh interval display (e.g., `[auto: 1s]`).

#### 6c — Plugin Page Interaction
- [ ] `↑↓` / `j/k` — navigate between plugins.
- [ ] `Enter` — expand/collapse a plugin section.
- [ ] `e` — toggle plugin enabled/disabled.
- [ ] `c` — open plugin config in `$EDITOR`.
- [ ] `r` — force-refresh a specific plugin.

### Verification

```bash
sysmon                       # launch TUI
# Press F4 → see all enabled plugins with full pane data
# Navigate with j/k, expand/collapse with Enter
```

---

## Phase 7: Theme Engine & Keybinding System

**Goal:** Implement the full theme engine with built-in themes (tokyo-night, nord, dracula, etc.) and a configurable keybinding system.

### Tasks

#### 7a — Theme Engine (`src/tui/theme.cpp` rewrite)
- [ ] Define `Theme` struct with full color palette (background, foreground, accent, semantic colors, bar styles).
- [ ] Implement built-in themes: default, dark, tokyo-night, nord, solarized-dark, dracula, monokai.
- [ ] Load user theme overrides from `config.toml` `[display]` section.
- [ ] `sysmon theme list`, `sysmon theme set <name>`, `sysmon theme preview <name>`.

#### 7b — Keybinding System (`src/tui/input_handler.cpp` rewrite)
- [ ] Define `KeyBinding` struct: action name, default key, user override, description.
- [ ] Hierarchical dispatch: Global → Screen → Panel → Modal.
- [ ] Load user keybinding overrides from `config.toml` `[keybindings]` section.

#### 7c — Updated Help Screen
- [ ] Dynamically generate help content from the keybinding registry.
- [ ] Display both default and user-overridden keys.
- [ ] Group by context: Global, Process List, Connections, Plugins.

### Verification

```bash
sysmon theme set tokyo-night   # apply theme
sysmon                         # TUI renders with tokyo-night colors
# Press '?' for help → shows all keybindings grouped by context
```

---

## Phase 8: Testing, Polish & Documentation

**Goal:** Comprehensive test suite, final UI polish, shell completions, man pages, and updated documentation.

### Tasks

#### 8a — Unit Tests
- [ ] `tests/test_config_parser.c` — TOML parsing correctness.
- [ ] `tests/test_plugin_protocol.c` — protocol V2 output parsing.
- [ ] `tests/test_sort_filter.cpp` — sort/filter logic with mock data.
- [ ] `tests/test_registry.c` — registry index loading and search.
- [ ] `tests/test_collection.c` — mock `/proc` data, verify snapshot fields.
- [ ] `tests/test_snapshot.c` — double-buffer atomicity under concurrent access.

#### 8b — Integration Tests
- [ ] `tests/test_plugin_lifecycle.sh` — install, enable, disable, uninstall flow.
- [ ] `tests/test_config_roundtrip.sh` — set config, read back, verify.
- [ ] `tests/test_plugin_mock.sh` — test plugin that outputs known data.

#### 8c — Shell Completions
- [ ] Bash completion script for `sysmon` and `sysmon-cli` subcommands.
- [ ] Zsh completion script.
- [ ] Install to `/usr/local/share/bash-completion/` and `/usr/local/share/zsh/site-functions/`.

#### 8d — Man Pages
- [ ] `sysmon.1` — TUI usage and keybindings.
- [ ] `sysmon-cli.1` — CLI subcommands reference.

#### 8e — Documentation Updates
- [ ] Update `README.md` with V2 features, new keybindings, plugin system overview.
- [ ] Update `RELEASE_NOTES.md` with V2 changelog.
- [ ] Update `CONTRIBUTING.md` with plugin development guide.

### Verification

```bash
make test                    # all unit and integration tests pass
make install                 # installs binaries, plugins, completions, man pages
sysmon --help                # updated help text with V2 features
man sysmon                   # man page renders correctly
```

---

## Phase Summary

| Phase | Name | Key Deliverable | Approx. Files |
|-------|------|-----------------|---------------|
| 0 | Codebase Reorganization | Dual-binary build, `src/cli/` module | ~5 new/modified |
| 1 | TOML Configuration | Config parser, XDG paths, CLI config commands | ~6 `.c/.h` |
| 2 | Plugin System V2 | Manifests, protocol V2, CLI plugin commands | ~8 `.c/.h` |
| 3 | Plugin Registry | Remote index, search, download, install | ~3 `.c/.h` |
| 4 | Enhanced CPU Pane | Breakdown, top consumers, thermal integration | ~2 `.cpp/.h` |
| 5 | Sorting & Filtering | Generic table view, sort/filter in process + connections | ~4 `.cpp/.h` |
| 6 | Plugin Page (F4) | Full-screen plugin page with collapsible sections | ~3 `.cpp/.h` |
| 7 | Theme & Keybindings | Built-in themes, configurable keybindings, updated help | ~3 `.cpp/.h` |
| 8 | Testing & Polish | Test suite, shell completions, man pages, docs | ~12 files |

---

> **How to use this roadmap:** Work through phases in order. Each phase builds on the previous one. Within a phase, tasks can often be parallelized (e.g., all Phase 1 sub-tasks are independent of each other). Do not move to the next phase until the current phase's verification passes.
