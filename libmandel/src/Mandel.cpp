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


MandelDevice::MandelDevice(void) :
    floatGenerator{ nullptr },
    doubleGenerator{ nullptr },
    floatGeneratorSmooth{ nullptr },
    doubleGeneratorSmooth{ nullptr }

{
}


mnd::Generator* MandelDevice::getGeneratorFloat(bool smooth) const
{
    if (smooth)
        return floatGeneratorSmooth.get();
    else
        return floatGenerator.get();
}


mnd::Generator* MandelDevice::getGeneratorDouble(bool smooth) const
{
    if (smooth)
        return doubleGeneratorSmooth.get();
    else
        return doubleGenerator.get();
}


/*
mnd::Generator* MandelDevice::getGeneratorQuad(bool smooth) const
{
    if (smooth)
        return quadGeneratorSmooth.get();
    else
        return quadGenerator.get();
}*/


/*
mnd::Generator* MandelDevice::getGenerator128(bool smooth) const
{
    if (smooth)
        return generator128Smooth.get();
    else
        return generator128.get();
}
*/

MandelContext::MandelContext(void) :
    cpuGeneratorQuad{ nullptr },
    cpuGeneratorQuadSmooth{ nullptr }
{

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86) 
    if (cpuInfo.hasAvx()) {
        cpuGeneratorFloat = std::make_unique<CpuGenerator<float, mnd::X86_AVX, true, false>>();
        cpuGeneratorDouble = std::make_unique<CpuGenerator<double, mnd::X86_AVX, true, false>>();
        cpuGeneratorFloatSmooth = std::make_unique<CpuGenerator<float, mnd::X86_AVX, true, true>>();
        cpuGeneratorDoubleSmooth = std::make_unique<CpuGenerator<double, mnd::X86_AVX, true, true>>();
    }
    else if (cpuInfo.hasSse2()) {
        cpuGeneratorFloat = std::make_unique<CpuGenerator<float, mnd::X86_SSE2, true, false>>();
        cpuGeneratorDouble = std::make_unique<CpuGenerator<double, mnd::X86_SSE2, true, false>>();
        cpuGeneratorFloatSmooth = std::make_unique<CpuGenerator<float, mnd::X86_SSE2, true, true>>();
        cpuGeneratorDoubleSmooth = std::make_unique<CpuGenerator<double, mnd::X86_SSE2, true, true>>();
    }
    else
#elif defined(__aarch64__)
    if (true) {
        cpuGeneratorFloat = std::make_unique<CpuGenerator<float, mnd::ARM_NEON, true, false>>();
        cpuGeneratorDouble = std::make_unique<CpuGenerator<double, mnd::ARM_NEON, true, false>>();
        cpuGeneratorFloatSmooth = std::make_unique<CpuGenerator<float, mnd::ARM_NEON>>();
        cpuGeneratorDoubleSmooth = std::make_unique<CpuGenerator<double, mnd::ARM_NEON>>();
    }
    else
#endif
    {
        cpuGeneratorFloat = std::make_unique<CpuGenerator<float, mnd::NONE, true, false>>();
        cpuGeneratorDouble = std::make_unique<CpuGenerator<double, mnd::NONE, true, false>>();
        cpuGeneratorFloatSmooth = std::make_unique<CpuGenerator<float, mnd::NONE, true, true>>();
        cpuGeneratorDoubleSmooth = std::make_unique<CpuGenerator<double, mnd::NONE, true, true>>();
    }

    //cpuGenerator128 = std::make_unique<CpuGenerator<Fixed128, mnd::NONE, true, false>>();
    //cpuGenerator128Smooth = std::make_unique<CpuGenerator<Fixed128>>();
    //cpuGeneratorFixedp = std::make_unique<CpuGenerator<fixed<1, 3>>>();

#ifdef WITH_BOOST
    cpuGeneratorQuad = std::make_unique<CpuGenerator<Float128, mnd::NONE, true, false>>();
    cpuGeneratorQuadSmooth = std::make_unique<CpuGenerator<Float128, mnd::NONE, true, true>>();
    cpuGeneratorOct = std::make_unique<CpuGenerator<Float256, mnd::NONE, true, false>>();
#endif // WITH_BOOST

    devices = createDevices();
    if (devices.empty()) {
#ifdef WITH_BOOST
        adaptiveGenerator = std::make_unique<AdaptiveGenerator>(
            cpuGeneratorFloat.get(), cpuGeneratorDouble.get(), cpuGeneratorQuad.get());
        adaptiveGeneratorSmooth = std::make_unique<AdaptiveGenerator>(
            cpuGeneratorFloatSmooth.get(), cpuGeneratorDoubleSmooth.get(),
            cpuGeneratorQuadSmooth.get());
#else
        adaptiveGenerator = std::make_unique<AdaptiveGenerator>(
            cpuGeneratorFloat.get(), cpuGeneratorDouble.get());
        adaptiveGeneratorSmooth = std::make_unique<AdaptiveGenerator>(
            cpuGeneratorFloatSmooth.get(), cpuGeneratorDoubleSmooth.get());
#endif
    }
    else {
        auto& device1 = devices[0];
        Generator* floatGenerator = device1.getGeneratorFloat(false);
        Generator* doubleGenerator = device1.getGeneratorDouble(false);
        Generator* floatGeneratorSmooth = device1.getGeneratorFloat(true);
        Generator* doubleGeneratorSmooth = device1.getGeneratorDouble(true);
        if (floatGenerator == nullptr)
            floatGenerator = cpuGeneratorFloat.get();
        if (doubleGenerator == nullptr)
            doubleGenerator = cpuGeneratorDouble.get();
        if (floatGeneratorSmooth == nullptr)
            floatGeneratorSmooth = cpuGeneratorFloatSmooth.get();
        if (doubleGeneratorSmooth == nullptr)
            doubleGeneratorSmooth = cpuGeneratorDoubleSmooth.get();
#ifdef WITH_BOOST
        adaptiveGeneratorSmooth = std::make_unique<AdaptiveGenerator>(floatGeneratorSmooth, doubleGeneratorSmooth, cpuGeneratorQuadSmooth.get());
        adaptiveGenerator = std::make_unique<AdaptiveGenerator>(floatGenerator, doubleGenerator, cpuGeneratorQuad.get());
#else
        adaptiveGeneratorSmooth = std::make_unique<AdaptiveGenerator>(floatGeneratorSmooth, doubleGeneratorSmooth);
        adaptiveGenerator = std::make_unique<AdaptiveGenerator>(floatGenerator, doubleGenerator);
#endif
    }
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
                md.floatGenerator = std::make_unique<ClGeneratorFloat>(device, false);
                md.floatGeneratorSmooth = std::make_unique<ClGeneratorFloat>(device, true);
            }
            catch (const std::string& err) {
                printf("err: %s", err.c_str());
            }

            if (supportsDouble) {
                try {
                    md.doubleGenerator = std::make_unique<ClGeneratorDouble>(device, false);
                    md.doubleGeneratorSmooth = std::make_unique<ClGeneratorDouble>(device, true);
                }
                catch (const std::string& err) {
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


const std::string& MandelDevice::getName(void) const
{
    return name;
}


Generator& MandelContext::getDefaultGenerator(bool smooth)
{
    if (smooth)
        return *adaptiveGeneratorSmooth;
    else
        return *adaptiveGenerator;
}


const std::vector<MandelDevice>& MandelContext::getDevices(void)
{
    return devices;
}


Generator& MandelContext::getCpuGeneratorFloat(void)
{
    return *cpuGeneratorFloat;
}


Generator& MandelContext::getCpuGeneratorDouble(void)
{
    return *cpuGeneratorDouble;
}


Generator* MandelContext::getCpuGeneratorQuad(void)
{
    return cpuGeneratorQuad.get();
}


Generator* MandelContext::getCpuGeneratorOct(void)
{
    return cpuGeneratorOct.get();
}
