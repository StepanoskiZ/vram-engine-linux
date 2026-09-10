# ⚡ S-OS V-RAM Engine™ (Linux / POSIX Edition)

![License](https://img.shields.io/badge/License-Community_Edition-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Linux_|_Yocto_|_Ubuntu_|_Debian-orange.svg)
![Integrity](https://img.shields.io/badge/Data_Integrity-100%25_Bit--Exact-brightgreen.svg)
![Throughput](https://img.shields.io/badge/Throughput->21_MB/s-emerald.svg)
[![Downloads](https://img.shields.io/github/downloads/StepanoskiZ/vram-engine-linux/total?color=emerald&logo=github)](https://github.com/StepanoskiZ/vram-engine-linux/releases)

**Eliminate Out-Of-Memory (OOM) crashes and protect your eMMC Flash wear on Embedded Linux.**  
S-OS V-RAM Engine™ is a high-performance, dynamic C++ virtual memory paging system designed for Embedded Linux devices, Automotive Telematics (TCU), and Industrial IoT Gateways. Powered by the proprietary **S-OS Adaptive Compression Engine™**, it transparently extends physical RAM capacity while drastically reducing physical storage write cycles.

---

## 🚀 The "Zero-Code" Integration (`LD_PRELOAD`)

You do **not** need to modify a single line of code in existing closed-source applications!

Using standard POSIX Shared Library Interception (`LD_PRELOAD`), the V-RAM Engine transparently intercepts standard file I/O calls (`open`, `open64`, `write`, `pwrite`, `close`). The target application believes it is writing raw telemetry to physical storage, while the data is actually routed directly into V-RAM's compressed, log-structured in-memory cache.

**Engineered for Automotive Telematics & Edge Gateways:**
* Expand a restrictive eMMC log partition quota into **gigabytes of virtual circular buffer**.
* Log high-frequency CAN-bus and sensor data without fear of exhausting device storage.
* Protect hardware longevity: reduce physical eMMC Flash write wear by **over 90%** via strict dirty-block tracking.

---

## 📊 Performance Benchmarks (Empirically Proven)

*Benchmarks executed on Linux POSIX environment processing structured CAN-bus telemetry records:*

| Metric | Measured Value | Verification Details |
| :--- | :--- | :--- |
| **Real-World Compression Ratio** | **12.31x – 12.51x** | 500 KB raw CAN telemetry compressed down to **41.5 KB** on disk |
| **Process RAM Footprint (RSS)** | **90.8% Saved** | Physical process memory consumption reduced from 30.5 MB to **2.8 MB** |
| **Write Interception Throughput** | **21.2 MB/s** | Real-time POSIX `write()` hook throughput (up to 45.8 MB/s burst) |
| **Data Integrity Verification** | **100% Perfect (0 Errors)** | Bit-exact validation over 100,000 float samples and 500,000 log records |
| **POSIX VEH Memory Hook** | **Verified** | Successful runtime memory paging via `mmap` / `mprotect` Page Fault trapping |

---

## 📂 Repository Structure

```text
vram-engine-linux/
├── README.md                           # Documentation & Benchmark overview
├── include/                            
│   └── VirtualMemoryEngine.h           # Clean C++ Public Header API (Pimpl pattern)
├── lib/                                
│   ├── libVirtualMemoryEngine.a        # Pre-compiled, hardened static library (x86_64)
│   └── libvram_hook.so                 # Ready-to-use LD_PRELOAD shared library
└── examples/
    ├── tcu_vram_hook.cpp               # Source code for the POSIX interceptor hook
    ├── process_stress_test.cpp         # High-throughput telemetry stress test
    ├── test_vram.cpp                   # Standalone C++ API integration test
    └── test_veh_hook.cpp               # POSIX Page Fault (SIGSEGV) interceptor demo
```

---

## 💻 Quickstart & Evaluation Guide

### 1. Standalone C++ Verification
Compile and run the basic V-RAM API test linking against the pre-compiled static library:
```bash
g++ -O3 examples/test_vram.cpp -Iinclude/ -Llib/ -lVirtualMemoryEngine -lpthread -o test_vram
./test_vram
```

### 2. Zero-Code Automotive Telematics Hook (`LD_PRELOAD`)
Compile the telemetry benchmark generator:
```bash
g++ -O3 examples/process_stress_test.cpp -o process_stress_test
```

Run the application with the pre-compiled V-RAM shared library hook injected:
```bash
LD_PRELOAD=./lib/libvram_hook.so ./process_stress_test
```
*The hook will automatically detect the telemetry log file, intercept the `write()` calls, route them into the compressed engine, and flush upon exit.*

### 3. Build Your Own Custom Hook
To customize target log file names or paths (e.g., `/var/log/tcu_telemetry.bin`), adjust the `is_target_log_file()` filter in `examples/tcu_vram_hook.cpp` and recompile:
```bash
g++ -fPIC -shared -O3 examples/tcu_vram_hook.cpp -Iinclude/ -Llib/ -lVirtualMemoryEngine -ldl -lpthread -o libvram_hook.so
strip --strip-debug --strip-unneeded libvram_hook.so
```

---

## 🚚 Automated Deployment in Production (Yocto / Automotive Linux)

To enable transparent V-RAM compression system-wide without modifying systemd services or scripts:

### Method A: Global System Preload (Recommended)
Add the path of the shared library to `/etc/ld.so.preload`:
```bash
echo "/usr/lib/libvram_hook.so" >> /etc/ld.so.preload
```
*All applications writing targeted logs will automatically compress their data into V-RAM on boot.*

### Method B: Containerized Podman / Docker Deployment
Inject the hook inside your container launcher script:
```bash
export LD_PRELOAD=/opt/vram/libvram_hook.so
```

---

## 🛡️ License & Commercial Rights

* **Community Edition:** The binaries provided in this repository are free for non-commercial projects, education, and evaluation purposes (strictly hard-capped at 512 KB virtual address space).
* **Commercial Pro, Growth & Enterprise OEM:** Copyright © 2026 **Syntetika Universe** by Zoran Stepanoski. All Rights Reserved.

**Need Unlimited V-RAM or Native ARM Cross-Compilation (Yocto / i.MX6 / i.MX8 / Texas Instruments)?**  
Commercial licenses include:
* Unlimited virtual memory addressing space (16 MB – 50 MB+).
* Pre-compiled toolchains for ARMv7, AArch64, RISC-V, and custom embedded Linux distros.
* Dedicated SLA engineering and integration support.

For commercial licensing, custom porting, or eMMC wear-leveling consultation:  
📧 **Email:** `zstepanoski@gmail.com`  