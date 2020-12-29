#ifndef MANDEL_MANDEL_H
#define MANDEL_MANDEL_H

// don't expose this library interface as it clashes with qt
//#include <asmjit/asmjit.h>
namespace asmjit { class JitRuntime; }

#ifndef WITH_ASMJIT
// if no asmjit, use dummy implementation
namespace asmjit { class JitRuntime{}; }
#endif // WITH_ASMJITH

#include <vector>
#include <map>
#include <string>
#include <memory>


#include "MandelUtil.h"
#include "Generators.h"
#include "IterationGenerator.h"
#include "CpuGenerators.h"
#include "Hardware.h"

namespace mnd
{
    class MandelContext;
    class MandelDevice;

    struct ClDeviceWrapper;

    extern MandelContext initializeContext(void);

    struct MandelContextCache;
}


class mnd::MandelDevice
{
private:
    friend class MandelContext;

    std::string platformName;
    std::string vendor;
    std::string name;
    std::string extensions;
    std::unique_ptr<ClDeviceWrapper> clDevice;

    std::map<Precision, std::unique_ptr<MandelGenerator>> mandelGenerators;

public:
    MandelDevice(ClDeviceWrapper, const std::string& platformName);
    MandelDevice(const MandelDevice&) = delete;
    MandelDevice(MandelDevice&&) = default;
    MandelDevice& operator=(const MandelDevice&) = delete;
    MandelDevice& operator=(MandelDevice&&) = default;

    inline const std::string& getVendor(void) const { return vendor; }
    inline const std::string& getName(void) const { return name; }

    MandelGenerator* getGenerator(Precision type) const;
    inline ClDeviceWrapper& getClDevice(void) { return *clDevice; }
    inline const ClDeviceWrapper& getClDevice(void) const { return *clDevice; }

    std::vector<Precision> getSupportedTypes(void) const;
    bool supportsDouble(void) const;
};


class mnd::MandelContext
{
private:
    friend MandelContext mnd::initializeContext(void);

    using GeneratorType = std::pair<Precision, HardwareFeature>;

    CpuInfo cpuInfo;
    std::unique_ptr<asmjit::JitRuntime> jitRuntime;

    std::map<GeneratorType, std::unique_ptr<MandelGenerator>> cpuGenerators;

    std::unique_ptr<AdaptiveGenerator> adaptiveGenerator;

    std::vector<std::unique_ptr<mnd::MandelDevice>> devices;

    MandelContext(void);

    std::unique_ptr<AdaptiveGenerator> createAdaptiveGenerator(void);
    std::vector<std::unique_ptr<mnd::MandelDevice>> createDevices(void);
public:
    ~MandelContext(void);
    MandelContext(const MandelContext&) = delete;
    MandelContext(MandelContext&&) = default;
    MandelContext& operator=(const MandelContext&) = delete;
    MandelContext& operator=(MandelContext&&) = default;

    AdaptiveGenerator& getDefaultGenerator(void);
    std::vector<std::unique_ptr<mnd::MandelDevice>>& getDevices(void);

    asmjit::JitRuntime& getJitRuntime(void);

    MandelGenerator* getCpuGenerator(mnd::Precision type, mnd::HardwareFeature ex);
    std::vector<GeneratorType> getSupportedTypes(void) const;
    std::vector<MandelGenerator*> getCpuGenerators(mnd::Precision prec) const;

    const CpuInfo& getCpuInfo(void) const { return cpuInfo; }

    void saveCache(const std::string& path) const;
    static MandelContext initializeFromCache(const std::string& path);
};


struct mnd::MandelContextCache
{
};


#endif // MANDEL_MANDEL_H

