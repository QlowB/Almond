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

    bool avx;
public:
    CpuInfo(void);

    inline const std::string& getVendor(void) const { return vendor; };
    inline const std::string& getBrand(void) const { return brand; };

    inline bool hasAvx(void) const { return avx; };
};

#endif // MANDEL_HARDWARE_H
