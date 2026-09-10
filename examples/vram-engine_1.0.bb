SUMMARY = "Syntetika Universe S-OS V-RAM Engine for Yocto Linux"
DESCRIPTION = "High-performance compressed virtual memory engine for Bosch TCU gateways."
LICENSE = "PROPRIETARY"
LIC_FILES_CHKSUM = "file://LICENSE;md5=xxxx"

SRC_URI = "file://VirtualMemoryEngine.cpp \
           file://VirtualMemoryEngine.h \
           file://CMakeLists.txt \
           file://LICENSE"

S = "${WORKDIR}"

inherit cmake

# Podrška za C++11 i POSIX nitovanje na Bosch TCU ARM procesorima
EXTRA_OECMAKE = ""

FILES_${PN} += "${libdir}/libvram-engine.so"