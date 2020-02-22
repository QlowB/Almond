#include "ClGenerators.h"
#include "doubledouble.h"
#include "doublefloat.h"

#ifdef WITH_OPENCL

#include <iostream>
#include <iterator>
#include <utility>


using namespace cl;

using mnd::ClGenerator;
using mnd::ClGeneratorFloat;
using mnd::ClGeneratorDoubleFloat;
using mnd::ClGeneratorDouble;
using mnd::ClGeneratorDoubleDouble;
using mnd::ClGeneratorQuadDouble;
using mnd::ClGenerator128;

Platform getPlatform() {
    /* Returns the first platform found. */
    std::vector<Platform> all_platforms;
    Platform::get(&all_platforms);

    if (all_platforms.size() == 0) {
        std::cout << "No platforms found. Check OpenCL installation!\n";
        exit(1);
    }
    for (auto& p : all_platforms) {
        std::string name = p.getInfo<CL_PLATFORM_NAME>();
        std::string profile = p.getInfo<CL_PLATFORM_PROFILE>();
        printf("Platform: %s, %s\n", name.c_str(), profile.c_str());
    }
    return all_platforms[0];
}


Device getDevice(Platform& platform, int i, bool display = false) {
    /* Returns the deviced specified by the index i on platform.
    * If display is true, then all of the platforms are listed.
    */
    std::vector<Device> all_devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if (all_devices.size() == 0) {
        std::cout << "No devices found. Check OpenCL installation!\n";
        exit(1);
    }

    if (display) {
        for (::size_t j = 0; j < all_devices.size(); j++) {
            printf("Device %d: %s\n", int(j), all_devices[j].getInfo<CL_DEVICE_NAME>().c_str());
            printf("preferred float width: %d\n", all_devices[j].getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT>());
            printf("vendor: %s\n", all_devices[j].getInfo<CL_DEVICE_VENDOR>().c_str());
        }
    }
    return all_devices[i];
}


ClGenerator::ClGenerator(cl::Device device) :
    device{ device }
{
    /*Platform p = getPlatform();
    device = getDevice(p, 0, true);
    context = Context{ device };
    Program::Sources sources;

    std::string kcode = this->getKernelCode();

    sources.push_back({ kcode.c_str(), kcode.length() });

    program = Program{ context, sources };
    if (program.build({ device }) != CL_SUCCESS) {
        std::cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
        exit(1);
    }

    queue = CommandQueue(context, device);*/
}


ClGenerator::~ClGenerator(void)
{
    queue.flush();
    queue.finish();
}


void ClGenerator::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);
    float pixelScaleX = float(info.view.width / info.bWidth);
    float pixelScaleY = float(info.view.height / info.bHeight);

    Kernel iterate = Kernel(program, "iterate");
    iterate.setArg(0, buffer_A);
    iterate.setArg(1, int(info.bWidth));
    iterate.setArg(2, float(info.view.x));
    iterate.setArg(3, float(info.view.y));
    iterate.setArg(4, float(pixelScaleX));
    iterate.setArg(5, float(pixelScaleY));
    iterate.setArg(6, int(info.maxIter));
    iterate.setArg(7, int(info.smooth ? 1 : 0));

    // TODO check for overflow
    if (false && device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT>() == 4) {
        queue.enqueueNDRangeKernel(iterate, 0, NDRange(info.bWidth * info.bHeight / 4));
    } else {
        queue.enqueueNDRangeKernel(iterate, 0, NDRange(info.bWidth * info.bHeight));
    }
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);
}


ClGeneratorFloat::ClGeneratorFloat(cl::Device device) :
    ClGenerator{ device }
{
    context = Context{ device };
    Program::Sources sources;

    std::string kcode = this->getKernelCode(false);

    sources.push_back({ kcode.c_str(), kcode.length() });

    program = Program{ context, sources };
    if (program.build({ device }) != CL_SUCCESS) {
        throw std::string(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
    }

    queue = CommandQueue(context, device);
}


std::string ClGeneratorFloat::getKernelCode(bool smooth) const
{
    return 
//        "#pragma OPENCL EXTENSION cl_khr_fp64 : enable"
        "__kernel void iterate(__global float* A, const int width, float xl, float yt, float pixelScaleX, float pixelScaleY, int max, int smooth) {"
        "   int index = get_global_id(0);\n"
        "   int x = index % width;"
        "   int y = index / width;"
        "   float a = x * pixelScaleX + xl;"
        "   float b = y * pixelScaleY + yt;"
        "   float ca = a;"
        "   float cb = b;"
        ""
        "   int n = 0;"
        "   while (n < max - 1) {"
        "       float aa = a * a;"
        "       float bb = b * b;"
        "       float ab = a * b;"
        "       if (aa + bb > 16) break;"
        "       a = aa - bb + ca;"
        "       b = ab + ab + cb;"
        "       n++;"
        "   }\n"
        "   if (n >= max - 1)\n"
        "       A[index] = max;\n"
        "   else {"
        "       if (smooth != 0)\n"
        "           A[index] = ((float)n) + 1 - log(log(a * a + b * b) / 2) / log(2.0f);\n"
        "       else\n"
        "           A[index] = ((float)n);\n"
        "   }"
        "}";
}


ClGeneratorDoubleFloat::ClGeneratorDoubleFloat(cl::Device device) :
    ClGenerator{ device }
{
    context = Context{ device };
    Program::Sources sources;

    std::string kcode = this->getKernelCode(false);

    sources.push_back({ kcode.c_str(), kcode.length() });

    program = Program{ context, sources };
    if (program.build({ device }) != CL_SUCCESS) {
        throw std::string(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
    }

    queue = CommandQueue(context, device);
}


void ClGeneratorDoubleFloat::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    auto splitDouble = [] (double x) {
        float hi = float(x);
        float lo = float(x - double(hi));
        return std::pair{ hi, lo };
    };

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);
    double pixelScaleX = double(info.view.width / info.bWidth);
    double pixelScaleY = double(info.view.height / info.bHeight);

    auto[x1, x2] = splitDouble(double(info.view.x));
    auto[y1, y2] = splitDouble(double(info.view.y));
    auto[w1, w2] = splitDouble(pixelScaleX);
    auto[h1, h2] = splitDouble(pixelScaleY);


    Kernel iterate = Kernel(program, "iterate");
    iterate.setArg(0, buffer_A);
    iterate.setArg(1, int(info.bWidth));
    iterate.setArg(2, x1);
    iterate.setArg(3, x2);
    iterate.setArg(4, y1);
    iterate.setArg(5, y2);
    iterate.setArg(6, w1);
    iterate.setArg(7, w2);
    iterate.setArg(8, h1);
    iterate.setArg(9, h2);
    iterate.setArg(10, int(info.maxIter));
    iterate.setArg(11, int(info.smooth ? 1 : 0));

    cl_int result = queue.enqueueNDRangeKernel(iterate, 0, NDRange(info.bWidth * info.bHeight));
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);
}


std::string ClGeneratorDoubleFloat::getKernelCode(bool smooth) const
{
    return (char*) doublefloat_cl;
}


ClGeneratorDouble::ClGeneratorDouble(cl::Device device) :
    ClGenerator{ device }
{
    context = Context{ device };
    Program::Sources sources;

    std::string kcode = this->getKernelCode(false);

    sources.push_back({ kcode.c_str(), kcode.length() });

    program = Program{ context, sources };
    if (program.build({ device }) != CL_SUCCESS) {
        throw std::string(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
    }

    queue = CommandQueue(context, device);
}


void ClGeneratorDouble::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);
    double pixelScaleX = double(info.view.width / info.bWidth);
    double pixelScaleY = double(info.view.height / info.bHeight);

    Kernel iterate = Kernel(program, "iterate");
    iterate.setArg(0, buffer_A);
    iterate.setArg(1, int(info.bWidth));
    iterate.setArg(2, double(info.view.x));
    iterate.setArg(3, double(info.view.y));
    iterate.setArg(4, double(pixelScaleX));
    iterate.setArg(5, double(pixelScaleY));
    iterate.setArg(6, int(info.maxIter));
    iterate.setArg(7, int(info.smooth ? 1 : 0));

    cl_int result = queue.enqueueNDRangeKernel(iterate, 0, NDRange(info.bWidth * info.bHeight));
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);
}


std::string ClGeneratorDouble::getKernelCode(bool smooth) const
{
    return
        "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
        "__kernel void iterate(__global float* A, const int width, double xl, double yt, double pixelScaleX, double pixelScaleY, int max, int smooth) {\n"
        "   int index = get_global_id(0);\n"
        "   int x = index % width;"
        "   int y = index / width;"
        "   double a = x * pixelScaleX + xl;"
        "   double b = y * pixelScaleY + yt;"
        "   double ca = a;"
        "   double cb = b;"
        ""
        "   int n = 0;"
        "   while (n < max - 1) {"
        "       double aa = a * a;"
        "       double bb = b * b;"
        "       double ab = a * b;"
        "       if (aa + bb > 16) break;"
        "       a = aa - bb + ca;"
        "       b = ab + ab + cb;"
        "       n++;"
        "   }\n"
        // N + 1 - log (log  |Z(N)|) / log 2
        "   if (n >= max - 1)\n"
        "       A[index] = max;\n"
        "   else {"
        "       if (smooth != 0)\n"
        "           A[index] = ((float)n) + 1 - log(log((float)(a * a + b * b)) / 2) / log(2.0f);\n"
        "       else\n"
        "           A[index] = ((float)n);\n"
        "   }"
        "}";
}


ClGeneratorDoubleDouble::ClGeneratorDoubleDouble(cl::Device device) :
    ClGenerator{ device }
{
    context = Context{ device };
    Program::Sources sources;

    std::string kcode = this->getKernelCode(false);

    sources.push_back({ kcode.c_str(), kcode.length() });

    program = Program{ context, sources };
    if (program.build({ device }) != CL_SUCCESS) {
        throw std::string(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
    }

    queue = CommandQueue(context, device);
}


void ClGeneratorDoubleDouble::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);

    mnd::DoubleDouble x = mnd::convert<mnd::DoubleDouble>(info.view.x);
    mnd::DoubleDouble y = mnd::convert<mnd::DoubleDouble>(info.view.y);

    mnd::DoubleDouble psx = mnd::convert<mnd::DoubleDouble>(info.view.width / info.bWidth);
    mnd::DoubleDouble psy = mnd::convert<mnd::DoubleDouble>(info.view.height / info.bHeight);

    Kernel iterate = Kernel(program, "iterate");
    iterate.setArg(0, buffer_A);
    iterate.setArg(1, int(info.bWidth));
    iterate.setArg(2, x.x[0]);
    iterate.setArg(3, x.x[1]);
    iterate.setArg(4, y.x[0]);
    iterate.setArg(5, y.x[1]);
    iterate.setArg(6, psx.x[0]);
    iterate.setArg(7, psx.x[1]);
    iterate.setArg(8, psy.x[0]);
    iterate.setArg(9, psy.x[1]);
    iterate.setArg(10, int(info.maxIter));
    iterate.setArg(11, int(info.smooth ? 1 : 0));

    cl_int result = queue.enqueueNDRangeKernel(iterate, 0, NDRange(info.bWidth * info.bHeight));
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);
}


std::string ClGeneratorDoubleDouble::getKernelCode(bool smooth) const
{
    return (char*) doubledouble_cl;
}


ClGeneratorQuadDouble::ClGeneratorQuadDouble(cl::Device device) :
    ClGenerator{ device }
{
    context = Context{ device };
    Program::Sources sources;

    std::string kcode = this->getKernelCode(false);

    sources.push_back({ kcode.c_str(), kcode.length() });

    program = Program{ context, sources };
    if (program.build({ device }) != CL_SUCCESS) {
        throw std::string(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
    }

    queue = CommandQueue(context, device);
}


void ClGeneratorQuadDouble::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);

    mnd::DoubleDouble x = mnd::convert<mnd::DoubleDouble>(info.view.x);
    mnd::DoubleDouble y = mnd::convert<mnd::DoubleDouble>(info.view.y);

    mnd::DoubleDouble psx = mnd::convert<mnd::DoubleDouble>(info.view.width / info.bWidth);
    mnd::DoubleDouble psy = mnd::convert<mnd::DoubleDouble>(info.view.height / info.bHeight);

    Kernel iterate = Kernel(program, "iterate");
    iterate.setArg(0, buffer_A);
    iterate.setArg(1, int(info.bWidth));
    iterate.setArg(2, x.x[0]);
    iterate.setArg(3, x.x[1]);
    iterate.setArg(4, y.x[0]);
    iterate.setArg(5, y.x[1]);
    iterate.setArg(6, psx.x[0]);
    iterate.setArg(7, psx.x[1]);
    iterate.setArg(8, psy.x[0]);
    iterate.setArg(9, psy.x[1]);
    iterate.setArg(10, int(info.maxIter));
    iterate.setArg(11, int(info.smooth ? 1 : 0));

    cl_int result = queue.enqueueNDRangeKernel(iterate, 0, NDRange(info.bWidth * info.bHeight));
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);
}


std::string ClGeneratorQuadDouble::getKernelCode(bool smooth) const
{
    return (char*) doubledouble_cl;
}


ClGenerator128::ClGenerator128(cl::Device device) :
    ClGenerator{ device }
{
    context = Context{ device };
    Program::Sources sources;

    std::string kcode = this->getKernelCode(false);

    sources.push_back({ kcode.c_str(), kcode.length() });

    program = Program{ context, sources };
    if (program.build({ device }) != CL_SUCCESS) {
        throw std::string(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
    }

    queue = CommandQueue(context, device);
}


void ClGenerator128::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);
    float pixelScaleX = float(info.view.width / info.bWidth);
    float pixelScaleY = float(info.view.height / info.bHeight);

    Kernel iterate = Kernel(program, "iterate");
    iterate.setArg(0, buffer_A);
    iterate.setArg(1, int(info.bWidth));
    iterate.setArg(2, double(info.view.x));
    iterate.setArg(3, double(info.view.y));
    iterate.setArg(4, double(pixelScaleX));
    iterate.setArg(5, double(pixelScaleY));
    iterate.setArg(6, int(info.maxIter));

    queue.enqueueNDRangeKernel(iterate, 0, NDRange(info.bWidth * info.bHeight));
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);
}

#include <string>
#include <fstream>
#include <streambuf>

std::string ClGenerator128::getKernelCode(bool smooth) const
{
    //fprintf(stderr, "starting file read\n");
    std::ifstream t("mandel128.cl");
    std::string str((std::istreambuf_iterator<char>(t)),
        std::istreambuf_iterator<char>());
    //fprintf(stderr, "%s\n", str);
    return str;
}


#endif // WITH_OPENCL

