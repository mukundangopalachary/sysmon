## SYSMON - System Resource Monitor

### Table of Contents
1. [Architecture Overview](#1-architecture-overview)
2. [Data Model](#2-data-model)
3. [Component Design](#3-component-design)
4. [Collection Engine](#4-collection-engine)
5. [Snapshot Manager](#5-snapshot-manager)
6. [TUI Framework](#6-tui-framework)
7. [Plugin System](#7-plugin-system)
8. [Build System](#8-build-system)
9. [Configuration](#9-configuration)
10. [API Reference](#10-api-reference)

---

## 1. Architecture Overview

### Design Philosophy

```
┌─────────────────────────────────────────────────────────────┐
│                     ARCHITECTURE PRINCIPLES                  │
├─────────────────────────────────────────────────────────────┤
│ 1. Immutable Snapshots - Data frozen at collection time     │
│ 2. Lock-Free Reads - UI never blocks on data                │
│ 3. Single Writer - One collection thread, many readers      │
│ 4. Contiguous Memory - Arrays over linked lists             │
│ 5. Pre-Computed Values - Calculate once, display many times │
│ 6. Lazy Collection - Only fetch what's being viewed         │
└─────────────────────────────────────────────────────────────┘
```

### High-Level Data Flow

```
/proc/* ──→ [Collection Engine] ──→ [Snapshot N+1] (mutable, private)
                                       │
                                  atomic swap
                                       │
                                       v
                                  [Snapshot N] (immutable, shared)
                                       │
                    ┌──────────────────┼──────────────────┐
                    │                  │                  │
                    v                  v                  v
              [CPU Panel]      [Memory Panel]      [Process Panel]
              (reads only)     (reads only)       (reads only)
                    │                  │                  │
                    └──────────────────┼──────────────────┘
                                       │
                                       v
                               [TUI Render Engine]
                                       │
                                       v
                                  [Terminal]
```

### Thread Model

```
Thread 1: Collection Engine (C)
  - Runs every 1000ms (configurable)
  - Reads /proc, /sys, /proc/*/stat
  - Builds next snapshot
  - Atomic swap when complete
  - Never touches UI

Thread 2: TUI Event Loop (C++)
  - Runs at display refresh rate (~60fps if needed)
  - Reads current snapshot (immutable)
  - Handles keyboard/mouse input
  - Triggers re-renders
  - Never waits for collection

Thread 3: Plugin Executor (optional, Bash)
  - Runs async, independent timing
  - Produces plugin-specific data files
  - Signals collection engine when ready
```

### Why C for Collection, C++ for UI

```
┌─────────────────────┬──────────────────────────────────────┐
│ C (Collection)      │ C++ (UI)                             │
├─────────────────────┼──────────────────────────────────────┤
│ Raw /proc parsing   │ Component hierarchy                  │
│ No exceptions       │ RAII for ncurses windows             │
│ Fixed-size arrays   │ std::vector for display lists        │
│ No STL dependency   │ std::string for user input           │
│ Direct syscalls     │ Virtual dispatch for panels          │
│ Predictable memory  │ Rich standard library                │
└─────────────────────┴──────────────────────────────────────┘
```

---

## 2. Data Model

### Core Types

```c
/*
 * METRIC CLASSIFICATION
 * 
 * Counter:  Monotonically increasing value. Rate = (current - previous) / delta_t
 * Gauge:    Instantaneous value. No history needed.
 * Table:    Collection of structured rows. Sortable, filterable.
 * Info:     Static system information. Collected once.
 */

typedef enum {
    METRIC_CLASS_COUNTER,
    METRIC_CLASS_GAUGE,
    METRIC_CLASS_TABLE,
    METRIC_CLASS_INFO
} MetricClass;
```

### SystemInfo Structure

```c
/*
 * Collected once at startup, updated on change detection
 * Size: ~512 bytes
 * Read frequency: Once + on SIGWINCH/resume
 */
typedef struct {
    char hostname[256];
    char kernel_release[256];
    char kernel_version[256];
    char architecture[32];
    uint64_t total_memory_bytes;
    uint64_t total_swap_bytes;
    uint64_t page_size;
    int64_t clock_ticks_per_sec;    // sysconf(_SC_CLK_TCK)
    int num_cpu_cores;
    int num_cpu_sockets;
    uint64_t boot_time_epoch;
} SystemInfo;
```

### CPU Snapshot

```c
/*
 * Per-core CPU times
 * All values in clock ticks (convert via clock_ticks_per_sec)
 * CPU% = delta(active_ticks) / delta(total_ticks) * 100
 */
typedef struct {
    uint64_t user;
    uint64_t nice;
    uint64_t system;
    uint64_t idle;
    uint64_t iowait;
    uint64_t irq;
    uint64_t softirq;
    uint64_t steal;
    uint64_t guest;
    uint64_t guest_nice;
    
    /* Pre-computed during collection */
    uint64_t total_ticks;           // Sum of all above
    uint64_t active_ticks;          // total - idle - iowait
    double usage_percent;           // Calculated from previous snapshot
} CpuCoreSnapshot;

typedef struct {
    uint64_t timestamp_us;
    int num_cores;
    CpuCoreSnapshot cores[256];     // Supports up to 256 logical cores
    double load_avg_1min;
    double load_avg_5min;
    double load_avg_15min;
} CpuSnapshot;
```

### Memory Snapshot

```c
/*
 * All values in bytes (converted from /proc/meminfo kB)
 * Usage% = (total - available) / total * 100
 */
typedef struct {
    /* Main metrics */
    uint64_t total_bytes;
    uint64_t available_bytes;       // MemAvailable - best estimate
    uint64_t free_bytes;            // MemFree - actually free
    uint64_t used_bytes;            // total - available
    double usage_percent;
    
    /* Breakdown */
    uint64_t buffers_bytes;
    uint64_t cached_bytes;
    uint64_t shared_bytes;
    uint64_t slab_bytes;
    uint64_t kernel_stack_bytes;
    uint64_t page_tables_bytes;
    
    /* Swap */
    uint64_t swap_total_bytes;
    uint64_t swap_free_bytes;
    uint64_t swap_used_bytes;
    double swap_usage_percent;
    
    /* Huge pages */
    uint64_t hugepages_total_bytes;
    uint64_t hugepages_free_bytes;
} MemorySnapshot;
```

### Network Snapshot

```c
/*
 * Per-interface statistics
 * Rate = delta(bytes) / delta(time)
 */
typedef struct {
    char name[32];                  // eth0, wlan0, etc.
    uint64_t rx_bytes;
    uint64_t rx_packets;
    uint64_t rx_errors;
    uint64_t rx_dropped;
    uint64_t tx_bytes;
    uint64_t tx_packets;
    uint64_t tx_errors;
    uint64_t tx_dropped;
    
    /* Pre-computed rates (requires previous snapshot) */
    double rx_bytes_per_sec;
    double tx_bytes_per_sec;
    double rx_packets_per_sec;
    double tx_packets_per_sec;
    
    bool is_up;
    bool is_loopback;
} NetworkIfaceSnapshot;

typedef struct {
    int num_interfaces;
    NetworkIfaceSnapshot interfaces[32];  // Typical system has <32
} NetworkSnapshot;
```

### Process Snapshot

```c
/*
 * THE HEAVY STRUCTURE
 * 
 * Design decisions:
 * - Fixed-size comm[256]: no malloc per process for name
 * - username pre-resolved: UI doesn't call getpwuid()
 * - cpu_percent pre-calculated: expensive delta math done once
 * - ppid included: tree view built by C++ without re-reading /proc
 * 
 * Memory impact: ~400 bytes per process
 * 1000 processes = ~400KB (trivial for modern systems)
 */
typedef struct {
    int pid;
    int ppid;                       // For tree reconstruction
    int pgrp;
    int session;
    int tty_nr;
    
    char comm[256];                 // Process name (truncated from /proc/*/comm)
    char state;                     // R, S, D, Z, T, etc.
    uid_t uid;
    char username[32];              // Pre-resolved, empty if resolution failed
    
    /* CPU */
    uint64_t utime_ticks;           // User time
    uint64_t stime_ticks;           // System time
    uint64_t starttime_ticks;       // Process start time
    double cpu_percent;             // Pre-calculated CPU usage %
    
    /* Memory */
    uint64_t rss_bytes;             // Resident set size
    uint64_t vsize_bytes;           // Virtual memory size
    double mem_percent;             // % of total system memory
    
    /* Scheduling */
    int nice;
    int num_threads;
    uint64_t wchan;                 // Wait channel
    
    /* I/O (optional, only if /proc/*/io is readable) */
    uint64_t io_read_bytes;
    uint64_t io_write_bytes;
    
    /* Display metadata */
    bool is_kernel_thread;          // cmdline is empty
    bool is_zombie;                 // state == 'Z'
    
} ProcessSnapshot;

typedef struct {
    int num_processes;
    int capacity;                   // Allocated size of array
    ProcessSnapshot* processes;     // Contiguous array, sorted by PID
    
    /* Sorting indices built on demand by C++ */
    // Not stored here - UI creates sorted views
} ProcessTableSnapshot;
```

### Connection Snapshot

```c
/*
 * Collected only when connections view is active
 * Parsed from /proc/net/tcp, /proc/net/udp, etc.
 */
typedef enum {
    CONN_TCP,
    CONN_TCP6,
    CONN_UDP,
    CONN_UDP6,
    CONN_UNIX
} ConnectionType;

typedef enum {
    CONN_ESTABLISHED,
    CONN_LISTEN,
    CONN_TIME_WAIT,
    CONN_CLOSE_WAIT,
    CONN_SYN_SENT,
    CONN_OTHER
} ConnectionState;

typedef struct {
    ConnectionType type;
    ConnectionState state;
    
    char local_addr[64];            // IP:port as string
    char remote_addr[64];
    
    int pid;                        // Owning process PID (may be -1 if unknown)
    int uid;
    uint64_t inode;
    
    uint32_t rx_queue;
    uint32_t tx_queue;
} ConnectionSnapshot;

typedef struct {
    int num_connections;
    int capacity;
    ConnectionSnapshot* connections;
} ConnectionTableSnapshot;
```

### Disk Snapshot

```c
typedef struct {
    char device_name[64];           // nvme0n1, sda, etc.
    uint64_t read_sectors;
    uint64_t write_sectors;
    uint64_t read_ios;
    uint64_t write_ios;
    uint64_t read_ticks_ms;
    uint64_t write_ticks_ms;
    uint64_t io_ticks_ms;
    
    /* Pre-computed rates */
    double read_bytes_per_sec;
    double write_bytes_per_sec;
    double iops;
} DiskStatsSnapshot;

typedef struct {
    int num_devices;
    DiskStatsSnapshot devices[32];
} DiskSnapshot;
```

### Filesystem Snapshot

```c
/*
 * Collected less frequently (every 30s)
 * Mount points that don't change often
 */
typedef struct {
    char device[256];
    char mount_point[512];
    char fs_type[64];
    uint64_t total_bytes;
    uint64_t used_bytes;
    uint64_t available_bytes;
    double usage_percent;
    uint64_t inodes_total;
    uint64_t inodes_used;
} FilesystemSnapshot;

typedef struct {
    int num_filesystems;
    int capacity;
    FilesystemSnapshot* filesystems;
} FilesystemTableSnapshot;
```

### The Master Snapshot

```c
/*
 * SYSTEM SNAPSHOT - The Immutable Truth
 * 
 * This is the single data structure passed to all UI components.
 * Once published, it never changes until the next collection cycle.
 * 
 * Total size estimate (1000 processes, 100 connections):
 *   SystemInfo:     512 bytes
 *   CpuSnapshot:    ~8KB (256 cores * 32 bytes)
 *   MemorySnapshot: 128 bytes
 *   NetworkSnapshot:~4KB (32 interfaces * 128 bytes)
 *   ProcessTable:   ~400KB (1000 * 400 bytes)
 *   Connections:    ~20KB (100 * 200 bytes)
 *   DiskSnapshot:   ~4KB (32 devices * 128 bytes)
 *   Filesystems:    ~2KB (10 mounts * 200 bytes)
 *   ──────────────────────
 *   TOTAL:          ~440KB
 * 
 * At 1 second intervals, this is negligible memory pressure.
 */
typedef struct {
    /* Metadata */
    uint64_t collection_timestamp_us;
    uint64_t collection_sequence;   // Monotonically increasing
    uint64_t collection_duration_us;// How long did collection take?
    
    /* Sub-snapshots */
    SystemInfo* system_info;        // Pointer (collected once, shared across cycles)
    CpuSnapshot cpu;
    MemorySnapshot memory;
    NetworkSnapshot network;
    ProcessTableSnapshot processes;
    
    /* Optional sub-snapshots (NULL if not being viewed) */
    ConnectionTableSnapshot* connections;
    DiskSnapshot* disk;
    FilesystemTableSnapshot* filesystems;
    
    /* Plugin data (optional) */
    void* plugin_data;              // Opaque, cast by plugin manager
    
} SystemSnapshot;
```

### State Machine for Snapshot Lifecycle

```
                    ┌──────────────┐
                    │  UNINITIALIZED│
                    └──────┬───────┘
                           │ snapshot_init()
                           v
                    ┌──────────────┐
         ┌─────────│   BUILDING   │◄──────────────┐
         │         └──────┬───────┘               │
         │                │ collection complete    │
         │                v                        │
         │         ┌──────────────┐               │
         │         │  PUBLISHED   │───────────────┘
         │         │  (immutable) │  swap happens,
         │         └──────┬───────┘  this becomes
         │                │          "next" buffer
         │                │
         │                v
         │         ┌──────────────┐
         │         │  RETIRING    │
         │         └──────┬───────┘
         │                │ snapshot_reset()
         │                v
         │         ┌──────────────┐
         └─────────│   BUILDING   │
                   └──────────────┘
```

---

## 3. Component Design

### Component Tree (C++ Side)

```
Application (app.cpp)
├── ScreenManager
│   ├── DashboardScreen
│   │   ├── HeaderPanel          (hostname, uptime, load)
│   │   ├── CpuPanel             (bars + per-core)
│   │   ├── MemoryPanel          (gauge + breakdown)
│   │   ├── NetworkPanel         (interfaces + throughput)
│   │   └── DiskPanel            (I/O rates)
│   ├── ProcessListScreen
│   │   └── ProcessTablePanel    (sortable, filterable)
│   ├── ConnectionScreen
│   │   └── ConnectionTablePanel
│   └── ProcessDetailScreen
│       └── ProcessDetailPanel   (single process deep dive)
├── InputHandler                 (keyboard, mouse, resize)
├── ThemeManager                 (colors, styles)
└── PluginPanel                  (external Bash plugin display)
```

### Panel Base Class

```cpp
/*
 * Abstract base for all TUI panels
 * 
 * Each panel:
 * - Receives a const snapshot reference (never modifies)
 * - Owns a WINDOW* (ncurses)
 * - Handles its own drawing within bounds
 * - Can receive focus for keyboard input
 * - Supports resize events
 */
class Panel {
public:
    Panel(int height, int width, int start_y, int start_x);
    virtual ~Panel();
    
    // Pure virtual - every panel must implement
    virtual void render(const SystemSnapshot* snapshot) = 0;
    
    // Optional overrides
    virtual void on_focus() {}
    virtual void on_blur() {}
    virtual void on_resize(int height, int width, int start_y, int start_x);
    virtual bool handle_input(int key);  // Return true if consumed
    
    // Common utilities
    void draw_border(bool focused = false);
    void draw_title(const char* title);
    void clear_content();
    
protected:
    WINDOW* window;
    int height, width, start_y, start_x;
    bool has_focus;
};
```

### Screen Base Class

```cpp
/*
 * A Screen is a full-terminal view composed of Panels
 * Only one screen is active at a time
 */
class Screen {
public:
    virtual ~Screen();
    
    virtual void on_enter();                    // Called when becoming active
    virtual void on_exit();                     // Called when leaving
    virtual void render(const SystemSnapshot* snapshot) = 0;
    virtual bool handle_input(int key) = 0;     // Return true if consumed
    virtual void on_resize();
    
    void set_visible(bool visible);
    bool is_visible() const;
    
protected:
    std::vector<std::unique_ptr<Panel>> panels;
    bool visible;
};
```

---

## 4. Collection Engine

### Design

```c
/*
 * COLLECTION ENGINE
 * 
 * Single-threaded, runs in a loop with configurable interval.
 * Each iteration produces one complete SystemSnapshot.
 * 
 * Collection order matters:
 * 1. Read all /proc files (minimize time between first and last read)
 * 2. Parse and compute derived values
 * 3. Publish snapshot atomically
 * 
 * Error handling:
 * - Missing /proc entries → zero-fill, log warning
 * - Permission denied → skip, leave blank
 * - Malformed data → skip entry, log error
 * - /proc not mounted → fatal, exit with message
 */

typedef struct CollectionEngine CollectionEngine;

struct CollectionEngine {
    /* Configuration */
    int collection_interval_ms;     // Default: 1000
    int process_max_count;          // Default: 10000
    bool collect_connections;       // Default: false (lazy)
    bool collect_disk_io;           // Default: true
    
    /* State */
    bool running;
    pthread_t thread;
    SnapshotManager* snapshot_mgr;
    
    /* Previous snapshot for rate calculations */
    CpuCoreSnapshot* prev_cpu;      // Per-core previous times
    NetworkIfaceSnapshot* prev_net; // Per-interface previous bytes
    uint64_t prev_timestamp_us;
    
    /* Callbacks */
    void (*on_collection_start)(void);
    void (*on_collection_complete)(SystemSnapshot* snapshot);
    void (*on_collection_error)(const char* error_msg);
};

/* Public API */
int collection_engine_init(CollectionEngine* engine, SnapshotManager* mgr);
int collection_engine_start(CollectionEngine* engine);
int collection_engine_stop(CollectionEngine* engine);
void collection_engine_destroy(CollectionEngine* engine);
```

### Collection Cycle Pseudocode

```c
void collection_cycle(CollectionEngine* engine) {
    uint64_t cycle_start = get_time_us();
    
    SystemSnapshot* snap = snapshot_get_next_buffer(engine->snapshot_mgr);
    
    /* Phase 1: Raw data gathering (fast, minimal processing) */
    uint64_t t1 = get_time_us();
    read_proc_stat(snap);           // CPU times, boot time
    read_proc_loadavg(snap);        // Load averages
    read_proc_meminfo(snap);        // Memory details
    read_proc_net_dev(snap);        // Network interfaces
    uint64_t t2 = get_time_us();
    
    /* Phase 2: Expensive operations */
    read_all_processes(snap);       // Iterate /proc/*/stat
    if (engine->collect_connections) {
        read_connections(snap);     // /proc/net/tcp, etc.
    }
    if (engine->collect_disk_io) {
        read_disk_stats(snap);      // /proc/diskstats
    }
    uint64_t t3 = get_time_us();
    
    /* Phase 3: Compute derived values */
    compute_cpu_percentages(snap, engine->prev_cpu, engine->prev_timestamp_us);
    compute_network_rates(snap, engine->prev_net, engine->prev_timestamp_us);
    compute_process_cpu_percentages(snap);
    resolve_usernames(snap);        // getpwuid() calls
    
    /* Store previous state for next cycle */
    memcpy(engine->prev_cpu, snap->cpu.cores, sizeof(CpuCoreSnapshot) * snap->cpu.num_cores);
    memcpy(engine->prev_net, snap->network.interfaces, sizeof(NetworkIfaceSnapshot) * snap->network.num_interfaces);
    engine->prev_timestamp_us = cycle_start;
    
    uint64_t t4 = get_time_us();
    
    /* Phase 4: Metadata */
    snap->collection_timestamp_us = cycle_start;
    snap->collection_duration_us = t4 - cycle_start;
    snap->collection_sequence++;
    
    /* Publish */
    snapshot_publish(engine->snapshot_mgr);
    
    /* Timing report (debug) */
    /*
     * Phase 1 (raw read):   t2-t1  (~100-500us)
     * Phase 2 (processes):  t3-t2  (~5-50ms for 500 procs)
     * Phase 3 (compute):    t4-t3  (~1-10ms)
     * Total:                t4-t1  (~6-60ms)
     */
}
```

---

## 5. Snapshot Manager

### Design

```c
/*
 * SNAPSHOT MANAGER
 * 
 * Manages the double-buffer and atomic swapping.
 * 
 * Memory strategy:
 * - Two SystemSnapshot structs allocated at init
 * - Process arrays grow to max seen process count and stay there
 * - Connection arrays grow on demand
 * - SystemInfo allocated once, shared between both buffers
 * - Memory is never freed during operation (only at shutdown)
 * 
 * Thread safety:
 * - publish() uses atomic exchange
 * - get_current() is lock-free read
 * - Only collection thread modifies the "next" buffer
 */
typedef struct SnapshotManager SnapshotManager;

struct SnapshotManager {
    SystemSnapshot* buffers[2];     // Double buffer
    int current_index;              // 0 or 1, which buffer is "current"
    SystemInfo* shared_system_info; // Shared between both buffers
    
    /* Statistics */
    uint64_t total_swaps;
    uint64_t total_collections;
    uint64_t max_process_count;
    uint64_t max_connection_count;
    uint64_t max_collection_time_us;
};

/* Public API */
int snapshot_manager_init(SnapshotManager* mgr);
void snapshot_manager_destroy(SnapshotManager* mgr);

/*
 * Get current immutable snapshot for reading.
 * Returns NULL if no snapshot has been published yet.
 * Thread-safe: can be called from any thread without locks.
 */
const SystemSnapshot* snapshot_manager_get_current(SnapshotManager* mgr);

/*
 * Get the "next" buffer for the collection engine to fill.
 * Only call from collection thread.
 * Returns mutable pointer - this buffer is NOT visible to readers.
 */
SystemSnapshot* snapshot_manager_get_next(SnapshotManager* mgr);

/*
 * Publish the completed "next" buffer.
 * Atomic swap makes it the new "current".
 * Only call from collection thread.
 */
void snapshot_manager_publish(SnapshotManager* mgr);
```

### Implementation Details

```c
void snapshot_manager_publish(SnapshotManager* mgr) {
    int next_index = 1 - mgr->current_index;
    
    /* 
     * Atomic store with release semantics.
     * Ensures all writes to the snapshot data are visible
     * to any thread that reads current_index with acquire semantics.
     */
    __atomic_store(&mgr->current_index, next_index, __ATOMIC_RELEASE);
    
    mgr->total_swaps++;
}

const SystemSnapshot* snapshot_manager_get_current(SnapshotManager* mgr) {
    /* 
     * Atomic load with acquire semantics.
     * Ensures we see all the writes that happened before the swap.
     */
    int idx = __atomic_load(&mgr->current_index, __ATOMIC_ACQUIRE);
    
    SystemSnapshot* snap = mgr->buffers[idx];
    
    /* Snapshots start with collection_sequence = 0.
     * If it's 0, no collection has happened yet. */
    if (snap->collection_sequence == 0) {
        return NULL;
    }
    
    return snap;
}
```

---

## 6. TUI Framework

### Screen Navigation

```
                    ┌─────────────┐
                    │  DASHBOARD  │ ◄──── Default on launch
                    └──┬──┬──┬───┘
                       │  │  │
            F5         │  │  │  F6
            ┌──────────┘  │  └──────────┐
            v             │             v
    ┌───────────────┐    │    ┌──────────────────┐
    │ PROCESS LIST  │    │    │ NET CONNECTIONS  │
    └───────┬───────┘    │    └──────────────────┘
            │            │
            │ Enter      │
            v            │
    ┌────────────────┐  │
    │ PROCESS DETAIL  │  │
    └────────────────┘  │
                        │
              Esc/Del   │
              └─────────┘
```

### Key Bindings

```
┌────────────────────┬──────────────────────────────────────┐
│ GLOBAL             │                                      │
├────────────────────┼──────────────────────────────────────┤
│ F1                 │ Help screen                          │
│ F2                 │ Settings/configuration               │
│ F5                 │ Switch to Process List               │
│ F6                 │ Switch to Network Connections        │
│ F7                 │ Switch to Dashboard                  │
│ q / Ctrl+C         │ Quit                                 │
│ :                  │ Command mode                         │
│ /                  │ Search/Filter mode                   │
│ r                  │ Force refresh                        │
│ p                  │ Pause/resume collection              │
│                    │                                      │
├────────────────────┼──────────────────────────────────────┤
│ PROCESS LIST       │                                      │
├────────────────────┼──────────────────────────────────────┤
│ ↑/↓ or j/k        │ Move selection                       │
│ PgUp/PgDn         │ Page scroll                          │
│ Home/End          │ Top/bottom of list                   │
│ Enter             │ View process details                 │
│ F9                │ Send SIGKILL to selected process     │
│ F10               │ Send SIGTERM to selected process     │
│ s                 │ Change sort column                   │
│ R                 │ Reverse sort order                   │
│ t                 │ Toggle tree view                     │
│ /                 │ Filter by name                       │
│ u                 │ Filter by user                       │
│                    │                                      │
├────────────────────┼──────────────────────────────────────┤
│ PROCESS DETAIL     │                                      │
├────────────────────┼──────────────────────────────────────┤
│ Esc / Backspace   │ Back to process list                 │
│ c                 │ Copy command line to clipboard       │
│ k                 │ Send SIGTERM                         │
│ K                 │ Send SIGKILL                         │
└────────────────────┴──────────────────────────────────────┘
```

---

## 7. Plugin System

### Design Philosophy

```
Bash plugins are FIRST-CLASS CITIZENS, not afterthoughts.

Each plugin is a single executable script that:
1. Receives configuration via stdin (JSON)
2. Outputs metrics to stdout (simple line protocol)
3. Exits with code 0 on success
4. Can run at its own interval (different from main collection)
```

### Plugin Protocol

```
PLUGIN OUTPUT FORMAT (one line per metric):

TYPE:KEY=VALUE[:UNIT]

Examples:
gauge:gpu.temperature=67:C
gauge:gpu.memory_used=2147483648:bytes
gauge:battery.percentage=85:%
counter:fan.rpm=3200
info:docker.containers_running=5

Multi-value output (for tables):
table:docker.containers
col:container_id,container_name,cpu_percent,memory_bytes
row:abc123,web_server,2.5,134217728
row:def456,database,12.1,536870912
endtable

Error handling:
error:Failed to read GPU sensor: permission denied

Metadata:
interval:2000  (suggested collection interval in ms)
version:1.0
```

### Plugin Contract

```bash
#!/bin/bash
# PLUGIN: GPU Monitor
# DESCRIPTION: Monitors NVIDIA GPU metrics using nvidia-smi
# INTERVAL: 2000ms (suggestion)
# REQUIRES: nvidia-smi
# VERSION: 1.0.0

# Plugin receives config via stdin
read -r config_json

# Parse config if needed
refresh_interval=$(echo "$config_json" | jq -r '.interval // 2000')

# Output metrics to stdout
output_metrics() {
    if ! command -v nvidia-smi &>/dev/null; then
        echo "error:nvidia-smi not found"
        exit 1
    fi
    
    # Temperature
    temp=$(nvidia-smi --query-gpu=temperature.gpu --format=csv,noheader 2>/dev/null)
    if [[ -n "$temp" ]]; then
        echo "gauge:gpu.temperature=$temp:C"
    fi
    
    # Memory
    mem_used=$(nvidia-smi --query-gpu=memory.used --format=csv,noheader 2>/dev/null | sed 's/ MiB//')
    echo "gauge:gpu.memory_used=$((mem_used * 1048576)):bytes"
    
    # Utilization
    util=$(nvidia-smi --query-gpu=utilization.gpu --format=csv,noheader 2>/dev/null | sed 's/ %//')
    echo "gauge:gpu.utilization=$util:%"
}

# Main loop (runs until killed)
while true; do
    output_metrics
    sleep "$((refresh_interval / 1000)).$((refresh_interval % 1000))"
done
```

### Plugin Manager (C side)

```c
/*
 * PLUGIN MANAGER
 * 
 * Manages lifecycle of Bash plugins.
 * Each plugin runs as a child process.
 * Manager reads plugin stdout, parses protocol, stores in snapshot.
 */
typedef struct PluginManager PluginManager;

typedef struct {
    char name[64];
    char path[512];                 // Path to script
    pid_t pid;
    int stdout_fd;                  // Pipe for reading plugin output
    int interval_ms;
    bool running;
    bool enabled;
} PluginInstance;

struct PluginManager {
    int num_plugins;
    PluginInstance plugins[32];     // Max 32 plugins
    
    /* Plugin data for current snapshot */
    void* plugin_metrics;           // Parsed metrics, opaque to core
};

/* Public API */
int plugin_manager_init(PluginManager* mgr, const char* plugin_dir);
int plugin_manager_load_plugins(PluginManager* mgr);
int plugin_manager_start_all(PluginManager* mgr);
int plugin_manager_stop_all(PluginManager* mgr);
int plugin_manager_collect(PluginManager* mgr);  // Non-blocking read of all plugin pipes
```

---

## 8. Build System

### Makefile

```makefile
# SYSMON Makefile
# Quick build + common tasks
# For full build, CMakeLists.txt is the source of truth

.PHONY: all build debug release clean install uninstall run test

# Default target
all: build

# Build using CMake
build:
	@mkdir -p build
	@cd build && cmake .. -DCMAKE_BUILD_TYPE=Release
	@cd build && make -j$(nproc)

debug:
	@mkdir -p build
	@cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
	@cd build && make -j$(nproc)

release:
	@mkdir -p build
	@cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_LTO=ON
	@cd build && make -j$(nproc)

# Install
install: build
	@cd build && make install

uninstall:
	@cd build && make uninstall 2>/dev/null || true

# Run
run: build
	@./build/sysmon

# Test
test: debug
	@cd build && ctest --output-on-failure

# Clean
clean:
	@rm -rf build

# Format code
format:
	@clang-format -i src/core/*.c src/core/*.h
	@clang-format -i src/tui/**/*.cpp src/tui/**/*.h
	@echo "Code formatted"

# Static analysis
analyze:
	@mkdir -p build
	@cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug
	@cd build && scan-build make

# Install plugins to user directory
install-plugins:
	@mkdir -p ~/.local/share/sysmon/plugins
	@cp scripts/plugins/*.sh ~/.local/share/sysmon/plugins/
	@chmod +x ~/.local/share/sysmon/plugins/*.sh
	@echo "Plugins installed to ~/.local/share/sysmon/plugins/"

# Generate tags for IDE
tags:
	@ctags -R --languages=C,C++ src/
	@echo "Tags generated"

# Help
help:
	@echo "SYSMON Build System"
	@echo ""
	@echo "Targets:"
	@echo "  build          - Release build"
	@echo "  debug          - Debug build with sanitizers"
	@echo "  release        - Optimized release build"
	@echo "  run            - Build and run"
	@echo "  test           - Build and run tests"
	@echo "  clean          - Remove build directory"
	@echo "  install        - Install to system"
	@echo "  format         - Format source code"
	@echo "  analyze        - Run static analysis"
	@echo "  install-plugins - Install Bash plugins"
	@echo "  tags           - Generate ctags"
```

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(sysmon 
    VERSION 1.0.0
    DESCRIPTION "Terminal System Resource Monitor"
    LANGUAGES C CXX
)

# ── Project Options ──────────────────────────────────────────
option(ENABLE_SANITIZERS "Enable address/undefined sanitizers" OFF)
option(ENABLE_LTO "Enable link-time optimization" OFF)
option(BUILD_TESTS "Build test suite" ON)
option(INSTALL_PLUGINS "Install Bash plugins" ON)

# ── Standards ─────────────────────────────────────────────────
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Compiler Flags ────────────────────────────────────────────
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

if(ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

if(ENABLE_LTO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# ── Dependencies ──────────────────────────────────────────────
find_package(Curses REQUIRED)
find_package(Threads REQUIRED)

# Optional: nlohmann_json for plugin config parsing
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
    add_compile_definitions(HAVE_NLOHMANN_JSON)
endif()

# ── Source Files ──────────────────────────────────────────────
# C Core Library
set(CORE_SOURCES
    src/core/cpu_stats.c
    src/core/mem_stats.c
    src/core/net_stats.c
    src/core/disk_stats.c
    src/core/process_list.c
    src/core/proc_reader.c
    src/core/sysinfo.c
    src/core/snapshot_manager.c
    src/core/collection_engine.c
    src/core/plugin_manager.c
    src/core/username_cache.c
)

set(CORE_HEADERS
    src/core/cpu_stats.h
    src/core/mem_stats.h
    src/core/net_stats.h
    src/core/disk_stats.h
    src/core/process_list.h
    src/core/proc_reader.h
    src/core/sysinfo.h
    src/core/snapshot_manager.h
    src/core/collection_engine.h
    src/core/plugin_manager.h
    src/core/username_cache.h
    src/bridge/sysmon_bridge.h
    src/core/types.h
)

# C++ TUI Library
set(TUI_SOURCES
    src/tui/app.cpp
    src/tui/event_loop.cpp
    src/tui/theme.cpp
    src/tui/input_handler.cpp
    src/tui/screen_manager.cpp
    src/tui/components/header.cpp
    src/tui/components/cpu_panel.cpp
    src/tui/components/memory_panel.cpp
    src/tui/components/network_panel.cpp
    src/tui/components/disk_panel.cpp
    src/tui/components/process_table.cpp
    src/tui/components/connection_table.cpp
    src/tui/components/process_detail.cpp
    src/tui/screens/dashboard_screen.cpp
    src/tui/screens/process_list_screen.cpp
    src/tui/screens/connection_screen.cpp
    src/tui/screens/process_detail_screen.cpp
    src/tui/screens/help_screen.cpp
)

set(TUI_HEADERS
    src/tui/app.h
    src/tui/event_loop.h
    src/tui/theme.h
    src/tui/input_handler.h
    src/tui/screen_manager.h
    src/tui/components/panel.h
    src/tui/components/header.h
    src/tui/components/cpu_panel.h
    src/tui/components/memory_panel.h
    src/tui/components/network_panel.h
    src/tui/components/disk_panel.h
    src/tui/components/process_table.h
    src/tui/components/connection_table.h
    src/tui/components/process_detail.h
    src/tui/screens/screen.h
    src/tui/screens/dashboard_screen.h
    src/tui/screens/process_list_screen.h
    src/tui/screens/connection_screen.h
    src/tui/screens/process_detail_screen.h
    src/tui/screens/help_screen.h
)

# ── Libraries ─────────────────────────────────────────────────
# C Core Library (static)
add_library(sysmon_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})
target_include_directories(sysmon_core PUBLIC
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/bridge
)
target_link_libraries(sysmon_core PUBLIC Threads::Threads)

# C++ TUI Library (static)
add_library(sysmon_tui STATIC ${TUI_SOURCES} ${TUI_HEADERS})
target_include_directories(sysmon_tui PUBLIC
    ${CMAKE_SOURCE_DIR}/src/tui
    ${CMAKE_SOURCE_DIR}/src/bridge
)
target_link_libraries(sysmon_tui PUBLIC 
    sysmon_core
    ${CURSES_LIBRARIES}
)

# ── Main Executable ───────────────────────────────────────────
add_executable(sysmon src/main.cpp)
target_link_libraries(sysmon PRIVATE sysmon_tui)

# ── Installation ──────────────────────────────────────────────
include(GNUInstallDirs)

install(TARGETS sysmon 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Config file
install(FILES config/default.conf 
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/sysmon
)

# Plugins
if(INSTALL_PLUGINS)
    install(DIRECTORY scripts/plugins/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/sysmon/plugins
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                         GROUP_READ GROUP_EXECUTE
                         WORLD_READ WORLD_EXECUTE
    )
    
    install(DIRECTORY scripts/alerts/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/sysmon/alerts
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                         GROUP_READ GROUP_EXECUTE
                         WORLD_READ WORLD_EXECUTE
    )
endif()

# ── Tests ─────────────────────────────────────────────────────
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ── Print Summary ─────────────────────────────────────────────
message(STATUS "SYSMON v${PROJECT_VERSION}")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Curses:         ${CURSES_LIBRARIES}")
message(STATUS "  Sanitizers:     ${ENABLE_SANITIZERS}")
message(STATUS "  LTO:            ${ENABLE_LTO}")
message(STATUS "  Tests:          ${BUILD_TESTS}")
```

---

## 9. Configuration

### Default Configuration File

```ini
# SYSMON Configuration
# Location: ~/.config/sysmon/config
# System-wide: /etc/sysmon/default.conf

[collection]
# Main collection interval in milliseconds
interval = 1000

# Maximum number of processes to track
max_processes = 10000

# Collect these metrics (true/false)
collect_connections = false    # Only when connections view is active
collect_disk_io = true
collect_filesystems = true

# Username resolution (getpwuid can be slow with LDAP)
resolve_usernames = true
username_cache_ttl_seconds = 300

[display]
# Update interval for TUI redraw (Hz)
refresh_rate = 30

# Theme
theme = default    # default, dark, light, solarized, custom

# Color mode
color = 256        # 16, 256, truecolor

# Show these panels on dashboard
dashboard_panels = cpu,memory,network,disk

# Default sort for process list
process_sort_column = cpu
process_sort_descending = true

# Tree view
process_tree_view = false

[plugins]
# Plugin directories (searched in order)
plugin_dirs = 
    ~/.local/share/sysmon/plugins
    /usr/share/sysmon/plugins

# Auto-start these plugins
auto_start = gpu_monitor,temperature

# Plugin timeout (seconds, kill if no output)
plugin_timeout = 5

[alerts]
# Alert scripts directory
alert_dir = ~/.local/share/sysmon/alerts

# Enable alerts
enable_alerts = false

[keys]
# Custom key bindings (override defaults)
# quit = q
# process_list = F5
# connections = F6
# dashboard = F7
# help = F1

[advanced]
# Debug logging
debug = false
log_file = /tmp/sysmon.log

# Process stat file read limit per cycle
# Prevents DoS on systems with thousands of processes
process_read_budget_ms = 50
```

### Configuration Loading Priority

```
1. Command-line arguments (highest priority)
2. ~/.config/sysmon/config
3. $XDG_CONFIG_HOME/sysmon/config
4. /etc/sysmon/default.conf
5. Built-in defaults (lowest priority)
```

---

## 10. API Reference

### C API (for embedding/extending)

```c
/* ── Snapshot Manager ──────────────────────────────────── */

/* Initialize the snapshot manager. Call once at startup. */
int snapshot_manager_init(SnapshotManager* mgr);

/* Get the current immutable snapshot. Thread-safe. */
const SystemSnapshot* snapshot_get_current(SnapshotManager* mgr);

/* ── Collection Engine ─────────────────────────────────── */

/* Initialize collection engine with snapshot manager */
int collection_engine_init(CollectionEngine* engine, SnapshotManager* mgr);

/* Start collection in background thread */
int collection_engine_start(CollectionEngine* engine);

/* Stop collection thread */
int collection_engine_stop(CollectionEngine* engine);

/* ── Process Table Helpers ─────────────────────────────── */

/* Find process by PID in snapshot. O(n) scan. */
const ProcessSnapshot* process_find_by_pid(const SystemSnapshot* snap, int pid);

/* Get children of a process. Returns count, fills array. */
int process_get_children(const SystemSnapshot* snap, int ppid, 
                         const ProcessSnapshot** children, int max_children);

/* ── Utility ───────────────────────────────────────────── */

/* Format bytes to human-readable string */
const char* format_bytes(uint64_t bytes);  // Returns static buffer

/* Format duration in microseconds to human-readable */
const char* format_duration_us(uint64_t us);

/* Get current monotonic time in microseconds */
uint64_t get_time_us(void);
```

### C++ API (for extending panels)

```cpp
/* ── Panel Base ────────────────────────────────────────── */

class Panel {
public:
    Panel(int height, int width, int start_y, int start_x);
    
    /* Must override */
    virtual void render(const SystemSnapshot* snapshot) = 0;
    
    /* Optional overrides */
    virtual void on_focus();
    virtual void on_blur();
    virtual bool handle_input(int key);
    virtual void on_resize(int h, int w, int y, int x);
};

/* ── Screen Base ───────────────────────────────────────── */

class Screen {
public:
    /* Must override */
    virtual void render(const SystemSnapshot* snapshot) = 0;
    virtual bool handle_input(int key) = 0;
    
    /* Optional */
    virtual void on_enter();
    virtual void on_exit();
    virtual void on_resize();
    
    /* Add a panel to this screen */
    void add_panel(std::unique_ptr<Panel> panel);
};

/* ── Application ───────────────────────────────────────── */

class Application {
public:
    int run(int argc, char** argv);
    
    /* Register a custom screen */
    void register_screen(const std::string& name, 
                         std::unique_ptr<Screen> screen);
    
    /* Switch to screen by name */
    void switch_screen(const std::string& name);
};
```

### Plugin Protocol (Bash stdout)

```
FORMAT:
  type:key=value[:unit]

TYPES:
  gauge     - Instantaneous value (temperature, memory)
  counter   - Monotonically increasing (bytes transferred)
  info      - Static information (version, capabilities)
  table     - Multi-row data (container list)
  error     - Error message (displayed to user)

EXAMPLES:
  gauge:cpu.temperature=45:C
  gauge:memory.used=8589934592:bytes
  counter:network.bytes_rx=1234567890
  info:plugin.version=1.0.0
  table:docker.containers
  col:id,name,status,cpu
  row:abc123,web,up,2.5
  row:def456,db,up,12.1
  endtable
  error:GPU not found

PROTOCOL RULES:
  1. One metric per line
  2. Lines starting with # are comments (ignored)
  3. Empty lines are ignored
  4. Exit code 0 = success
  5. Exit code non-zero = plugin error (logged)
  6. Plugin can suggest interval: interval:2000
  7. Plugin can declare version: version:1.0.0
```

---

## Appendix A: /proc Files Used

```
FILE                    PURPOSE                     FREQUENCY
─────────────────────────────────────────────────────────────
/proc/stat              CPU times, boot time        Every cycle
/proc/loadavg           Load averages               Every cycle
/proc/meminfo           Memory details              Every cycle
/proc/net/dev           Network interfaces          Every cycle
/proc/diskstats         Disk I/O                    Every cycle
/proc/uptime            System uptime               Every cycle
/proc/*/stat            Process stats               Every cycle
/proc/*/status          Process extended info       Every cycle
/proc/*/cmdline         Process command line        On demand
/proc/*/io              Process I/O                 Every cycle
/proc/*/fd/             File descriptors            On demand (detail view)
/proc/*/environ         Environment                 On demand (detail view)
/proc/net/tcp           TCP connections             When view active
/proc/net/tcp6          TCPv6 connections           When view active
/proc/net/udp           UDP connections             When view active
/proc/net/udp6          UDPv6 connections           When view active
/proc/net/unix          Unix sockets                When view active
/proc/mounts            Mount points                Every 30s
/sys/class/thermal/*    Temperature sensors         Every 5s (if plugin)
/sys/block/*/stat       Block device stats          Every cycle (alt)
```

## Appendix B: Exit Codes

```
0   - Normal exit (user quit)
1   - General error
2   - /proc not mounted
3   - Terminal too small (minimum 80x24)
4   - Cannot open terminal
5   - Permission denied (need /proc read access)
6   - Another instance running (single-instance lock)
7   - Configuration error
130 - Interrupted (SIGINT)
143 - Terminated (SIGTERM)
```

---

*Documentation version: 1.0.0 | Generated for sysmon architecture review*
