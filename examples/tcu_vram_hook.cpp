// tcu_vram_hook.cpp

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <cstdarg>
#include <mutex>
#include <unordered_map>
#include "VirtualMemoryEngine.h"

// Originalni pokazivači na Linux libc funkcije
typedef int (*real_open_t)(const char *pathname, int flags, ...);
typedef int (*real_openat_t)(int dirfd, const char *pathname, int flags, ...);
typedef ssize_t (*real_write_t)(int fd, const void *buf, size_t count);
typedef ssize_t (*real_pwrite_t)(int fd, const void *buf, size_t count, off_t offset);
typedef int (*real_close_t)(int fd);

// Statik pokazivači na prave libc funkcije (Čitaju se samo jednom!)
static real_open_t real_open_fn = nullptr;
static real_open_t real_open64_fn = nullptr; 
static real_openat_t real_openat_fn = nullptr;
static real_write_t real_write_fn = nullptr;
static real_pwrite_t real_pwrite_fn = nullptr;
static real_close_t real_close_fn = nullptr;

// Globalni pokazivači i sinhronizacija
static VirtualMemoryEngine* g_vram = nullptr;

static std::recursive_mutex g_hook_mutex;
// Mapa koja prati sve otvorene FD fajlove i njihove V-RAM offsete
static std::unordered_map<int, uint32_t> g_active_fds;
static uint32_t g_global_vram_addr = 0;

// Zaštita od rekurzivnih poziva po niti
static __thread bool g_in_hook = false;

// =========================================================================
// DINAMIČKI FILTER: Prepoznaje log fajlove po ekstenzijama ili folderu
// =========================================================================
static bool is_target_log_file(const char* pathname) {
    if (!pathname) return false;
    
    // IGNORIŠI SOPSTVENE SWAP FAJLOVE V-RAM ENGINE-A
    if (strstr(pathname, "vram_swap") != nullptr || strstr(pathname, "v_dat") != nullptr) {
        return false;
    }

    // Prepoznajemo log fajl
    if (strstr(pathname, "telemetry") != nullptr ||
        strstr(pathname, ".log") != nullptr ||
        strstr(pathname, ".bin") != nullptr) {
        return true;
    }
    return false;
}

// =========================================================================
// AUTOMATSKO POKRETANJE I GAŠENJE BIBLIOTEKE (CONSTRUCTOR / DESTRUCTOR)
// =========================================================================
__attribute__((constructor))
static void on_library_load() {
    g_in_hook = true;

    // 1. Čitamo prave libc pokazivače SAMO JEDNOM pri učitavanju biblioteke
    real_open_fn = (real_open_t)dlsym(RTLD_NEXT, "open");
    real_open64_fn = (real_open_t)dlsym(RTLD_NEXT, "open64");
    real_openat_fn = (real_openat_t)dlsym(RTLD_NEXT, "openat");
    real_write_fn = (real_write_t)dlsym(RTLD_NEXT, "write");
    real_pwrite_fn = (real_pwrite_t)dlsym(RTLD_NEXT, "pwrite");
    real_close_fn = (real_close_t)dlsym(RTLD_NEXT, "close");

    {
        std::lock_guard<std::recursive_mutex> lock(g_hook_mutex);
        if (!g_vram) {
            g_vram = new VirtualMemoryEngine(32768, 131072, 16, 524288, "/tmp/vram_swap");
            if (g_vram->begin()) {
                printf("[V-RAM HOOK] 🚀 Shared Library ucitana! S-OS V-RAM Engine spreman.\n");
            }
        }
    }

    g_in_hook = false; // Odblokiramo huker za rad aplikacije
}

__attribute__((destructor))
static void on_library_unload() {
    g_in_hook = true;
    {
        std::lock_guard<std::recursive_mutex> lock(g_hook_mutex);
        if (g_vram) {
            printf("[V-RAM HOOK] 🛑 Process exiting. Flushing V-RAM...\n");
            g_vram->flush();

            size_t compBytes = g_vram->getCompressedSize();
            float ratio = g_vram->getCompressionRatio(g_global_vram_addr);

            printf("========================================================\n");
            printf("✓ STVARNO KOMPRIMOVANO NA DISKU: %zu B (%.2f MB)\n", compBytes, compBytes / (1024.0 * 1024.0));
            printf("✓ STVARNI FAKTOR KOMPRESIJE: %.4fx\n", ratio);
            printf("========================================================\n");

            delete g_vram;
            g_vram = nullptr;
        }
    }
    g_in_hook = false;
}

// =========================================================================
// HUKERI ZA 'open', 'open64', 'openat'
// =========================================================================
static void register_fd_if_target(int fd, const char* pathname) {
    if (fd >= 0 && is_target_log_file(pathname)) {
        std::lock_guard<std::recursive_mutex> lock(g_hook_mutex);
        // Dodeljujemo trenutni V-RAM offset ovom File Descriptor-u
        g_active_fds[fd] = g_global_vram_addr;
        printf("[V-RAM HOOK] 🎯 Detektovan log fajl: %s (FD: %d | V-RAM Offset: %u)\n", pathname, fd, g_global_vram_addr);
    }
}

extern "C" int open(const char *pathname, int flags, ...) {
    if (!real_open_fn) real_open_fn = (real_open_t)dlsym(RTLD_NEXT, "open");

    // Obavezna obrada varijabilnih argumenata (ako postoje)
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }
    
    // Pozivamo pravi 'open' da operativni sistem zapravo otvori fajl
    int fd = real_open_fn(pathname, flags, mode);
    
    if (!g_in_hook) {
        g_in_hook = true;
        register_fd_if_target(fd, pathname);
        g_in_hook = false;
    }
    return fd;
}

extern "C" int open64(const char *pathname, int flags, ...) {
    if (!real_open64_fn) real_open64_fn = (real_open_t)dlsym(RTLD_NEXT, "open64");
    
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }
    
    int fd = real_open64_fn(pathname, flags, mode);
    
    if (!g_in_hook) {
        g_in_hook = true;
        register_fd_if_target(fd, pathname);
        g_in_hook = false;
    }
    return fd;
}

extern "C" int openat(int dirfd, const char *pathname, int flags, ...) {
    if (!real_openat_fn) real_openat_fn = (real_openat_t)dlsym(RTLD_NEXT, "openat");

    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }
    
    int fd = real_openat_fn(dirfd, pathname, flags, mode);
    
    if (!g_in_hook) {
        g_in_hook = true;
        register_fd_if_target(fd, pathname);
        g_in_hook = false;
    }
    return fd;
}

// =========================================================================
// HUKERI ZA 'write' I 'pwrite' (Upis u V-RAM umesto na disk)
// =========================================================================
extern "C" ssize_t write(int fd, const void *buf, size_t count) {
    if (!real_write_fn) real_write_fn = (real_write_t)dlsym(RTLD_NEXT, "write");

    if (!g_in_hook && g_vram != nullptr && count > 0) {
        g_in_hook = true;
        
        bool is_intercepted = false;
        uint32_t addr = 0;
        
        {
            std::lock_guard<std::recursive_mutex> lock(g_hook_mutex);
            auto it = g_active_fds.find(fd);
            if (it != g_active_fds.end()) {
                is_intercepted = true;
                addr = it->second;
                it->second += count;
                g_global_vram_addr += count;
            }
        }

        if (is_intercepted) {
            g_vram->write(addr, buf, count);
            g_in_hook = false;
            return count; // Preusmereno u V-RAM
        }

        g_in_hook = false;
    }

    return real_write_fn(fd, buf, count);
}

extern "C" ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset) {
    if (!real_pwrite_fn) real_pwrite_fn = (real_pwrite_t)dlsym(RTLD_NEXT, "pwrite");

    if (!g_in_hook && g_vram != nullptr && count > 0) {
        g_in_hook = true;
        bool is_intercepted = false;

        {
            std::lock_guard<std::recursive_mutex> lock(g_hook_mutex);
            auto it = g_active_fds.find(fd);
            if (it != g_active_fds.end()) {
                is_intercepted = true;
            }
        }

        if (is_intercepted) {
            g_vram->write((uint32_t)offset, buf, count);
            g_in_hook = false;
            return count;
        }
        g_in_hook = false;
    }

    return real_pwrite_fn(fd, buf, count, offset);
}

// =========================================================================
// HUKER ZA 'close'
// =========================================================================
extern "C" int close(int fd) {
    if (!real_close_fn) real_close_fn = (real_close_t)dlsym(RTLD_NEXT, "close");

    if (!g_in_hook && fd >= 0) {
        g_in_hook = true;

        {
            std::lock_guard<std::recursive_mutex> lock(g_hook_mutex);
            auto it = g_active_fds.find(fd);
            if (it != g_active_fds.end()) {
                printf("[V-RAM HOOK] 🛑 Zatvaram log fajl (FD: %d). Flushujem V-RAM kes...\n", fd);
                if (g_vram) {
                    g_vram->flush();
                }
                g_active_fds.erase(it);
            }
        }

        g_in_hook = false;
    }

    return real_close_fn(fd);
}
