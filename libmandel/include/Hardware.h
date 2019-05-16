#ifndef MANDEL_HARDWARE_H
#define MANDEL_HARDWARE_H

#include <string>

namespace mnd
{
    class CpuInfo;
}

class mnd::CpuInfo
{
    std::string vendor;
    std::string brand;

    bool sse2;
    bool avx;
    bool avx512;
public:
    CpuInfo(void);

    inline const std::string& getVendor(void) const { return vendor; };
    inline const std::string& getBrand(void) const { return brand; };

    inline bool hasSse2(void) const { return sse2; };
    inline bool hasAvx(void) const { return avx; };
    inline bool hasAvx512(void) const { return avx512; };
};

#endif // MANDEL_HARDWARE_H
