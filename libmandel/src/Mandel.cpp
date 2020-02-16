#include "Mandel.h"
#include "Fixed.h"

#include "CpuGenerators.h"
#include "ClGenerators.h"

using mnd::MandelDevice;
using mnd::MandelContext;
using mnd::Generator;
using mnd::AdaptiveGenerator;



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


MandelContext::MandelContext(void)
{

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86) 
    if (cpuInfo.hasAvx()) {
        auto fl = std::make_unique<CpuGenerator<float, mnd::X86_AVX, true>>();
        auto db = std::make_unique<CpuGenerator<double, mnd::X86_AVX, true>>();
        cpuGenerators.insert({ GeneratorType::FLOAT_AVX, std::move(fl) });
        cpuGenerators.insert({ GeneratorType::DOUBLE_AVX512, std::move(db) });
    }
    else if (cpuInfo.hasSse2()) {
        auto fl = std::make_unique<CpuGenerator<float, mnd::X86_SSE2, true>>();
        auto db = std::make_unique<CpuGenerator<double, mnd::X86_SSE2, true>>();
        cpuGenerators.insert({ GeneratorType::FLOAT_SSE2, std::move(fl) });
        cpuGenerators.insert({ GeneratorType::DOUBLE_SSE2, std::move(db) });
    }
    else
#elif defined(__aarch64__)
    if (true) {
        auto fl = std::make_unique<CpuGenerator<float, mnd::ARM_NEON, true>>();
        auto db = std::make_unique<CpuGenerator<double, mnd::ARM_NEON, true>>();
        cpuGenerators.insert({ GeneratorType::FLOAT_NEON, std::move(fl) });
        cpuGenerators.insert({ GeneratorType::DOUBLE_NEON, std::move(db) });
    }
    else
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

#ifdef WITH_QD
    auto dd = std::make_unique<CpuGenerator<DoubleDouble, mnd::NONE, true>>();
    auto qd = std::make_unique<CpuGenerator<QuadDouble, mnd::NONE, true>>();
    cpuGenerators.insert({ GeneratorType::DOUBLE_DOUBLE, std::move(dd) });
    cpuGenerators.insert({ GeneratorType::DOUBLE_DOUBLE, std::move(qd) });
#endif // WITH_QD

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


Generator& MandelContext::getDefaultGenerator(bool smooth)
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
