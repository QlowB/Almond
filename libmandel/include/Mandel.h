#ifndef MANDEL_MANDEL_H
#define MANDEL_MANDEL_H

#include <vector>
#include <string>
#include <memory>

#include "MandelUtil.h"
#include "Generators.h"
#include "Hardware.h"

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
    MandelDevice(void);
public:

    inline const std::string& getVendor(void) const { return vendor; }
    const std::string& getName(void) const;

    Generator* getGeneratorFloat(void) const;
    Generator* getGeneratorDouble(void) const;
};


class mnd::MandelContext
{
private:
    friend MandelContext initializeContext(void);

    CpuInfo cpuInfo;

    std::unique_ptr<Generator> cpuGeneratorFloat;
    std::unique_ptr<Generator> cpuGeneratorDouble;

    std::vector<MandelDevice> devices;

    MandelContext(void);

    std::vector<MandelDevice> createDevices(void);
public:

    Generator& getDefaultGenerator(void);
    const std::vector<MandelDevice>& getDevices(void);

    Generator& getCpuGeneratorFloat(void);
    Generator& getCpuGeneratorDouble(void);

    const CpuInfo& getCpuInfo(void) const { return cpuInfo; }
};


#endif // MANDEL_MANDEL_H