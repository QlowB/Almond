#include "Mandel.h"

#include "CpuGenerators.h"
#include "ClGenerators.h"

using mnd::MandelDevice;
using mnd::MandelContext;
using mnd::Generator;



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


MandelContext::MandelContext(void)
{
    if (cpuInfo.hasAvx()) {
        cpuGeneratorFloat = std::make_unique<CpuGeneratorAvxFloat>();
        cpuGeneratorDouble = std::make_unique<CpuGeneratorAvxDouble>();
    }
    else if (cpuInfo.hasSse2()) {
        cpuGeneratorFloat = std::make_unique<CpuGeneratorSse2Float>();
        cpuGeneratorDouble = std::make_unique<CpuGeneratorSse2Double>();
    }
    else {
        cpuGeneratorFloat = std::make_unique<CpuGeneratorFloat>();
        cpuGeneratorDouble = std::make_unique<CpuGeneratorDouble>();
    }

    cpuGenerator128 = std::make_unique<CpuGenerator128>();

    devices = createDevices();
}


std::vector<MandelDevice> MandelContext::createDevices(void)
{
    std::vector<MandelDevice> mandelDevices;

    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    platforms.erase(platforms.begin() + 1);

    for (auto& platform : platforms) {
        std::string name = platform.getInfo<CL_PLATFORM_NAME>();
        std::string profile = platform.getInfo<CL_PLATFORM_PROFILE>();

       std::string ext = platform.getInfo<CL_PLATFORM_EXTENSIONS>();
        printf("Platform extensions: %s\n", ext.c_str());
        printf("Platform: %s, %s\n", name.c_str(), profile.c_str());

        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
        for (auto& device : devices) {
            //printf("Device: %s\n", device.getInfo<CL_DEVICE_NAME>().c_str());
            //printf("preferred float width: %d\n", device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT>());
            //printf("vendor: %s\n", device.getInfo<CL_DEVICE_VENDOR>().c_str());

            std::string extensions = device.getInfo<CL_DEVICE_EXTENSIONS>();
            auto supportsDouble = extensions.find("cl_khr_fp64") != std::string::npos;

            printf("Device extensions: %s\n", ext.c_str());
            MandelDevice md;

            printf("clock: %d", device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>());

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
            mandelDevices.push_back(std::move(md));
        }
    }
    return mandelDevices;
}


const std::string& MandelDevice::getName(void) const
{
    return name;
}


Generator& MandelContext::getDefaultGenerator(void)
{
    return getCpuGeneratorDouble();
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
