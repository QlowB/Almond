#include "Hardware.h"

#include <array>
#include <vector>
#include <bitset>
#include <cstring>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
#ifdef __GNUC__
#include <cpuid.h>
#else
#include <intrin.h>
#endif
#endif

using mnd::CpuInfo;


#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86) 
CpuInfo::CpuInfo(void) :
    sse2{ false },
    avx{ false },
    avx2{ false },
    fma{ false },
    avx512{ false },
    neon{ false }
{


#if defined(__x86_64__) || defined(_M_X64)
    arch = Arch::X86_64;
#else
    arch = Arch::X86;
#endif

    std::array<unsigned int, 4> dat;
    std::vector<std::array<unsigned int, 4>> cpuData;
    std::vector<std::array<unsigned int, 4>> extData;

    unsigned int nData;
    unsigned int nExtData;

#ifdef __GNUC__
    __get_cpuid(0, &dat[0], &dat[1], &dat[2], &dat[3]);
    nData = dat[0];
    __get_cpuid(0x80000000, &dat[0], &dat[1], &dat[2], &dat[3]);
    nExtData = dat[0];
#else
    __cpuid((int*) dat.data(), 0);
    nData = dat[0];
    __cpuid((int*) dat.data(), 0x80000000);
    nExtData = dat[0];
#endif

    for (unsigned int i = 0; i <= nData; i++) {
#ifdef __GNUC__
        __get_cpuid_count(i, 0, &dat[0], &dat[1], &dat[2], &dat[3]);
#else
        __cpuidex((int*) dat.data(), i, 0);
#endif
        cpuData.push_back(dat);
    }

    for (unsigned int i = 0x80000000; i <= nExtData; i++) {
#ifdef __GNUC__
        __get_cpuid_count(i, 0, &dat[0], &dat[1], &dat[2], &dat[3]);
#else
        __cpuidex((int*) dat.data(), i, 0);
#endif
        extData.push_back(dat);
    }


    char vendor[32];
    memset(vendor, 0, sizeof vendor);
    *reinterpret_cast<int*>(vendor) = cpuData[0][1];
    *reinterpret_cast<int*>(vendor + 4) = cpuData[0][3];
    *reinterpret_cast<int*>(vendor + 8) = cpuData[0][2];
    this->vendor = vendor;

    char brand[64];
    memset(brand, 0, sizeof brand);

    if (nExtData >= 0x80000004) {
        memcpy(brand, extData[2].data(), sizeof(dat));
        memcpy(brand + 16, extData[3].data(), sizeof(dat));
        memcpy(brand + 32, extData[4].data(), sizeof(dat));
        this->brand = brand;

        // trim
        this->brand.erase(0, this->brand.find_first_not_of(" \n\r\t"));
        this->brand.erase(this->brand.find_last_not_of(" \n\r\t") + 1);
    }

    std::bitset<32> ecx1 = 0;
    std::bitset<32> edx1 = 0;
    std::bitset<32> ebx7 = 0;
    std::bitset<32> ecx7 = 0;

    if (nData >= 1) {
        ecx1 = cpuData[1][2];
        edx1 = cpuData[1][3];
    }
    if (nData >= 7) {
        ebx7 = cpuData[7][1];
        ecx7 = cpuData[7][2];
    }

    sse2 = edx1[26];
    avx = ecx1[28];
    fma = ecx1[12];
    avx2 = ebx7[5];
    avx512 = ebx7[16];
}

#elif defined(__arm__) || defined(__aarch64__)

CpuInfo::CpuInfo(void) :
    sse2{ false },
    avx{ false },
    avx512{ false },
    neon{ false }
{
#if defined(__aarch64__)
    arch = Arch::ARM64;
#else
    arch = Arch::ARM;
#endif
    // TODO implement check
    neon = (arch == Arch::ARM64);
}
#endif
