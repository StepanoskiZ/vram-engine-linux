//VirtualMemoryEngine.h

#ifndef VIRTUAL_MEMORY_ENGINE_H
#define VIRTUAL_MEMORY_ENGINE_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>

// Forward deklaracija - drži unutrašnju arhitekturu potpunom tajnom od klijenta (Pimpl Pattern)
class VirtualMemoryEngineImpl;

class VirtualMemoryEngine {
private:
    VirtualMemoryEngineImpl* pImpl; // Opaque pointer na tajnu implementaciju

public:
    // Konstruktor: (blockSize u B, ramLimit u B, maxBlocks u kešu, pathDoSwapFoldera)
    VirtualMemoryEngine(size_t blockSize = 32768, size_t ramLimit = 5242880, int maxBlocks = 64, uint32_t maxVramSize = 8388608, const std::string& swapPath = "/tmp/vram_swap");
    ~VirtualMemoryEngine();

    // Inicijalizacija sistema i LittleFS paging-a
    bool begin();

    // Bazične operacije za rad sa sirovim bajtovima
    void read(uint32_t address, void* dest, size_t size);
    void write(uint32_t address, const void* src, size_t size);
    void flush(); // Prisilno pražnjenje keša na disk

    // Vraća tačnu veličinu svih komprimovanih podataka u bajtovima
    size_t getCompressedSize();

    // Vraća stvarni algebarski faktor kompresije (ne zavisi od sektora čipa)
    float getCompressionRatio(size_t totalVirtualSize);

    // Templatizovana funkcija za brz upis bilo kojeg tipa/strukture
    template <typename T>
    void put(uint32_t address, const T& value) {
        write(address, &value, sizeof(T));
    }

    // Templatizovana funkcija za brzo čitanje bilo kojeg tipa/strukture
    template <typename T>
    T get(uint32_t address) {
        T value;
        read(address, &value, sizeof(T));
        return value;
    }
};

#endif // VIRTUAL_MEMORY_ENGINE_H
