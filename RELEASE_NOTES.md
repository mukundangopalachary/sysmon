# Release Notes

## Sysmon v1.0.0

**Release Date:** June 2026

We are incredibly proud to announce the stable `v1.0.0` release of **Sysmon**! This marks the culmination of a highly structured, 9-phase development roadmap. Sysmon is designed to be the ultimate terminal-based system monitoring tool, combining the raw speed of a C data engine with the interactivity of a C++ `ncurses` UI.

### Key Features
- **Atomic C Core Engine**: A lock-free, double-buffered architecture ensures that background data collection from `/proc` never blocks the main UI thread. 
- **100Hz Event Loop**: The TUI runs smoothly, allowing for instant feedback and rapid sorting of processes and network connections.
- **Dynamic Dashboard**: Gain an immediate visual overview of CPU, Memory, Disk I/O, and Network Throughput metrics.
- **Process Drill-Down**: An interactive vim-style process table (`j`/`k` navigation). Hitting `ENTER` on any process brings up a detailed pane covering advanced memory statistics and command-line execution parameters.
- **Network Sockets View**: A fully integrated TCP/UDP connection auditor to inspect local-to-remote routing in real-time.
- **Bash Plugin Subsystem**: Sysmon isn't strictly limited to system defaults. By placing executable scripts in the `scripts/plugins/` directory, users can easily inject custom hardware sensors (like GPU or temperature metrics) directly into the UI.

### Installation Options
- Source compilation via CMake.
- Available pre-compiled statically-linked binaries attached directly to the GitHub release page.
- Full support for `sudo make install` to enable global command-line execution from anywhere.

Thank you to everyone who supported this project. We look forward to community plugins and feature extensions in the `v1.1` cycle!
