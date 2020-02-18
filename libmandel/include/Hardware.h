#ifndef MANDEL_HARDWARE_H
#define MANDEL_HARDWARE_H

#include <string>

namespace mnd
{
    class CpuInfo;
}

class mnd::CpuInfo
{
public:
    enum class Arch
    {
        X86,
        X86_64,
        ARM,
        ARM64
    };
private:
    std::string vendor;
    std::string brand;

    Arch arch;

    bool sse2;
    bool avx;
    bool fma;
    bool avx512;
    bool neon;
public:
    CpuInfo(void);

    inline const std::string& getVendor(void) const { return vendor; };
    inline const std::string& getBrand(void) const { return brand; };


//#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    inline bool hasSse2(void) const { return sse2; };
    inline bool hasAvx(void) const { return avx; };
    inline bool hasFma(void) const { return fma; };
    inline bool hasAvx512(void) const { return avx512; };
//#elif defined(__arm__) || defined(__aarch64__)
    inline bool hasNeon(void) const { return neon; };
//#endif
};

#endif // MANDEL_HARDWARE_H
