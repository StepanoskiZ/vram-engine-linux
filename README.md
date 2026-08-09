# ⚡ S-OS V-RAM Engine™ (Linux / POSIX Edition)

![License](https://img.shields.io/badge/License-Community_Edition-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Linux_|_Yocto_|_Ubuntu-orange.svg)
![Downloads](https://img.shields.io/github/downloads/StepanoskiZ/vram-engine-linux/total?color=emerald)

**Eliminate Out-Of-Memory (OOM) crashes and protect your eMMC Flash wear.**  
S-OS V-RAM Engine™ is a high-performance, dynamic C++ virtual memory paging system designed for Embedded Linux devices, Automotive Telematics (TCU), and IoT Gateways. Powered by the proprietary **S-OS Adaptive Compression Engine™**, it extends physical RAM capacity while drastically reducing storage wear.

---

## 🚀 The "Zero-Code" Integration (`LD_PRELOAD`)

You do **not** need to modify a single line of your existing closed-source applications!

Using standard POSIX Shared Library Injection (`LD_PRELOAD`), the V-RAM Engine transparently intercepts standard file I/O calls (`open`, `write`, `close`) from target applications. The app believes it is writing raw telemetry to the disk, but the data is actually routed into the V-RAM's compressed LRU cache matrix.

**Perfect for Automotive & Fleet Telematics:**
* Expand a restrictive 25 MB eMMC log quota into **1+ Gigabytes of virtual circular buffer**.
* Log high-frequency CAN-bus data for days without internet connectivity.
* Reduce physical eMMC Flash write-wear by over **90%**.

---

## 📊 Performance Benchmarks (Empirically Proven)

*Benchmarks executed on a standard Ubuntu/WSL2 Linux environment processing 500,000 structured CAN-bus telemetry records (32 MB raw).*

* **Raw Data Processed:** 32,000,000 Bytes (~30.5 MB)
* **Actual Compressed Size on Disk:** 2,558,784 Bytes (~2.44 MB)
* **Real-World Compression Ratio:** **12.51x**
* **Process RAM Footprint (RSS):** Reduced from 30.5 MB down to **2.8 MB (90.8% RAM Saved!)**
* **Processing Throughput:** **45.8 MB/s** (45,879 KB/s)

---

## 📂 Repository Structure

```text
vram-engine-linux/
├── VirtualMemoryEngine.h               # Clean C++ Header API
├── lib/                                
│   ├── libVirtualMemoryEngine.a        # Pre-compiled static library (x86_64)
│   └── libvram_hook.so                 # Ready-to-use LD_PRELOAD shared library
└── examples/
    ├── tcu_vram_hook.cpp               # Source code for the POSIX interceptor hook
    ├── process_stress_test.cpp         # Linux telemetry stress test generator
    └── generate_charts.py              # Python script to visualize benchmarks
```

---

## 💻 Quickstart & Evaluation Guide

### 1. Compile the Stress Test
Compile the provided telemetry generator (which attempts to write 32MB of raw data to `/tmp/telemetry.log`):
```bash
g++ -O3 examples/process_stress_test.cpp -o process_stress_test
```

### 2. Run with V-RAM Zero-Code Hook
Inject the V-RAM shared library using `LD_PRELOAD`. The hook will automatically detect the log file, intercept the writes, and compress the payload on the fly:
```bash
LD_PRELOAD=./lib/libvram_hook.so ./process_stress_test
```
*You will see the V-RAM Engine dynamically intercept the FD and flush the compressed payload upon exit.*

### 3. Build Your Own Custom Hook
If your target application uses a different log path (e.g., `/var/log/my_app.dat`), simply edit `examples/tcu_vram_hook.cpp`, update the `is_target_log_file()` filter, and recompile it against our static library:
```bash
g++ -fPIC -shared -O3 examples/tcu_vram_hook.cpp -Llib/ -lVirtualMemoryEngine -ldl -lpthread -o libvram_hook.so
```

---

## 🛡️ License & Commercial Rights

* **Community Edition:** The `.a` and `.so` binaries provided in this repository are free for non-commercial open-source projects, education, and evaluation purposes. *(Note: The Community Edition has a hard-capped virtual memory limit of 512 KB).*
* **Commercial Pro & Enterprise OEM:** Copyright © 2026 **Syntetika Universe** by Zoran Stepanoski. All Rights Reserved.

**Need Unlimited V-RAM or ARM Cross-Compilation (Yocto / i.MX6)?**  
Commercial licenses unlock unlimited virtual memory capacity, custom Yocto ARM/AARCH64 builds, and dedicated SLA support.

For commercial licensing, custom MCU porting, or Automotive eMMC wear-leveling engineering:  
📧 Email: `zstepanoski@gmail.com`  
🌐 Website: [zoranstepanoski-prof-website.fly.dev](https://zoranstepanoski-prof-website.fly.dev/)
