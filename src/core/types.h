#ifndef SYSMON_TYPES_H
#define SYSMON_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

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
    int64_t clock_ticks_per_sec;    /* sysconf(_SC_CLK_TCK) */
    int num_cpu_cores;
    int num_cpu_sockets;
    uint64_t boot_time_epoch;
} SystemInfo;

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
    uint64_t total_ticks;           /* Sum of all above */
    uint64_t active_ticks;          /* total - idle - iowait */
    double usage_percent;           /* Calculated from previous snapshot */
    double user_percent;
    double sys_percent;
    double iowait_percent;
    double steal_percent;
} CpuCoreSnapshot;

typedef struct {
    uint64_t timestamp_us;
    int num_cores;
    CpuCoreSnapshot cores[256];     /* Supports up to 256 logical cores */
    double load_avg_1min;
    double load_avg_5min;
    double load_avg_15min;
    uint64_t context_switches;
    uint64_t interrupts;
    double ctxt_rate;               /* Context switches per sec */
    double irq_rate;                /* Interrupts per sec */
} CpuSnapshot;

/*
 * All values in bytes (converted from /proc/meminfo kB)
 * Usage% = (total - available) / total * 100
 */
typedef struct {
    /* Main metrics */
    uint64_t total_bytes;
    uint64_t available_bytes;       /* MemAvailable - best estimate */
    uint64_t free_bytes;            /* MemFree - actually free */
    uint64_t used_bytes;            /* total - available */
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

/*
 * Per-interface statistics
 * Rate = delta(bytes) / delta(time)
 */
typedef struct {
    char name[32];                  /* eth0, wlan0, etc. */
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
    NetworkIfaceSnapshot interfaces[32];  /* Typical system has <32 */
} NetworkSnapshot;

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
    int ppid;                       /* For tree reconstruction */
    int pgrp;
    int session;
    int tty_nr;
    
    char comm[256];                 /* Process name (truncated from /proc/[pid]/comm) */
    char state;                     /* R, S, D, Z, T, etc. */
    uid_t uid;
    char username[32];              /* Pre-resolved, empty if resolution failed */
    
    /* CPU */
    uint64_t utime_ticks;           /* User time */
    uint64_t stime_ticks;           /* System time */
    uint64_t starttime_ticks;       /* Process start time */
    double cpu_percent;             /* Pre-calculated CPU usage % */
    
    /* Memory */
    uint64_t rss_bytes;             /* Resident set size */
    uint64_t vsize_bytes;           /* Virtual memory size */
    double mem_percent;             /* % of total system memory */
    
    /* Scheduling */
    int nice;
    int num_threads;
    uint64_t wchan;                 /* Wait channel */
    
    /* I/O (optional, only if /proc/[pid]/io is readable) */
    uint64_t io_read_bytes;
    uint64_t io_write_bytes;
    
    /* Display metadata */
    bool is_kernel_thread;          /* cmdline is empty */
    bool is_zombie;                 /* state == 'Z' */
    
} ProcessSnapshot;

typedef struct {
    int num_processes;
    int capacity;                   /* Allocated size of array */
    ProcessSnapshot* processes;     /* Contiguous array, sorted by PID */
    
    /* Sorting indices built on demand by C++ */
    /* Not stored here - UI creates sorted views */
} ProcessTableSnapshot;

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
    
    char local_addr[64];            /* IP:port as string */
    char remote_addr[64];
    
    int pid;                        /* Owning process PID (may be -1 if unknown) */
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

typedef struct {
    char device_name[64];           /* nvme0n1, sda, etc. */
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
    uint64_t collection_sequence;   /* Monotonically increasing */
    uint64_t collection_duration_us;/* How long did collection take? */
    
    /* Sub-snapshots */
    SystemInfo* system_info;        /* Pointer (collected once, shared across cycles) */
    CpuSnapshot cpu;
    MemorySnapshot memory;
    NetworkSnapshot network;
    ProcessTableSnapshot processes;
    
    /* Optional sub-snapshots (NULL if not being viewed) */
    ConnectionTableSnapshot* connections;
    DiskSnapshot* disk;
    FilesystemTableSnapshot* filesystems;
    
    /* Plugin data (optional) */
    void* plugin_data;              /* Opaque, cast by plugin manager */
    void* plugin_mgr;               /* Opaque pointer to PluginManager */
    
} SystemSnapshot;

#endif /* SYSMON_TYPES_H */
