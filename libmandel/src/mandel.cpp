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
    doubleGenerator{ nullptr }
{
}


mnd::Generator* MandelDevice::getGeneratorFloat(void) const
{
    if (floatGenerator)
        return floatGenerator.get();
    else
        return nullptr;
}


mnd::Generator* MandelDevice::getGeneratorDouble(void) const
{
    if (doubleGenerator)
        return doubleGenerator.get();
    else
        return nullptr;
}


mnd::Generator* MandelDevice::getGenerator128(void) const
{
    if (generator128)
        return generator128.get();
    else
        return nullptr;
}


MandelContext::MandelContext(void)
{

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86) 
    if (cpuInfo.hasAvx()) {
        cpuGeneratorFloat = std::make_unique<CpuGenerator<float, mnd::X86_AVX>>();
        cpuGeneratorDouble = std::make_unique<CpuGenerator<double, mnd::X86_AVX>>();
    }
    else if (cpuInfo.hasSse2()) {
        cpuGeneratorFloat = std::make_unique<CpuGenerator<float, mnd::X86_SSE2>>();
        cpuGeneratorDouble = std::make_unique<CpuGenerator<double, mnd::X86_SSE2>>();
    }
    else
#elif defined(__aarch64__)
    if (true) {
        cpuGeneratorFloat = std::make_unique<CpuGenerator<float, mnd::ARM_NEON>>();
        cpuGeneratorDouble = std::make_unique<CpuGenerator<double, mnd::ARM_NEON>>();
    }
    else
#endif
    {
        cpuGeneratorFloat = std::make_unique<CpuGenerator<float>>();
        cpuGeneratorDouble = std::make_unique<CpuGenerator<double>>();
    }

    cpuGenerator128 = std::make_unique<CpuGenerator<Fixed128>>();

    adaptiveGenerator = std::make_unique<AdaptiveGenerator>(cpuGeneratorFloat.get(), cpuGeneratorDouble.get());

    devices = createDevices();
}


std::vector<MandelDevice> MandelContext::createDevices(void)
{
    std::vector<MandelDevice> mandelDevices;
#ifdef WITH_OPENCL
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    platforms.erase(platforms.begin() + 1);

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
                md.floatGenerator = std::make_unique<ClGeneratorFloat>(device);
            }
            catch (const std::string& err) {
                printf("err: %s", err.c_str());
            }

            if (supportsDouble) {
                try {
                    md.doubleGenerator = std::make_unique<ClGeneratorDouble>(device);
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


Generator& MandelContext::getDefaultGenerator(void)
{
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


Generator& MandelContext::getCpuGenerator128(void)
{
    return *cpuGenerator128;
}
