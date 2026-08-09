#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

// Struktura telemetrijskog paketa (64 bajta po zapisu)
struct TelemetryRecord {
    uint64_t timestamp;
    uint32_t vehicleId;
    float speedKmh;
    float engineRPM;
    float fuelConsumption;
    float latitude;
    float longitude;
    char canDataPayload[32];
};

// Funkcija koja čita stvarni fizički RAM (RSS) koji proces trenutno troši na Linuxu
size_t getProcessPhysicalRAMBytes() {
    std::ifstream stat_stream("/proc/self/statm", std::ios_base::in);
    size_t pages = 0, rssPages = 0;
    if (stat_stream >> pages >> rssPages) {
        return rssPages * sysconf(_SC_PAGESIZE);
    }
    return 0;
}

int main() {
    std::cout << "\n========================================================" << std::endl;
    std::cout << "🔥 SYNTETIKA UNIVERSE — V-RAM ENGINE (CE DEMO)" << std::endl;
    std::cout << "========================================================\n" << std::endl;

    size_t initialRAM = getProcessPhysicalRAMBytes();
    std::cout << "📊 Initial Process RAM (RSS): " << initialRAM / 1024 << " KB" << std::endl;

    // 🔒 Prilagođeno za Community Edition limit (max 512 KB = 8000 * 64 bytes = 500 KB)
    const size_t RECORD_COUNT = 8000; 
    std::cout << "▶ Generating " << RECORD_COUNT << " telemetry records (~500 KB raw data)..." << std::endl;
    // Otvaramo fajl koji će naša hook biblioteka presresti
    const char* logFileName = "/tmp/telemetry.log";
    int fd = open(logFileName, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if (fd < 0) {
        std::cerr << "❌ Failed to open file!" << std::endl;
        return 1;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    // Upisujemo 500,000 zapisa u petlji
    TelemetryRecord rec;
    memset(&rec, 0, sizeof(rec));
    rec.vehicleId = 1001;
    strncpy(rec.canDataPayload, "CAN_BUS_NORMAL_OPERATING_MODE", 32);

    for (size_t i = 0; i < RECORD_COUNT; i++) {
        rec.timestamp = i * 10;
        rec.speedKmh = 80.0f + (i % 20) * 0.5f;
        rec.engineRPM = 2000.0f + (i % 100) * 5.0f;
        rec.fuelConsumption = 6.5f + (i % 10) * 0.1f;
        rec.latitude = 44.8186f + (i % 50) * 0.0001f;
        rec.longitude = 20.4568f + (i % 50) * 0.0001f;

        ssize_t res = write(fd, &rec, sizeof(TelemetryRecord));
        (void)res;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double durationSec = std::chrono::duration<double>(endTime - startTime).count();

    close(fd);

    size_t finalRAM = getProcessPhysicalRAMBytes();
    size_t totalRawBytes = RECORD_COUNT * sizeof(TelemetryRecord);

    std::cout << "\n========================================================" << std::endl;
    std::cout << "🏁 V-RAM CE BENCHMARK COMPLETED" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "✓ Total raw data processed: " << totalRawBytes / 1024.0 << " KB" << std::endl;
    std::cout << "✓ Processing time: " << durationSec << " seconds" << std::endl;
    std::cout << "✓ Write throughput: " << (totalRawBytes / 1024.0) / durationSec << " KB/s" << std::endl;
    std::cout << "✓ Final Process RAM (RSS): " << finalRAM / 1024 << " KB" << std::endl;
    std::cout << "========================================================\n" << std::endl;

    return 0;
}