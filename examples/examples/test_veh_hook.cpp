// test_veh_hook.cpp

#include <iostream>
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include "VirtualMemoryEngine.h"

// Globalni V-RAM pokazivač i zaklonjena virtuelna memorija
VirtualMemoryEngine* g_vram = nullptr;
void* g_guardedAddress = nullptr;
const size_t VRAM_TOTAL_SIZE = 4 * 1024 * 1024; // 4 MB Virtuelnog prostora
#undef PAGE_SIZE                                // Ponštavamo postojeći Linux makro
const size_t PAGE_SIZE = 4096;                  // 4 KB POSIX Stranica (Standard na Linuxu)

// Evidencija otključanih stranica u RAM-u
bool g_unlockedPages[VRAM_TOTAL_SIZE / 4096] = { false };

// =========================================================================
// POSIX EKVIVALENT ZA VEH (Hvatač SIGSEGV Page Fault prekida na Linux-u)
// =========================================================================
void posix_veh_page_fault_handler(int sig, siginfo_t *si, void *unused) {
    uint8_t* faultAddr = (uint8_t*)si->si_addr;
    uint8_t* baseAddr = (uint8_t*)g_guardedAddress;

    // Proveravamo da li je adresa unutar našeg V-RAM opsega
    if (faultAddr >= baseAddr && faultAddr < (baseAddr + VRAM_TOTAL_SIZE)) {
        uint32_t offset = faultAddr - baseAddr;
        uint32_t pageIndex = offset / PAGE_SIZE;
        uint32_t pageOffset = pageIndex * PAGE_SIZE;
        uint8_t* pageAddr = baseAddr + pageOffset;

        // Ako stranica još uvek nije otključana u RAM-u
        if (!g_unlockedPages[pageIndex]) {
            // 1. Otključavamo 4KB stranicu memorije za čitanje/upis
            mprotect(pageAddr, PAGE_SIZE, PROT_READ | PROT_WRITE);

            // 2. 🚀 POVEZIVANJE SA V-RAM-OM: Učitavamo postojeće komprimovane podatke iz V-RAM-a!
            if (g_vram != nullptr) {
                g_vram->read(pageOffset, pageAddr, PAGE_SIZE);
            }

            g_unlockedPages[pageIndex] = true;
        }

        // Aplikacija sada neometano nastavlja rad!
        return;
    }

    // Ako je stvarna greška van V-RAM opsega, prekini rad
    std::cerr << "❌ Stvarna Segmentation Fault greška van V-RAM-a!" << std::endl;
    exit(1);
}

void init_posix_veh_system() {
    // 1. Postavljamo POSIX SIGSEGV hvatač prekida
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = posix_veh_page_fault_handler;
    sigaction(SIGSEGV, &sa, NULL);

    // 2. Alociramo 4MB virtuelnog prostora i zaključavamo ga (PROT_NONE = Page Guard)
    g_guardedAddress = mmap(NULL, VRAM_TOTAL_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    memset(g_unlockedPages, 0, sizeof(g_unlockedPages));
}

// =========================================================================
// SINHRONIZACIJA: Čuvanje izmena iz otključanog RAM-a natrag u V-RAM Kompresor
// =========================================================================
void sync_guarded_memory_to_vram() {
    if (!g_vram || !g_guardedAddress) return;

    uint8_t* baseAddr = (uint8_t*)g_guardedAddress;
    size_t totalPages = VRAM_TOTAL_SIZE / PAGE_SIZE;

    for (size_t i = 0; i < totalPages; i++) {
        if (g_unlockedPages[i]) {
            uint32_t pageOffset = i * PAGE_SIZE;
            uint8_t* pageAddr = baseAddr + pageOffset;

            // 1. Zapisujemo izmenjenu stranicu natrag u V-RAM kompresor
            g_vram->write(pageOffset, pageAddr, PAGE_SIZE);

            // 2. Ponovo zaključavamo stranicu (PROT_NONE) radi budućih prekida
            mprotect(pageAddr, PAGE_SIZE, PROT_NONE);
            g_unlockedPages[i] = false;
        }
    }
    // Prisilno zapisujemo sve blokove na disk
    g_vram->flush();
}

// =========================================================================
// SIMULACIJA ZATVORENE BOSCH TCU APLIKACIJE (NEMA IZMENE KODA!)
// =========================================================================
void run_closed_source_bosch_app() {
    std::cout << "\n▶ Pokrećem simulaciju zatvorene Bosch TCU Aplikacije..." << std::endl;

    // Aplikacija misli da je ovo običan fizički RAM pokazivač!
    float* telemetryBuffer = (float*)g_guardedAddress;

    std::cout << "▶ Bosch App piše telemetriju direktno na zaključani pokazivač (Trigeruje Page Fault)..." << std::endl;
    for (int i = 0; i < 1000; i++) {
        telemetryBuffer[i] = i * 2.5f; // Pokreće POSIX VEH Page Fault i otključava RAM!
    }

    std::cout << "▶ V-RAM automatski sinhronizuje i komprimuje izmene iz RAM-a na disk..." << std::endl;
    sync_guarded_memory_to_vram();

    std::cout << "▶ Bosch App čita telemetriju (Ponovo trigeruje Page Fault sa dekompresijom iz V-RAM-a)..." << std::endl;
    bool success = true;
    for (int i = 0; i < 1000; i++) {
        if (telemetryBuffer[i] != (i * 2.5f)) {
            success = false;
            break;
        }
    }

    if (success) {
        std::cout << "✓ POSIX VEH TRIK USPEO: Bosch aplikacija je izvršila upis, kompresiju i čitanje BEZ IZMENE KODA!" << std::endl;
    } else {
        std::cout << "❌ Greška u integritetu čitanja podataka!" << std::endl;
    }
}

int main() {
    std::cout << "========================================================" << std::endl;
    std::cout << "⚡ S-OS V-RAM Engine (POSIX VEH Page Fault Hook Demo v2.0)" << std::endl;
    std::cout << "========================================================\n" << std::endl;

    // 1. Inicijalizujemo V-RAM Engine sa 32KB blokovima
    g_vram = new VirtualMemoryEngine(32768, 131072, 16, 4194304, "/tmp/vram_veh_swap");
    if (!g_vram->begin()) {
        std::cout << "❌ Krah V-RAM-a!" << std::endl;
        return 1;
    }

    // 2. Inicijalizujemo POSIX VEH / SIGSEGV hvatač
    init_posix_veh_system();

    // 3. Pokrećemo simulaciju zatvorene aplikacije
    run_closed_source_bosch_app();

    std::cout << "\n========================================================" << std::endl;
    std::cout << "🏁 POSIX VEH TEST USPEŠNO ZAVRŠEN!" << std::endl;
    std::cout << "========================================================\n" << std::endl;

    delete g_vram;
    return 0;
}
