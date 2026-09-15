# 🖥️ Linux System Resource Monitor

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg?style=flat-square&logo=c%2B%2B" alt="C++17">
  <img src="https://img.shields.io/badge/Platform-Linux%20%2F%20WSL2-lightgrey.svg?style=flat-square&logo=linux" alt="Platform: Linux/WSL2">
  <img src="https://img.shields.io/badge/Build-GNU%20Make-orange.svg?style=flat-square&logo=gnu" alt="Build: GNU Make">
  <img src="https://img.shields.io/badge/License-MIT-green.svg?style=flat-square" alt="License: MIT">
</p>

---

## 📖 Overview & Motivation

**Linux System Resource Monitor** is a lightweight, dependency-free command-line telemetry tool that reads real-time CPU utilization, memory consumption, and system uptime directly from Linux kernel pseudo-filesystems — no shelling out to `top`, `free`, or `vmstat`.

Most beginner-level system monitors take the shortcut of forking a subprocess and parsing the text output of standard utilities. This works, but it is fundamentally the wrong tool for the job:

- **Process overhead** — Every call to `popen("top -bn1")` or `system("free -m")` forces a `fork()` + `exec()` of an entire new process image, incurring kernel scheduling overhead and page-table setup costs that dwarf the actual work being done.
- **Fragile parsing** — Shelling out ties correctness to the *locale-dependent, version-dependent* text formatting of an external binary. A distro upgrade or locale change can silently break column parsing.
- **Non-portable dependencies** — `top`, `free`, and `vmstat` are not guaranteed to exist (or behave identically) across every Linux distribution or container base image.
- **No fine-grained control** — Shell utilities expose only their own sampling model. Native access lets this tool define its own sampling interval, delta computation, and formatting logic.

By reading `/proc/stat`, `/proc/meminfo`, and `/proc/uptime` directly through `std::ifstream`, this project talks to the kernel's own accounting interfaces with zero external process spawns, zero third-party libraries, and full control over sampling precision — the same approach used internally by `top`, `htop`, and `free` themselves.

---

## ✨ Features

| Metric | Kernel Data Source | Calculation |
|---|---|---|
| **CPU Utilization** | `/proc/stat` | `Utilization = (1 - (Δidle / Δtotal)) * 100` computed across two sampling ticks |
| **Memory Usage** | `/proc/meminfo` | `Used = MemTotal - (MemFree + Buffers + Cached)` |
| **System Uptime** | `/proc/uptime` | Direct extraction of the first floating-point field (seconds since boot) |

---

## 📂 Directory Structure

```
Linux_System_resource_monitor/
├── Makefile
├── .gitignore
├── README.md
└── src/
    ├── main.cpp
    ├── ProcessMonitor.hpp
    └── ProcessMonitor.cpp
```

---

## 🔬 Technical Deep Dive

### CPU Accounting via `/proc/stat`

The kernel exposes cumulative tick counters for the aggregate CPU line (`cpu ...`) since boot, broken down by the mode the CPU was in during each tick:

```
cpu  user nice system idle iowait irq softirq steal guest guest_nice
```

| Field | Meaning |
|---|---|
| `user` | Time spent executing normal user-space processes |
| `nice` | Time spent on user-space processes with a positive nice (lowered priority) |
| `system` | Time spent executing kernel-space (syscalls, interrupts handling) |
| `idle` | Time spent idle — no runnable tasks |
| `iowait` | Time spent idle while waiting on outstanding I/O |
| `irq` | Time servicing hardware interrupts |
| `softirq` | Time servicing software interrupts |
| `steal` | Time stolen by the hypervisor for other virtual machines (relevant under WSL2/virtualized kernels) |

These counters are **monotonically increasing** since boot, not instantaneous values — a single read is meaningless on its own. Utilization is derived by sampling the counters at two points in time separated by a fixed interval, then computing tick **deltas**:

```
Δtotal = total(t1) - total(t0)
Δidle  = idle(t1)  - idle(t0)

Utilization % = (1 - (Δidle / Δtotal)) * 100
```

`total` is the sum of all fields in the line. `idle` in this implementation is treated as `idle + iowait`, since I/O-wait ticks represent a CPU that had no work to schedule, not active computation. This two-sample delta approach is exactly how `top` and `mpstat` compute their percentages internally — it eliminates the need for any privileged performance counters or `/dev/cpu` access.

### Memory Accounting via `/proc/meminfo`

A naive reading of "used memory" as `MemTotal - MemFree` is **incorrect** on Linux and produces alarmingly high numbers that don't reflect actual memory pressure. This is because the Linux kernel aggressively uses otherwise-idle RAM for:

- **Buffers** — Block-device I/O buffers (raw disk block caching), reclaimed on demand.
- **Cached** — Page cache for file contents (source files, executables, libraries), also reclaimable under pressure.

Both are reclaimable on demand by the kernel the instant an application actually needs the memory — they are not "in use" in any meaningful sense, they are the kernel opportunistically caching data to speed up future I/O. Excluding them from the "used" figure is what gives an accurate picture of memory actually committed to running processes:

```
Used = MemTotal - (MemFree + Buffers + Cached)
```

This mirrors the accounting logic behind the `free -m` `used` column and avoids the common pitfall of treating page-cache-heavy systems as memory-starved when they are, in fact, healthy.

### Uptime via `/proc/uptime`

`/proc/uptime` exposes two space-separated floating-point values:

```
<seconds_since_boot> <seconds_spent_idle_across_all_cores>
```

The first field is read directly via `std::ifstream` and reported as-is — no delta computation is required since it is already a continuously accumulating wall-clock measurement from the kernel's boot-time reference point.

---

## ⚙️ Build & Run Instructions

### Prerequisites

- `g++` supporting C++17 (GCC 7+)
- GNU `make`
- A Linux environment (native or WSL2)

### Build

```bash
make
```

### Run

```bash
./res_monitor_cpp
```

### Clean build artifacts

```bash
make clean
```

---

## 📊 Sample Output

```
--- System Resource Monitor ---
CPU Usage:     0.00%
Memory Usage:  4.27%
System Uptime: 514.25 seconds
```

---

## 📜 License & Author

Licensed under the **MIT License**.

**Author:** Rachit Patnaik
**Repository:** [Linux_System_resource_monitor](https://github.com/Rachit-Patnaik/Linux_System_resource_monitor)
