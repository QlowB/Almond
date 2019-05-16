#include "Hardware.h"

#include <array>
#include <vector>
#include <bitset>
#include <cstring>

#ifdef __GNUC__
#include <cpuid.h>
#else
#include <intrin.h>
#endif

using mnd::CpuInfo;


CpuInfo::CpuInfo(void)
{
    std::array<unsigned int, 4> dat;
    std::vector<std::array<unsigned int, 4>> cpuData;
    std::vector<std::array<unsigned int, 4>> extData;

    unsigned int nData;
    unsigned int nExtData;

#ifdef __GNUC__
    __cpuid(0, dat[0], dat[1], dat[2], dat[3]);
    nData = dat[0];
    __cpuid(0x80000000, dat[0], dat[1], dat[2], dat[3]);
    nExtData = dat[0];
#else
    __cpuid(dat.data(), 0);
    nData = dat[0];
    __cpuid(dat.data(), 0x80000000);
    nExtData = dat[0];
#endif

    for (int i = 0; i <= nData; i++) {
#ifdef __GNUC__
        __get_cpuid(i, &dat[0], &dat[1], &dat[2], &dat[3]);
#else
        __cpuidex(dat.data(), i, 0);
#endif
        cpuData.push_back(dat);
    }

    for (int i = 0x80000000; i <= nExtData; i++) {
#ifdef __GNUC__
        __get_cpuid(i, &dat[0], &dat[1], &dat[2], &dat[3]);
#else
        __cpuidex(dat.data(), i, 0);
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

    std::bitset<32> ecx1;
    std::bitset<32> edx1;

    if (nData >= 1) {
        ecx1 = cpuData[1][2];
        edx1 = cpuData[1][3];
    }

    avx = ecx1[28];
}

