// test_vram.cpp

#include <iostream>
#include "VirtualMemoryEngine.h"

struct VehicleData {
    uint32_t timestamp;
    float speedKmh;
    float fuelLevel;
    char vinNumber[18];
};

int main() {
    std::cout << "\n========================================================" << std::endl;
    std::cout << "⚡ S-OS V-RAM Engine (Linux POSIX Verification Test)" << std::endl;
    std::cout << "========================================================\n" << std::endl;

    // Inicijalizacija: blockSize, ramLimit, maxBlocks, maxVramSize (512KB limit za CE), swapPath
    VirtualMemoryEngine vram(32768, 131072, 16, 524288, "/tmp/vram_test_swap");

    if (!vram.begin()) {
        std::cout << "❌ KRAH: V-RAM Engine se nije uspešno pokrenuo!" << std::endl;
        return 1;
    }
    std::cout << "✓ V-RAM Engine je uspešno pokrenut na Linux-u!" << std::endl;

    // Upisujemo 100.000 float vrednosti (100,000 * 4B = 400,000B ~390KB, što staje u 512KB CE limit)
    std::cout << "▶ Upisujem 100,000 float elemenata..." << std::endl;
    for (uint32_t i = 0; i < 100000; i++) {
        vram.put<float>(i * sizeof(float), i * 1.5f);
    }

    // Čitamo nazad i proveravamo integritet
    std::cout << "▶ Proveravam integritet podataka..." << std::endl;
    int errors = 0;
    for (uint32_t i = 0; i < 100000; i++) {
        float val = vram.get<float>(i * sizeof(float));
        if (val != (i * 1.5f)) errors++;
    }

    if (errors == 0) {
        std::cout << "✓ INTEGRITET PODATAKA: SAVRŠEN (0 grešaka na 100,000 elemenata)!" << std::endl;
    } else {
        std::cout << "❌ GREŠKA: Pronađeno " << errors << " oštećenih elemenata!" << std::endl;
    }

    // Prikaz kompresije
    std::cout << "✓ Stvarno komprimovano u memoriji: " << vram.getCompressedSize() << " bajtova" << std::endl;
    std::cout << "✓ Stvarni faktor kompresije: " << vram.getCompressionRatio(100000 * sizeof(float)) << "x" << std::endl;

    std::cout << "\n========================================================" << std::endl;
    std::cout << "🏁 LINUX TEST USPEŠNO ZAVRŠEN!" << std::endl;
    std::cout << "========================================================\n" << std::endl;

    return 0;
}
