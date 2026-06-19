# SYSMON - System Resource Monitor

SYSMON is a high-performance, terminal-based system resource monitor written in C and C++. It provides real-time insights into system health, including CPU, memory, network, disk I/O, process lists, and active network connections.

## Features

- **Real-Time Dashboard**: Instant overview of CPU per-core usage, memory gauge, disk I/O, and network throughput.
- **Interactive Process Manager**: Sort, filter, and drill down into process details (environment, file descriptors, CPU/Mem usage) with Vim-style keybindings.
- **Live Network Connections**: View active TCP/UDP/Unix sockets mapped to their owning processes.
- **Double-Buffered Architecture**: Ensures the UI thread never blocks while the background thread collects `/proc` data.
- **Extensible Plugin System**: First-class support for Bash scripts to collect external metrics (e.g., GPU usage, temperature sensors) via a simple line protocol.
- **Highly Configurable**: Custom UI themes, update intervals, and data collection scopes.

## Architecture

SYSMON uses a multi-threaded, bifurcated architecture:
1. **Core Data Engine (C11)**: A highly optimized background thread that iterates through `/proc` and `/sys` to gather data into immutable memory snapshots.
2. **TUI Framework (C++17)**: An event-driven rendering thread powered by `ncurses` that consumes the snapshots without blocking and provides interactive views.

For deep technical details, see the full [Architecture Documentation](docs/whole_doc.md).

## Prerequisites

- **Compiler**: GCC or Clang (C11 and C++17 support required)
- **Build System**: CMake (>= 3.15) and Make
- **Libraries**: `ncurses` development headers (`libncurses-dev` on Debian/Ubuntu, `ncurses-devel` on RHEL/Fedora)

## Installation

### Method 1: The One-Liner (Recommended)
If you have `git`, `cmake`, and `make` installed, you can download, compile, and install Sysmon globally with a single command:
```bash
curl -fsSL https://raw.githubusercontent.com/mukundangopalachary/sysmon/master/install.sh | bash
```

### Method 2: Download the Pre-Compiled Binary

If you don't want to compile the project yourself, you can download the ready-to-use binary directly from our GitHub Releases page!

1. Go to the **Releases** tab on GitHub and download the `sysmon` executable.
2. Make it executable:
   ```bash
   chmod +x sysmon
   ```
3. Move it to your system binaries so you can run it from anywhere:
   ```bash
   sudo mv sysmon /usr/local/bin/
   ```

### Method 3: Building from Source (Manual)

To compile the source code yourself, you will need `cmake`, `make`, `gcc/g++`, and `libncurses-dev`.

```bash
# 1. Clone the repository
git clone https://github.com/mukundangopalachary/sysmon.git
cd sysmon

# 2. Build the project
make build

# 3. Install globally (this allows you to run `sysmon` from anywhere)
sudo make install
```

Other available Make targets:
- `make debug`: Build with address and undefined behavior sanitizers enabled.
- `make test`: Run the test suite.
- `make install`: Install to system paths (`/usr/local/bin` and config files to `/usr/local/share`).

## Usage

Simply type `sysmon` in your terminal to launch the dashboard.

**Keybindings:**
- `[d]` Dashboard View
- `[p]` Process List
- `[c]` Connections View
- `[q]` Quit Application
- `[Up/Down]` Navigate lists
- `[ENTER]` View deeper process details
- `[ESC]` Return from details

You can also run `sysmon --help` to view CLI options.

SYSMON looks for configuration files in the following order:
1. Command-line arguments (`--interval 500`, etc.)
2. `~/.config/sysmon/config`
3. `/etc/sysmon/default.conf`

See the [default configuration file](config/default.conf) for all available options.

## Development Roadmap

The project is currently under active development. We are building it in 9 distinct phases. Check out the [Build Roadmap](docs/roadmap.md) to see our progress and upcoming milestones.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
