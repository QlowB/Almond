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
#ifdef WITH_OPENCL
    extensions = clDevice->device.getInfo<CL_DEVICE_EXTENSIONS>();
    name = clDevice->device.getInfo<CL_DEVICE_NAME>();
    vendor = clDevice->device.getInfo<CL_DEVICE_VENDOR>();
#endif // WITH_OPENCL
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
        cpuGenerators.insert({ std::pair{ Precision::FLOAT, CpuExtension::X86_AVX_512 }, std::move(fl) });
        //cpuGenerators.insert({ { Precision::DOUBLE, CpuExtension::X86_AVX_512 }, std::move(db) });
    }
#   endif
    if (cpuInfo.hasAvx()) {
        auto fl = std::make_unique<CpuGenerator<float, mnd::X86_AVX, true>>();
        auto db = std::make_unique<CpuGenerator<double, mnd::X86_AVX, true>>();
        auto ddb = std::make_unique<CpuGenerator<DoubleDouble, mnd::X86_AVX, true>>();
        auto tdb = std::make_unique<CpuGenerator<TripleDouble, mnd::X86_AVX, true>>();
        cpuGenerators.insert({ std::pair{ Precision::FLOAT, CpuExtension::X86_AVX }, std::move(fl) });
        cpuGenerators.insert({ std::pair{ Precision::DOUBLE, CpuExtension::X86_AVX }, std::move(db) });
        cpuGenerators.insert({ std::pair{ Precision::DOUBLE_DOUBLE, CpuExtension::X86_AVX }, std::move(ddb) });
        cpuGenerators.insert({ std::pair{ Precision::TRIPLE_DOUBLE, CpuExtension::X86_AVX }, std::move(tdb) });
    }
    if (cpuInfo.hasAvx2() && cpuInfo.hasFma()) {
        auto favxfma = std::make_unique<CpuGenerator<float, mnd::X86_AVX_FMA, true>>();
        auto davxfma = std::make_unique<CpuGenerator<double, mnd::X86_AVX_FMA, true>>();
        auto ddavxfma = std::make_unique<CpuGenerator<DoubleDouble, mnd::X86_AVX_FMA, true>>();
        auto qdavxfma = std::make_unique<CpuGenerator<QuadDouble, mnd::X86_AVX_FMA, true>>();
        cpuGenerators.insert({ std::pair{ Precision::FLOAT, CpuExtension::X86_AVX_FMA }, std::move(favxfma) });
        cpuGenerators.insert({ std::pair{ Precision::DOUBLE, CpuExtension::X86_AVX_FMA }, std::move(davxfma) });
        cpuGenerators.insert({ std::pair{ Precision::DOUBLE_DOUBLE, CpuExtension::X86_AVX_FMA }, std::move(ddavxfma) });
        cpuGenerators.insert({ std::pair{ Precision::QUAD_DOUBLE, CpuExtension::X86_AVX_FMA }, std::move(qdavxfma) });
    }
    if (cpuInfo.hasSse2()) {
        auto fl = std::make_unique<CpuGenerator<float, mnd::X86_SSE2, true>>();
        auto db = std::make_unique<CpuGenerator<double, mnd::X86_SSE2, true>>();
        cpuGenerators.insert({ std::pair{ Precision::FLOAT, CpuExtension::X86_SSE2 }, std::move(fl) });
        cpuGenerators.insert({ std::pair{ Precision::DOUBLE, CpuExtension::X86_SSE2 }, std::move(db) });
    }
#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) 
    if (cpuInfo.hasNeon()) {
        auto fl = std::make_unique<CpuGenerator<float, mnd::ARM_NEON, true>>();
        auto db = std::make_unique<CpuGenerator<double, mnd::ARM_NEON, true>>();
        auto ddb = std::make_unique<CpuGenerator<mnd::DoubleDouble, mnd::ARM_NEON, true>>();
        cpuGenerators.insert({ std::pair{ Precision::FLOAT, CpuExtension::ARM_NEON }, std::move(fl) });
        cpuGenerators.insert({ std::pair{ Precision::DOUBLE, CpuExtension::ARM_NEON }, std::move(db) });
        cpuGenerators.insert({ std::pair{ Precision::DOUBLE_DOUBLE, CpuExtension::ARM_NEON }, std::move(ddb) });
    }
#endif
    {
        auto fl = std::make_unique<CpuGenerator<float, mnd::NONE, true>>();
        auto db = std::make_unique<CpuGenerator<double, mnd::NONE, true>>();
        cpuGenerators.insert({ std::pair{ Precision::FLOAT, CpuExtension::NONE }, std::move(fl) });
        cpuGenerators.insert({ std::pair{ Precision::DOUBLE, CpuExtension::NONE }, std::move(db) });

        auto fx64 = std::make_unique<CpuGenerator<Fixed64, mnd::NONE, true>>();
        auto fx128 = std::make_unique<CpuGenerator<Fixed128, mnd::NONE, true>>();
        cpuGenerators.insert({ std::pair{ Precision::FIXED64, CpuExtension::NONE }, std::move(fx64) });
        cpuGenerators.insert({ std::pair{ Precision::FIXED128, CpuExtension::NONE }, std::move(fx128) });
    }

#ifdef WITH_BOOST
    auto quad = std::make_unique<CpuGenerator<Float128, mnd::NONE, true>>();
    auto oct = std::make_unique<CpuGenerator<Float256, mnd::NONE, true>>();
    auto f512 = std::make_unique<CpuGenerator<Float512, mnd::NONE, true>>();
    cpuGenerators.insert({ std::pair{ Precision::FLOAT128, CpuExtension::NONE }, std::move(quad) });
    cpuGenerators.insert({ std::pair{ Precision::FLOAT256, CpuExtension::NONE }, std::move(oct) });
    cpuGenerators.insert({ std::pair{ Precision::FLOAT512, CpuExtension::NONE }, std::move(f512) });
#endif // WITH_BOOST

    auto dd = std::make_unique<CpuGenerator<DoubleDouble, mnd::NONE, true>>();
    auto td = std::make_unique<CpuGenerator<TripleDouble, mnd::NONE, true>>();
    auto qd = std::make_unique<CpuGenerator<QuadDouble, mnd::NONE, true>>();
    auto hd = std::make_unique<CpuGenerator<HexDouble, mnd::NONE, true>>();
    auto od = std::make_unique<CpuGenerator<OctaDouble, mnd::NONE, true>>();
    cpuGenerators.insert({ std::pair{ Precision::DOUBLE_DOUBLE, CpuExtension::NONE }, std::move(dd) });
    cpuGenerators.insert({ std::pair{ Precision::TRIPLE_DOUBLE, CpuExtension::NONE }, std::move(td) });
    cpuGenerators.insert({ std::pair{ Precision::QUAD_DOUBLE, CpuExtension::NONE }, std::move(qd) });
    cpuGenerators.insert({ std::pair{ Precision::HEX_DOUBLE, CpuExtension::NONE }, std::move(hd) });
    cpuGenerators.insert({ std::pair{ Precision::OCTA_DOUBLE, CpuExtension::NONE }, std::move(od) });


    auto fix512 = std::make_unique<CpuGenerator<Fixed512, mnd::NONE, true>>();
    cpuGenerators.insert({ std::pair{ Precision::FIXED512, CpuExtension::NONE }, std::move(fix512) });

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
        Precision::HEX_DOUBLE,
        Precision::OCTA_DOUBLE,
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
            auto mandelDevice = std::make_unique<mnd::MandelDevice>(
                ClDeviceWrapper{ device, context }, platformName);
            MandelDevice& md = *mandelDevice;

            auto supportsDouble = md.supportsDouble();

//#pragma omp parallel
#pragma omp sections
            {
#pragma omp section
                try {
                    md.mandelGenerators.insert({ Precision::FLOAT, std::make_unique<ClGeneratorFloat>(md) });
                    //md.mandelGenerators.insert({ Precision::FIXED64, std::make_unique<ClGenerator64>(md) });
                    //md.mandelGenerators.insert({ GeneratorType::FIXED128, std::make_unique<ClGenerator128>(md) });
                }
                catch (const std::string& err) {
                    printf("err: %s", err.c_str());
                }

#pragma omp section
                try {
                    md.mandelGenerators.insert({ Precision::DOUBLE_FLOAT, std::make_unique<ClGeneratorDoubleFloat>(md) });
                }
                catch (const std::string& err) {
                    printf("err: %s", err.c_str());
                }

#pragma omp section
                try {
                    md.mandelGenerators.insert({ Precision::TRIPLE_FLOAT, std::make_unique<ClGeneratorTripleFloat>(md) });
                }
                catch (const std::string& err) {
                    printf("err: %s", err.c_str());
                }

#pragma omp section
                if (supportsDouble) {
                    try {
                        md.mandelGenerators.insert({ Precision::DOUBLE, std::make_unique<ClGeneratorDouble>(md) });
                        md.mandelGenerators.insert({ Precision::DOUBLE_DOUBLE, std::make_unique<ClGeneratorDoubleDouble>(md) });
                    }
                    catch (const std::string& err) {
                        printf("err: %s", err.c_str());
                        fflush(stdout);
                    }
                }

#pragma omp section
                if (supportsDouble) {
                    try {
                        md.mandelGenerators.insert({ Precision::TRIPLE_DOUBLE, std::make_unique<ClGeneratorTripleDouble>(md) });
                    }
                    catch (const std::string& err) {
                        printf("err: %s", err.c_str());
                        fflush(stdout);
                    }
                }

#pragma omp section
                if (supportsDouble) {
                    try {
                        md.mandelGenerators.insert({ Precision::QUAD_DOUBLE, std::make_unique<ClGeneratorQuadDouble>(md) });
                    }
                    catch (const std::string& err) {
                        printf("err: %s", err.c_str());
                        fflush(stdout);
                    }
                }

#pragma omp section
                if (supportsDouble) {
                    try {
                        md.mandelGenerators.insert({ Precision::HEX_DOUBLE, std::make_unique<ClGeneratorHexDouble>(md) });
                    }
                    catch (const std::string& err) {
                        printf("err: %s", err.c_str());
                        fflush(stdout);
                    }
                }

#pragma omp section
                if (supportsDouble) {
                    try {
                        md.mandelGenerators.insert({ Precision::OCTA_DOUBLE, std::make_unique<ClGeneratorOctaDouble>(md) });
                    }
                    catch (const std::string& err) {
                        printf("err: %s", err.c_str());
                        fflush(stdout);
                    }
                }
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


