#include "Mandel.h"
#include "Fixed.h"

#include "CpuGenerators.h"
#include "ClGenerators.h"

#include <map>

using mnd::MandelDevice;
using mnd::MandelContext;
using mnd::Generator;
using mnd::AdaptiveGenerator;

template<typename T, typename U>
static std::map<U, T> invertMap(const std::map<T, U>& m)
{
    std::map<U, T> res;
    std::transform(m.begin(), m.end(), std::inserter(res, res.end()), [](auto& pair) {
        return std::pair{ pair.second, pair.first };
    });
    return res;
}


static const std::map<mnd::GeneratorType, std::string> typeNames =
{
    { mnd::GeneratorType::FLOAT, "float" },
    { mnd::GeneratorType::FLOAT_SSE2, "float SSE2" },
    { mnd::GeneratorType::FLOAT_AVX, "float AVX" },
    { mnd::GeneratorType::FLOAT_AVX512, "float AVX512" },
    { mnd::GeneratorType::FLOAT_NEON, "float NEON" },
    { mnd::GeneratorType::DOUBLE, "double" },
    { mnd::GeneratorType::DOUBLE_SSE2, "double SSE2" },
    { mnd::GeneratorType::DOUBLE_AVX, "double AVX" },
    { mnd::GeneratorType::DOUBLE_AVX512, "double AVX512" },
    { mnd::GeneratorType::DOUBLE_NEON, "double NEON" },
    { mnd::GeneratorType::DOUBLE_DOUBLE, "double double" },
    { mnd::GeneratorType::DOUBLE_DOUBLE_AVX, "double double AVX" },
    { mnd::GeneratorType::DOUBLE_DOUBLE_AVX_FMA, "double double AVX+FMA" },
    { mnd::GeneratorType::QUAD_DOUBLE, "quad double" },
    { mnd::GeneratorType::FLOAT128, "float128" },
    { mnd::GeneratorType::FLOAT256, "float256" },
    { mnd::GeneratorType::FIXED512, "fixed512" },
};


static const std::map<std::string, mnd::GeneratorType> nameTypes = invertMap(typeNames);


namespace mnd
{

    const std::string& getGeneratorName(mnd::GeneratorType type)
    {
        return typeNames.at(type);
    }


    mnd::GeneratorType getTypeFromName(const std::string& name)
    {
        return nameTypes.at(name);
    }

}


MandelContext mnd::initializeContext(void)
{
    MandelContext context = MandelContext();
    return context;
}


MandelDevice::MandelDevice(void)
{
}


mnd::Generator* MandelDevice::getGenerator(mnd::GeneratorType type) const
{
    auto it = generators.find(type);
    if (it != generators.end())
        return it->second.get();
    else
        return nullptr;
}


std::vector<mnd::GeneratorType> MandelDevice::getSupportedTypes(void) const
{
    std::vector<GeneratorType> types;
    for (auto& [type, gen] : generators) {
        types.push_back(type);
    }
    return types;
}


MandelContext::MandelContext(void)
{

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86) 
    if (cpuInfo.hasAvx()) {
        auto fl = std::make_unique<CpuGenerator<float, mnd::X86_AVX, true>>();
        auto db = std::make_unique<CpuGenerator<double, mnd::X86_AVX, true>>();
        auto ddb = std::make_unique<CpuGenerator<DoubleDouble, mnd::X86_AVX, true>>();
        cpuGenerators.insert({ GeneratorType::FLOAT_AVX, std::move(fl) });
        cpuGenerators.insert({ GeneratorType::DOUBLE_AVX, std::move(db) });
        cpuGenerators.insert({ GeneratorType::DOUBLE_DOUBLE_AVX, std::move(ddb) });
        if (cpuInfo.hasFma()) {
            auto ddavx = std::make_unique<CpuGenerator<DoubleDouble, mnd::X86_AVX_FMA, true>>();
            cpuGenerators.insert({ GeneratorType::DOUBLE_DOUBLE_AVX_FMA, std::move(ddavx) });
        }
    }
    if (cpuInfo.hasSse2()) {
        auto fl = std::make_unique<CpuGenerator<float, mnd::X86_SSE2, true>>();
        auto db = std::make_unique<CpuGenerator<double, mnd::X86_SSE2, true>>();
        cpuGenerators.insert({ GeneratorType::FLOAT_SSE2, std::move(fl) });
        cpuGenerators.insert({ GeneratorType::DOUBLE_SSE2, std::move(db) });
    }
#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) 
    if (cpuInfo.hasNeon()) {
        auto fl = std::make_unique<CpuGenerator<float, mnd::ARM_NEON, true>>();
        auto db = std::make_unique<CpuGenerator<double, mnd::ARM_NEON, true>>();
        cpuGenerators.insert({ GeneratorType::FLOAT_NEON, std::move(fl) });
        cpuGenerators.insert({ GeneratorType::DOUBLE_NEON, std::move(db) });
    }
#endif
    {
        auto fl = std::make_unique<CpuGenerator<float, mnd::NONE, true>>();
        auto db = std::make_unique<CpuGenerator<double, mnd::NONE, true>>();
        cpuGenerators.insert({ GeneratorType::FLOAT, std::move(fl) });
        cpuGenerators.insert({ GeneratorType::DOUBLE, std::move(db) });
    }

#ifdef WITH_BOOST
    auto quad = std::make_unique<CpuGenerator<Float128, mnd::NONE, true>>();
    auto oct = std::make_unique<CpuGenerator<Float256, mnd::NONE, true>>();
    cpuGenerators.insert({ GeneratorType::FLOAT128, std::move(quad) });
    cpuGenerators.insert({ GeneratorType::FLOAT256, std::move(oct) });
#endif // WITH_BOOST

    auto dd = std::make_unique<CpuGenerator<DoubleDouble, mnd::NONE, true>>();
    auto qd = std::make_unique<CpuGenerator<QuadDouble, mnd::NONE, true>>();
    cpuGenerators.insert({ GeneratorType::DOUBLE_DOUBLE, std::move(dd) });
    cpuGenerators.insert({ GeneratorType::QUAD_DOUBLE, std::move(qd) });

    auto fix512 = std::make_unique<CpuGenerator<Fixed512, mnd::NONE, true>>();
    cpuGenerators.insert({ GeneratorType::FIXED512, std::move(fix512) });

    devices = createDevices();

    adaptiveGenerator = createAdaptiveGenerator();
}


std::unique_ptr<mnd::AdaptiveGenerator> MandelContext::createAdaptiveGenerator(void)
{
    auto* floatGen = getCpuGenerator(GeneratorType::FLOAT);
    auto* doubleGen = getCpuGenerator(GeneratorType::DOUBLE);
    auto* doubleDoubleGen = getCpuGenerator(GeneratorType::DOUBLE_DOUBLE);
    auto* quadDoubleGen = getCpuGenerator(GeneratorType::QUAD_DOUBLE);
    auto* f256Gen = getCpuGenerator(GeneratorType::FLOAT256);

    if (cpuInfo.hasAvx()) {
        floatGen = getCpuGenerator(GeneratorType::FLOAT_AVX);
        doubleGen = getCpuGenerator(GeneratorType::DOUBLE_AVX);
    }
    else if (cpuInfo.hasSse2()) {
        floatGen = getCpuGenerator(GeneratorType::FLOAT_SSE2);
        doubleGen = getCpuGenerator(GeneratorType::DOUBLE_SSE2);
    }
    if (cpuInfo.hasAvx() && cpuInfo.hasFma()) {
        doubleDoubleGen = getCpuGenerator(GeneratorType::DOUBLE_DOUBLE_AVX_FMA);
    }

    if (cpuInfo.hasNeon()) {
        floatGen = getCpuGenerator(GeneratorType::FLOAT_NEON);
        doubleGen = getCpuGenerator(GeneratorType::DOUBLE_NEON);
    }

    if (!devices.empty()) {
        auto& device = devices[0];
        auto* fGen = device.getGenerator(GeneratorType::FLOAT);
        auto* dGen = device.getGenerator(GeneratorType::DOUBLE);
        auto* ddGen = device.getGenerator(GeneratorType::DOUBLE_DOUBLE);

        if (fGen)
            floatGen = fGen;
        if (dGen)
            doubleGen = dGen;
        if (ddGen)
            doubleDoubleGen = ddGen;
    }

    auto ag = std::make_unique<AdaptiveGenerator>();
    ag->addGenerator(Precision::FLOAT, *floatGen);
    ag->addGenerator(Precision::DOUBLE, *doubleGen);
    ag->addGenerator(Precision::DOUBLE_DOUBLE, *doubleDoubleGen);
    ag->addGenerator(Precision::QUAD_DOUBLE, *quadDoubleGen);
    ag->addGenerator(Precision::FLOAT256, *f256Gen);
    ag->addGenerator(Precision::INF_PREC, *f256Gen);

    return ag;
}


std::vector<MandelDevice> MandelContext::createDevices(void)
{
    std::vector<MandelDevice> mandelDevices;
#ifdef WITH_OPENCL
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    //platforms.erase(platforms.begin() + 1);

    for (auto& platform : platforms) {
        std::string name = platform.getInfo<CL_PLATFORM_NAME>();
        std::string profile = platform.getInfo<CL_PLATFORM_PROFILE>();

        //printf("using opencl platform: %s\n", name.c_str());

        //std::string ext = platform.getInfo<CL_PLATFORM_EXTENSIONS>();
        //printf("Platform extensions: %s\n", ext.c_str());
        //printf("Platform: %s, %s\n", name.c_str(), profile.c_str());

        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
        for (auto& device : devices) {
            //printf("Device: %s\n", device.getInfo<CL_DEVICE_NAME>().c_str());
            //printf("preferred float width: %d\n", device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT>());
            //printf("vendor: %s\n", device.getInfo<CL_DEVICE_VENDOR>().c_str());

            std::string extensions = device.getInfo<CL_DEVICE_EXTENSIONS>();
            auto supportsDouble = extensions.find("cl_khr_fp64") != std::string::npos;

            //printf("Device extensions: %s\n", ext.c_str());
            MandelDevice md;

            //printf("clock: %d", device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>());

            md.name = device.getInfo<CL_DEVICE_NAME>();
            md.vendor = device.getInfo<CL_DEVICE_VENDOR>();
            //printf("    using opencl device: %s\n", md.name.c_str());
            try {
                md.generators.insert({ GeneratorType::FLOAT, std::make_unique<ClGeneratorFloat>(device) });
            }
            catch (const std::string& err) {
                printf("err: %s", err.c_str());
            }

            if (supportsDouble) {
                try {
                    md.generators.insert({ GeneratorType::DOUBLE, std::make_unique<ClGeneratorDouble>(device) });
                    md.generators.insert({ GeneratorType::DOUBLE_DOUBLE, std::make_unique<ClGeneratorDoubleDouble>(device) });
                    md.generators.insert({ GeneratorType::QUAD_DOUBLE, std::make_unique<ClGeneratorQuadDouble>(device) });
                }
                catch (const std::string& err) {
                    printf("err: %s", err.c_str());
                    fflush(stdout);
                }
            }

            try {
                //md.generator128 = std::make_unique<ClGenerator128>(device);
            }
            catch (const std::string& err) {
                //fprintf(stderr, "error creating 128bit cl generator: %s\n", err.c_str());
            }

            mandelDevices.push_back(std::move(md));
        }
    }
#endif // WITH_OPENCL
    
    return mandelDevices;
}


AdaptiveGenerator& MandelContext::getDefaultGenerator(void)
{
    return *adaptiveGenerator;
}


const std::vector<MandelDevice>& MandelContext::getDevices(void)
{
    return devices;
}


Generator* MandelContext::getCpuGenerator(mnd::GeneratorType type)
{
    auto it = cpuGenerators.find(type);
    if (it != cpuGenerators.end())
        return it->second.get();
    else
        return nullptr;
}


std::vector<mnd::GeneratorType> MandelContext::getSupportedTypes(void) const
{
    std::vector<GeneratorType> types;
    for (auto& [type, gen] : cpuGenerators) {
        types.push_back(type);
    }
    return types;
}
