#ifndef MANDEL_MANDEL_H
#define MANDEL_MANDEL_H

#include <vector>
#include <map>
#include <string>
#include <memory>

#include "MandelUtil.h"
#include "Generators.h"
#include "CpuGenerators.h"
#include "Hardware.h"

//#include "Fixedp.h"

namespace mnd
{
    enum class GeneratorType;
    class MandelContext;
    class MandelDevice;

    extern MandelContext initializeContext(void);
}


enum class mnd::GeneratorType
{
    FLOAT,
    FLOAT_SSE2,
    FLOAT_AVX,
    FLOAT_AVX512,
    FLOAT_NEON,
    DOUBLE,
    DOUBLE_SSE2,
    DOUBLE_AVX,
    DOUBLE_AVX512,
    DOUBLE_NEON,
    DOUBLE_DOUBLE,
    QUAD_DOUBLE,
    FLOAT128,
    FLOAT256
};


class mnd::MandelDevice
{
private:
    friend class MandelContext;

    std::string vendor;
    std::string name;

    std::map<GeneratorType, std::unique_ptr<Generator>> generators;

    MandelDevice(void);
public:

    inline const std::string& getVendor(void) const { return vendor; }
    inline const std::string& getName(void) const { return name; }

    Generator* getGenerator(GeneratorType type) const;
    //Generator* getGeneratorQuad(bool smooth = true) const;
    //Generator* getGenerator128(bool smooth = true) const;
};


class mnd::MandelContext
{
private:
    friend MandelContext mnd::initializeContext(void);

    CpuInfo cpuInfo;

    std::map<GeneratorType, std::unique_ptr<Generator>> cpuGenerators;

    std::unique_ptr<AdaptiveGenerator> adaptiveGenerator;

    std::vector<MandelDevice> devices;

    MandelContext(void);

    std::unique_ptr<AdaptiveGenerator> createAdaptiveGenerator(void);
    std::vector<MandelDevice> createDevices(void);
public:

    Generator& getDefaultGenerator(bool smooth = true);
    const std::vector<MandelDevice>& getDevices(void);

    Generator* getCpuGenerator(mnd::GeneratorType type);

    const CpuInfo& getCpuInfo(void) const { return cpuInfo; }
};


#endif // MANDEL_MANDEL_H
