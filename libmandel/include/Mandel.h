#ifndef MANDEL_MANDEL_H
#define MANDEL_MANDEL_H

#include <vector>
#include <string>
#include <memory>

#include "MandelUtil.h"
#include "Generators.h"
#include "CpuGenerators.h"
#include "Hardware.h"

//#include "Fixedp.h"

namespace mnd
{
    class MandelContext;
    class MandelDevice;

    extern MandelContext initializeContext(void);
}


class mnd::MandelDevice
{
private:
    friend class MandelContext;

    std::string vendor;
    std::string name;

    std::unique_ptr<Generator> floatGenerator;
    std::unique_ptr<Generator> doubleGenerator;
    std::unique_ptr<Generator> doubleDoubleGenerator;
    //std::unique_ptr<Generator> quadGenerator;
    //std::unique_ptr<Generator> generator128;

    std::unique_ptr<Generator> floatGeneratorSmooth;
    std::unique_ptr<Generator> doubleGeneratorSmooth;
    std::unique_ptr<Generator> doubleDoubleGeneratorSmooth;
    //std::unique_ptr<Generator> quadGeneratorSmooth;
    //std::unique_ptr<Generator> generator128Smooth;

    MandelDevice(void);
public:

    inline const std::string& getVendor(void) const { return vendor; }
    const std::string& getName(void) const;

    Generator* getGeneratorFloat(bool smooth = true) const;
    Generator* getGeneratorDouble(bool smooth = true) const;
    Generator* getGeneratorDoubleDouble(bool smooth = true) const;
    //Generator* getGeneratorQuad(bool smooth = true) const;
    //Generator* getGenerator128(bool smooth = true) const;
};


class mnd::MandelContext
{
private:
    friend MandelContext mnd::initializeContext(void);

    CpuInfo cpuInfo;

    std::unique_ptr<Generator> cpuGeneratorFloat;
    std::unique_ptr<Generator> cpuGeneratorDouble;
    std::unique_ptr<Generator> cpuGeneratorQuad;
    std::unique_ptr<Generator> cpuGeneratorOct;
    std::unique_ptr<Generator> cpuGenerator128;
    std::unique_ptr<Generator> cpuGeneratorDD;
    std::unique_ptr<Generator> cpuGeneratorQD;

    std::unique_ptr<Generator> cpuGeneratorFloatSmooth;
    std::unique_ptr<Generator> cpuGeneratorDoubleSmooth;
    std::unique_ptr<Generator> cpuGeneratorQuadSmooth;
    std::unique_ptr<Generator> cpuGenerator128Smooth;
    std::unique_ptr<Generator> cpuGeneratorDDSmooth;
    std::unique_ptr<Generator> cpuGeneratorQDSmooth;

    std::unique_ptr<AdaptiveGenerator> adaptiveGenerator;
    std::unique_ptr<AdaptiveGenerator> adaptiveGeneratorSmooth;

    std::vector<MandelDevice> devices;

    MandelContext(void);

    std::vector<MandelDevice> createDevices(void);
public:

    Generator& getDefaultGenerator(bool smooth = true);
    const std::vector<MandelDevice>& getDevices(void);

    Generator& getCpuGeneratorFloat(void);
    Generator& getCpuGeneratorDouble(void);
    Generator* getCpuGeneratorQuad(void);
    Generator* getCpuGeneratorOct(void);
    Generator* getCpuGenerator128(void);
    Generator* getCpuGeneratorDD(void);
    Generator* getCpuGeneratorQD(void);

    const CpuInfo& getCpuInfo(void) const { return cpuInfo; }
};


#endif // MANDEL_MANDEL_H
