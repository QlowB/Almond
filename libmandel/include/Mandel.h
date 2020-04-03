#ifndef MANDEL_MANDEL_H
#define MANDEL_MANDEL_H

// don't expose this library interface as it clashes with qt
//#include <asmjit/asmjit.h>
namespace asmjit { class JitRuntime; }

#include <vector>
#include <map>
#include <string>
#include <memory>


#include "MandelUtil.h"
#include "Generators.h"
#include "IterationGenerator.h"
#include "CpuGenerators.h"
#include "Hardware.h"

//#include "Fixedp.h"

namespace mnd
{
    class MandelContext;
    class MandelDevice;

    struct ClDeviceWrapper;

    extern MandelContext initializeContext(void);

    const std::string& getGeneratorName(mnd::GeneratorType);
    GeneratorType getTypeFromName(const std::string& name);
}


class mnd::MandelDevice
{
private:
    friend class MandelContext;

    std::string vendor;
    std::string name;
    std::unique_ptr<ClDeviceWrapper> clDevice;

    std::map<GeneratorType, std::unique_ptr<MandelGenerator>> mandelGenerators;

    MandelDevice(ClDeviceWrapper);
public:
    MandelDevice(const MandelDevice&) = delete;
    MandelDevice(MandelDevice&&) = default;
    MandelDevice& operator=(const MandelDevice&) = delete;
    MandelDevice& operator=(MandelDevice&&) = default;

    inline const std::string& getVendor(void) const { return vendor; }
    inline const std::string& getName(void) const { return name; }

    MandelGenerator* getGenerator(GeneratorType type) const;
    inline ClDeviceWrapper& getClDevice(void) { return *clDevice; }
    inline const ClDeviceWrapper& getClDevice(void) const { return *clDevice; }

    std::vector<GeneratorType> getSupportedTypes(void) const;
};


class mnd::MandelContext
{
private:
    friend MandelContext mnd::initializeContext(void);

    CpuInfo cpuInfo;
    std::unique_ptr<asmjit::JitRuntime> jitRuntime;

    std::map<GeneratorType, std::unique_ptr<MandelGenerator>> cpuGenerators;

    std::unique_ptr<AdaptiveGenerator> adaptiveGenerator;

    std::vector<MandelDevice> devices;

    MandelContext(void);

    std::unique_ptr<AdaptiveGenerator> createAdaptiveGenerator(void);
    std::vector<MandelDevice> createDevices(void);
public:
    ~MandelContext(void);
    MandelContext(const MandelContext&) = delete;
    MandelContext(MandelContext&&) = default;
    MandelContext& operator=(const MandelContext&) = delete;
    MandelContext& operator=(MandelContext&&) = default;

    AdaptiveGenerator& getDefaultGenerator(void);
    std::vector<MandelDevice>& getDevices(void);

    asmjit::JitRuntime& getJitRuntime(void);

    MandelGenerator* getCpuGenerator(mnd::GeneratorType type);
    std::vector<GeneratorType> getSupportedTypes(void) const;

    const CpuInfo& getCpuInfo(void) const { return cpuInfo; }
};


#endif // MANDEL_MANDEL_H

