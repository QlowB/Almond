#include "Mandel.h"
#include "Fixed.h"

#include "CpuGenerators.h"
#include "ClGenerators.h"
#include "OpenClInternal.h"
#include "OpenClCode.h"

#ifdef WITH_ASMJIT
#include <asmjit/asmjit.h>
#endif // WITH_ASMJIT

#include <map>
#include <array>

using mnd::MandelDevice;
using mnd::MandelContext;
using mnd::MandelGenerator;
using mnd::AdaptiveGenerator;


MandelContext mnd::initializeContext(void)
{
    return MandelContext();
}


MandelDevice::MandelDevice(mnd::ClDeviceWrapper device, const std::string& platformName) :
    clDevice{ std::make_unique<ClDeviceWrapper>(std::move(device)) },
    platformName{ platformName }
{
    extensions = clDevice->device.getInfo<CL_DEVICE_EXTENSIONS>();
    name = clDevice->device.getInfo<CL_DEVICE_NAME>();
    vendor = clDevice->device.getInfo<CL_DEVICE_VENDOR>();
}


mnd::MandelGenerator* MandelDevice::getGenerator(mnd::Precision type) const
{
    auto it = mandelGenerators.find(type);
    if (it != mandelGenerators.end())
        return it->second.get();
    else
        return nullptr;
}


std::vector<mnd::Precision> MandelDevice::getSupportedTypes(void) const
{
    std::vector<Precision> types;
    for (auto& [type, gen] : mandelGenerators) {
        types.push_back(type);
    }
    return types;
}


bool MandelDevice::supportsDouble(void) const
{
    return extensions.find("cl_khr_fp64") != std::string::npos;
}


MandelContext::MandelContext(void)
#ifdef WITH_ASMJIT
    : jitRuntime{ std::make_unique<asmjit::JitRuntime>() }
#endif // WITH_ASMJIT
{

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86) 
#   if defined(WITH_AVX512)
    if (cpuInfo.hasAvx512()) {
        auto fl = std::make_unique<CpuGenerator<float, mnd::X86_AVX_512, true>>();
        //auto db = std::make_unique<CpuGenerator<double, mnd::X86_AVX_512, true>>();
        cpuGenerators.insert({ { Precision::FLOAT, CpuExtension::X86_AVX_512 }, std::move(fl) });
        //cpuGenerators.insert({ { Precision::DOUBLE, CpuExtension::X86_AVX_512 }, std::move(db) });
    }
#   endif
    if (cpuInfo.hasAvx()) {
        auto fl = std::make_unique<CpuGenerator<float, mnd::X86_AVX, true>>();
        auto db = std::make_unique<CpuGenerator<double, mnd::X86_AVX, true>>();
        auto ddb = std::make_unique<CpuGenerator<DoubleDouble, mnd::X86_AVX, true>>();
        auto tdb = std::make_unique<CpuGenerator<TripleDouble, mnd::X86_AVX, true>>();
        cpuGenerators.insert({ { Precision::FLOAT, CpuExtension::X86_AVX }, std::move(fl) });
        cpuGenerators.insert({ { Precision::DOUBLE, CpuExtension::X86_AVX }, std::move(db) });
        cpuGenerators.insert({ { Precision::DOUBLE_DOUBLE, CpuExtension::X86_AVX }, std::move(ddb) });
        cpuGenerators.insert({ { Precision::TRIPLE_DOUBLE, CpuExtension::X86_AVX }, std::move(tdb) });
    }
    if (cpuInfo.hasAvx2() && cpuInfo.hasFma()) {
        auto favxfma = std::make_unique<CpuGenerator<float, mnd::X86_AVX_FMA, true>>();
        auto davxfma = std::make_unique<CpuGenerator<double, mnd::X86_AVX_FMA, true>>();
        auto ddavxfma = std::make_unique<CpuGenerator<DoubleDouble, mnd::X86_AVX_FMA, true>>();
        auto qdavxfma = std::make_unique<CpuGenerator<QuadDouble, mnd::X86_AVX_FMA, true>>();
        cpuGenerators.insert({ { Precision::FLOAT, CpuExtension::X86_AVX_FMA }, std::move(favxfma) });
        cpuGenerators.insert({ { Precision::DOUBLE, CpuExtension::X86_AVX_FMA }, std::move(davxfma) });
        cpuGenerators.insert({ { Precision::DOUBLE_DOUBLE, CpuExtension::X86_AVX_FMA }, std::move(ddavxfma) });
        cpuGenerators.insert({ { Precision::QUAD_DOUBLE, CpuExtension::X86_AVX_FMA }, std::move(qdavxfma) });
    }
    if (cpuInfo.hasSse2()) {
        auto fl = std::make_unique<CpuGenerator<float, mnd::X86_SSE2, true>>();
        auto db = std::make_unique<CpuGenerator<double, mnd::X86_SSE2, true>>();
        cpuGenerators.insert({ { Precision::FLOAT, CpuExtension::X86_SSE2 }, std::move(fl) });
        cpuGenerators.insert({ { Precision::DOUBLE, CpuExtension::X86_SSE2 }, std::move(db) });
    }
#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) 
    if (cpuInfo.hasNeon()) {
        auto fl = std::make_unique<CpuGenerator<float, mnd::ARM_NEON, true>>();
        auto db = std::make_unique<CpuGenerator<double, mnd::ARM_NEON, true>>();
        auto ddb = std::make_unique<CpuGenerator<mnd::DoubleDouble, mnd::ARM_NEON, true>>();
        cpuGenerators.insert({ { Precision::FLOAT, CpuExtension::ARM_NEON }, std::move(fl) });
        cpuGenerators.insert({ { Precision::DOUBLE, CpuExtension::ARM_NEON }, std::move(db) });
        cpuGenerators.insert({ { Precision::DOUBLE_DOUBLE, CpuExtension::ARM_NEON }, std::move(ddb) });
    }
#endif
    {
        auto fl = std::make_unique<CpuGenerator<float, mnd::NONE, true>>();
        auto db = std::make_unique<CpuGenerator<double, mnd::NONE, true>>();
        cpuGenerators.insert({ { Precision::FLOAT, CpuExtension::NONE }, std::move(fl) });
        cpuGenerators.insert({ { Precision::DOUBLE, CpuExtension::NONE }, std::move(db) });

        auto fx64 = std::make_unique<CpuGenerator<Fixed64, mnd::NONE, true>>();
        auto fx128 = std::make_unique<CpuGenerator<Fixed128, mnd::NONE, true>>();
        cpuGenerators.insert({ { Precision::FIXED64, CpuExtension::NONE }, std::move(fx64) });
        cpuGenerators.insert({ { Precision::FIXED128, CpuExtension::NONE }, std::move(fx128) });
    }

#ifdef WITH_BOOST
    auto quad = std::make_unique<CpuGenerator<Float128, mnd::NONE, true>>();
    auto oct = std::make_unique<CpuGenerator<Float256, mnd::NONE, true>>();
    cpuGenerators.insert({ { Precision::FLOAT128, CpuExtension::NONE }, std::move(quad) });
    cpuGenerators.insert({ { Precision::FLOAT256, CpuExtension::NONE }, std::move(oct) });
#endif // WITH_BOOST

    auto dd = std::make_unique<CpuGenerator<DoubleDouble, mnd::NONE, true>>();
    auto qd = std::make_unique<CpuGenerator<QuadDouble, mnd::NONE, true>>();
    cpuGenerators.insert({ { Precision::DOUBLE_DOUBLE, CpuExtension::NONE }, std::move(dd) });
    cpuGenerators.insert({ { Precision::QUAD_DOUBLE, CpuExtension::NONE }, std::move(qd) });

    auto td = std::make_unique<CpuGenerator<TripleDouble, mnd::NONE, true>>();
    cpuGenerators.insert({ { Precision::TRIPLE_DOUBLE, CpuExtension::NONE }, std::move(td) });

    auto fix512 = std::make_unique<CpuGenerator<Fixed512, mnd::NONE, true>>();
    cpuGenerators.insert({ { Precision::FIXED512, CpuExtension::NONE }, std::move(fix512) });

    devices = createDevices();

    adaptiveGenerator = createAdaptiveGenerator();
}


std::unique_ptr<mnd::AdaptiveGenerator> MandelContext::createAdaptiveGenerator(void)
{

    std::vector<Precision> types {
        Precision::FLOAT,
        Precision::DOUBLE_FLOAT,
        Precision::DOUBLE,
        Precision::DOUBLE_DOUBLE,
        Precision::TRIPLE_DOUBLE,
        Precision::QUAD_DOUBLE,
        Precision::FLOAT256,
        Precision::FLOAT512
    };

    auto ag = std::make_unique<AdaptiveGenerator>();

    for (auto type : types) {
        MandelGenerator* chosenGen = nullptr;
        auto generators = getCpuGenerators(type);
        CpuExtension ex = CpuExtension::NONE;
        for (auto* generator : generators) {
            if (generator->getExtension() >= ex) {
                ex = generator->getExtension();
                chosenGen = generator;
            }
        }
        for (auto& device : getDevices()) {
            auto* clGen = device->getGenerator(type);
            if (clGen != nullptr) {
                chosenGen = clGen;
            }
        }
        if (chosenGen != nullptr) {
            ag->addGenerator(mnd::getPrecision(type), *chosenGen);
        }
    }

    return ag;
}


std::vector<std::unique_ptr<MandelDevice>> MandelContext::createDevices(void)
{
    std::vector<std::unique_ptr<MandelDevice>> mandelDevices;
#ifdef WITH_OPENCL
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    //platforms.erase(platforms.begin() + 1);

    for (auto& platform : platforms) {
        std::string platformName = platform.getInfo<CL_PLATFORM_NAME>();
        std::string profile = platform.getInfo<CL_PLATFORM_PROFILE>();

        //printf("using opencl platform: %s\n", platformName.c_str());

        std::string ext = platform.getInfo<CL_PLATFORM_EXTENSIONS>();
        //printf("Platform extensions: %s\n", ext.c_str());
        //printf("Platform: %s, %s\n", platformName.c_str(), profile.c_str());

        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
        auto onError = [] (const char* errinfo, 
            const void* private_info,
            size_t cb, 
            void* user_data) {
                printf("opencl error: %s\n", errinfo);
        };
        
        cl::Context context{ devices, nullptr, onError };
        for (auto& device : devices) {
            //printf("Device: %s\n", device.getInfo<CL_DEVICE_NAME>().c_str());
            //printf("preferred float width: %d\n", device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT>());
            //printf("vendor: %s\n", device.getInfo<CL_DEVICE_VENDOR>().c_str());


            //printf("Device extensions: %s\n", ext.c_str());
            auto mandelDevice = std::make_unique<mnd::MandelDevice>(
                ClDeviceWrapper{ device, context }, platformName);
            MandelDevice& md = *mandelDevice;

            auto supportsDouble = md.supportsDouble();
            //printf("clock: %d", device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>());

            //printf("    using opencl device: %s\n", md.name.c_str());
            try {
                md.mandelGenerators.insert({ Precision::FLOAT, std::make_unique<ClGeneratorFloat>(md) });
                md.mandelGenerators.insert({ Precision::FIXED64, std::make_unique<ClGenerator64>(md) });
                //md.mandelGenerators.insert({ GeneratorType::FIXED128, std::make_unique<ClGenerator128>(md) });
            }
            catch (const std::string& err) {
                printf("err: %s", err.c_str());
            }
            try {
                md.mandelGenerators.insert({ Precision::DOUBLE_FLOAT, std::make_unique<ClGeneratorDoubleFloat>(md) });
            }
            catch (const std::string& err) {
                printf("err: %s", err.c_str());
            }

            if (supportsDouble) {
                try {
                    md.mandelGenerators.insert({ Precision::DOUBLE, std::make_unique<ClGeneratorDouble>(md) });
                    md.mandelGenerators.insert({ Precision::DOUBLE_DOUBLE, std::make_unique<ClGeneratorDoubleDouble>(md) });
                    md.mandelGenerators.insert({ Precision::TRIPLE_DOUBLE, std::make_unique<ClGeneratorTripleDouble>(md) });
                    md.mandelGenerators.insert({ Precision::QUAD_DOUBLE, std::make_unique<ClGeneratorQuadDouble>(md) });
                }
                catch (const std::string& err) {
                    printf("err: %s", err.c_str());
                    fflush(stdout);
                }
            }

            try {
                //md.generator128 = std::make_unique<ClGenerator128>(device);
            }
            catch (const std::string& /*err*/) {
                //fprintf(stderr, "error creating 128bit cl generator: %s\n", err.c_str());
            }

            mandelDevices.push_back(std::move(mandelDevice));
        }
    }
#endif // WITH_OPENCL
    
    return mandelDevices;
}


MandelContext::~MandelContext(void)
{
}


AdaptiveGenerator& MandelContext::getDefaultGenerator(void)
{
    return *adaptiveGenerator;
}


std::vector<std::unique_ptr<mnd::MandelDevice>>& MandelContext::getDevices(void)
{
    return devices;
}


asmjit::JitRuntime& MandelContext::getJitRuntime(void)
{
    return *jitRuntime;
}


MandelGenerator* MandelContext::getCpuGenerator(mnd::Precision type, mnd::CpuExtension ex)
{
    auto it = cpuGenerators.find({ type, ex });
    if (it != cpuGenerators.end())
        return it->second.get();
    else
        return nullptr;
}


std::vector<MandelContext::GeneratorType> MandelContext::getSupportedTypes(void) const
{
    std::vector<GeneratorType> types;
    for (auto& [type, gen] : cpuGenerators) {
        types.push_back(type);
    }
    return types;
}


std::vector<MandelGenerator*> MandelContext::getCpuGenerators(mnd::Precision prec) const
{
    std::vector<MandelGenerator*> generators;
    for (const auto& [type, gen] : cpuGenerators) {
        if (type.first == prec)
            generators.push_back(gen.get());
    }
    return generators;
}


