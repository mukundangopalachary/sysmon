# Sysmon

**Sysmon** is a highly optimized, lightning-fast interactive system resource monitor and command-line management tool built in C and C++. It features an interactive Terminal UI (TUI) powered by `ncurses`, real-time process monitoring, customizable dashboards, and a fully extensible bash script plugin system!

---

## Features

- **Interactive TUI**: Navigate between Dashboard, Process List, and Connections screens seamlessly.
- **Lightning Fast**: Built in C/C++ with a multithreaded architecture. Minimal CPU overhead.
- **Double-Buffered State**: Guarantees tear-free rendering while asynchronous data collection runs in the background.
- **Global Search & Filter**: Instantly filter processes or network connections on the fly (press `/`).
- **Pause & Resume**: Stop the background data collection at any time to inspect volatile processes without them jumping around (press `p`).
- **Customizable Themes**: Supports dynamic theme swapping (`dracula`, `default`, etc.) via TOML configurations.
- **Plugin System**: Write plugins in standard Bash scripts. Integrate things like Docker stats, Network latency, or GPU loads directly into your sysmon dashboard!

---

##Installation

### 1. Pre-compiled Binaries (Easiest)

You can download the pre-compiled binary directly using `curl` and extract it:

```bash
curl -LO https://github.com/mukundangopalachary/sysmon/releases/download/v2.0.1/sysmon-linux-x86_64.tar.gz
tar -xzvf sysmon-linux-x86_64.tar.gz
sudo mv sysmon sysmon-cli /usr/local/bin/
```

### 2. Build from Source

#### Prerequisites
- CMake (3.15+)
- GCC/G++ or Clang (C11 and C++17 support)
- Ncurses Library (`libncurses-dev` or `ncurses-devel`)

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/sysmon.git
   cd sysmon
   ```

2. Build the application using CMake:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

3. Install the application and default configs (Optional):
   ```bash
   sudo make install
   ```

*(Alternatively, you can just run `make` in the root directory if you want to use the included Makefile wrapper).*

---

## Usage

Sysmon acts as a smart "fat binary". It can launch both the visual dashboard (TUI) and run headless administrative commands (CLI).

### Launching the Dashboard (TUI)

Just run `sysmon` with no arguments:
```bash
sysmon
```

**Interactive Keybindings:**
- `F1` : Dashboard View
- `F2` : Process List View
- `F3` : Active Connections View
- `F4` : Plugins View
- `p`  : Pause/Resume data collection
- `/`  : Enter Search/Filter mode inside tables
- `ENTER` : Inspect detailed metrics for a specific process
- `ESC`: Return to the previous screen or clear a search filter
- `q`  : Quit Sysmon

### Using the CLI

You do not need to manually edit `.toml` files to install or manage plugins! You can do everything directly from your terminal using the built-in CLI:

```bash
# First, pull the latest plugin index from the remote registry
sysmon-cli registry update

# Search for available plugins
sysmon-cli plugin search docker

# Install a plugin
sysmon-cli plugin install docker-stats

# Enable or Disable an installed plugin
sysmon-cli plugin enable docker-stats
sysmon-cli plugin disable docker-stats

# List all your currently installed plugins
sysmon-cli plugin list
```

---

##Configuration

Sysmon uses TOML for configuration. After installation, your config file is located at `~/.config/sysmon/sysmon.toml`.

Example `sysmon.toml`:
```toml
[collection]
interval_ms = 1000

[display]
theme = "dracula"
refresh_rate_hz = 30

[keybindings]
dashboard = "F1"
process_list = "F2"

[plugins]
auto_update = true
repos = [
    "github.com/sysmon-org/docker-stats",
]
```

---

## Testing

To run the internal CTest suite to verify parsers and memory safety:

```bash
cd build
cmake ..
make
ctest --output-on-failure
```

---

## License

Sysmon is open-sourced software licensed under the MIT license.
