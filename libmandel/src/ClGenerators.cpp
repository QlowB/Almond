#include "ClGenerators.h"
#include "Types.h"
#include "Mandel.h"
#include "OpenClInternal.h"
#include "OpenClCode.h"

#include <CL/cl2.hpp>

#if WITH_OPENCL

#include <iostream>
#include <iterator>
#include <utility>


using namespace cl;

using mnd::ClGenerator;
using mnd::ClGeneratorFloat;
using mnd::ClGeneratorDoubleFloat;
using mnd::ClGeneratorTripleFloat;
using mnd::ClGeneratorDouble;
using mnd::ClGeneratorDoubleDouble;
using mnd::ClGeneratorTripleDouble;
using mnd::ClGeneratorQuadDouble;
using mnd::ClGeneratorHexDouble;
using mnd::ClGeneratorOctaDouble;
using mnd::ClGenerator128;
using mnd::ClGenerator64;

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


ClGenerator::ClGenerator(mnd::MandelDevice& device, const std::string& source, mnd::Precision type) :
    MandelGenerator{ type },
    device{ device },
    context{ device.getClDevice().context }
{
    const cl::Device& dev = device.getClDevice().device;

    Program::Sources sources;
    sources.push_back({ source.c_str(), source.length() });

    program = Program{ context, sources };
    if (program.build({ dev }) != CL_SUCCESS) {
        printf("code -> %s\n", source.c_str());
        throw std::string(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev));
    }

#if CL_HPP_TARGET_OPENCL_VERSION >= 200
    /*cl_queue_properties qcp[] {
        CL_QUEUE_PROPERTIES, 0,
//        CL_QUEUE_PRIORITY_KHR, CL_QUEUE_PRIORITY_LOW_KHR,
        0
    };
    int err = 0;
    cl_command_queue dq = clCreateCommandQueueWithProperties(
        this->context.get(),
        dev.get(),
        qcp,
        &err
    );
    if (err == CL_SUCCESS) {
        printf("queue success\n");
        queue = CommandQueue(dq);
    }
    else {
        printf("queue non-success\n");
        queue = CommandQueue(context, dev);
    }
    */
    queue = CommandQueue(context, dev);
#else
    queue = CommandQueue(context, dev);
#endif

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


mnd::MandelDevice* ClGenerator::getDevice(void)
{
    return &device;
}


ClGeneratorFloat::ClGeneratorFloat(mnd::MandelDevice& device, const std::string& code) :
    ClGenerator{ device, code, mnd::Precision::FLOAT }
{
    const cl::Device& dev = device.getClDevice().device;
    useVec = dev.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT>() >= 4;
    // often still slower than non-vec variation
    useVec = false;
    kernel = Kernel(program, useVec ? "iterate_vec4" : "iterate");
}


void ClGeneratorFloat::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);
    float pixelScaleX = float(info.view.width / info.bWidth);
    float pixelScaleY = float(info.view.height / info.bHeight);

    kernel.setArg(0, buffer_A);
    kernel.setArg(1, int(info.bWidth));
    kernel.setArg(2, float(info.view.x));
    kernel.setArg(3, float(info.view.y));
    kernel.setArg(4, float(pixelScaleX));
    kernel.setArg(5, float(pixelScaleY));
    kernel.setArg(6, int(info.maxIter));
    kernel.setArg(7, int(info.smooth ? 1 : 0));
    kernel.setArg(8, int(info.julia ? 1 : 0));
    kernel.setArg(9, float(info.juliaX));
    kernel.setArg(10, float(info.juliaY));

    if (useVec) {
        queue.enqueueNDRangeKernel(kernel, 0, NDRange(info.bWidth * info.bHeight / 4));
    } else {
        queue.enqueueNDRangeKernel(kernel, 0, NDRange(info.bWidth * info.bHeight));
    }
    cl::Event event;
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data, nullptr, &event);
}


std::string ClGeneratorFloat::getKernelCode(bool smooth) const
{
    return mnd::getFloat_cl();
}


ClGeneratorDoubleFloat::ClGeneratorDoubleFloat(mnd::MandelDevice& device) :
    ClGenerator{ device, this->getKernelCode(false), mnd::Precision::DOUBLE_FLOAT  }
{
    kernel = Kernel(program, "iterate");
}


void ClGeneratorDoubleFloat::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);
    mnd::LightDoubleFloat pixelScX = double(info.view.width / info.bWidth);
    mnd::LightDoubleFloat pixelScY = double(info.view.height / info.bHeight);

    mnd::LightDoubleFloat x = double(info.view.x);
    mnd::LightDoubleFloat y = double(info.view.y);
    mnd::LightDoubleFloat jx = double(info.juliaX);
    mnd::LightDoubleFloat jy = double(info.juliaY);

    kernel.setArg(0, buffer_A);
    kernel.setArg(1, int(info.bWidth));
    kernel.setArg(2, x[0]);
    kernel.setArg(3, x[1]);
    kernel.setArg(4, y[0]);
    kernel.setArg(5, y[1]);
    kernel.setArg(6, pixelScX[0]);
    kernel.setArg(7, pixelScX[1]);
    kernel.setArg(8, pixelScY[0]);
    kernel.setArg(9, pixelScY[1]);
    kernel.setArg(10, int(info.maxIter));
    kernel.setArg(11, int(info.smooth ? 1 : 0));
    kernel.setArg(12, int(info.julia ? 1 : 0));
    kernel.setArg(13, jx[0]);
    kernel.setArg(14, jx[1]);
    kernel.setArg(15, jy[0]);
    kernel.setArg(16, jy[1]);

    cl_int result = queue.enqueueNDRangeKernel(kernel, 0, NDRange(info.bWidth * info.bHeight));
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);
}


std::string ClGeneratorDoubleFloat::getKernelCode(bool smooth) const
{
    return getDoubleFloat_cl();
}


ClGeneratorTripleFloat::ClGeneratorTripleFloat(mnd::MandelDevice& device) :
    ClGenerator{ device, this->getKernelCode(false), mnd::Precision::TRIPLE_FLOAT  }
{
    kernel = Kernel(program, "iterate");
}


void ClGeneratorTripleFloat::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);
    mnd::TripleFloat pixelScX = mnd::convert<mnd::TripleFloat>(info.view.width / info.bWidth);
    mnd::TripleFloat pixelScY = mnd::convert<mnd::TripleFloat>(info.view.height / info.bHeight);

    mnd::TripleFloat x = mnd::convert<mnd::TripleFloat>(info.view.x);
    mnd::TripleFloat y = mnd::convert<mnd::TripleFloat>(info.view.y);
    mnd::TripleFloat jx = mnd::convert<mnd::TripleFloat>(info.juliaX);
    mnd::TripleFloat jy = mnd::convert<mnd::TripleFloat>(info.juliaY);

    kernel.setArg(0, buffer_A);
    kernel.setArg(1, int(info.bWidth));
    kernel.setArg(2, x[0]);
    kernel.setArg(3, x[1]);
    kernel.setArg(4, x[2]);
    kernel.setArg(5, y[0]);
    kernel.setArg(6, y[1]);
    kernel.setArg(7, y[2]);
    kernel.setArg(8, pixelScX[0]);
    kernel.setArg(9, pixelScX[1]);
    kernel.setArg(10, pixelScX[2]);
    kernel.setArg(11, pixelScY[0]);
    kernel.setArg(12, pixelScY[1]);
    kernel.setArg(13, pixelScY[2]);
    kernel.setArg(14, int(info.maxIter));
    kernel.setArg(15, int(info.smooth ? 1 : 0));
    kernel.setArg(16, int(info.julia ? 1 : 0));
    kernel.setArg(17, jx[0]);
    kernel.setArg(18, jx[1]);
    kernel.setArg(19, jx[2]);
    kernel.setArg(20, jy[0]);
    kernel.setArg(21, jy[1]);
    kernel.setArg(22, jy[2]);

    cl_int result = queue.enqueueNDRangeKernel(kernel, 0, NDRange(info.bWidth * info.bHeight));
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);
}


std::string ClGeneratorTripleFloat::getKernelCode(bool smooth) const
{
    return getTripleFloat_cl();
}


ClGeneratorDouble::ClGeneratorDouble(mnd::MandelDevice& device, const std::string& source) :
    ClGenerator{ device, source, mnd::Precision::DOUBLE }
{
    kernel = Kernel(program, "iterate");
}


void ClGeneratorDouble::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);
    double pixelScaleX = double(info.view.width / info.bWidth);
    double pixelScaleY = double(info.view.height / info.bHeight);

    kernel.setArg(0, buffer_A);
    kernel.setArg(1, int(info.bWidth));
    kernel.setArg(2, double(info.view.x));
    kernel.setArg(3, double(info.view.y));
    kernel.setArg(4, double(pixelScaleX));
    kernel.setArg(5, double(pixelScaleY));
    kernel.setArg(6, int(info.maxIter));
    kernel.setArg(7, int(info.smooth ? 1 : 0));
    kernel.setArg(8, int(info.julia ? 1 : 0));
    kernel.setArg(9, double(info.juliaX));
    kernel.setArg(10, double(info.juliaY));

    cl_int result = queue.enqueueNDRangeKernel(kernel, 0, NDRange(info.bWidth * info.bHeight));
    cl::Event event;
    queue.enqueueReadBuffer(buffer_A, CL_FALSE, 0, bufferSize, data, nullptr, &event);
    queue.flush();
    event.wait();
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


ClGeneratorDoubleDouble::ClGeneratorDoubleDouble(mnd::MandelDevice& device) :
    ClGenerator{ device, getDoubleDouble_cl(), mnd::Precision::DOUBLE_DOUBLE }
{
    kernel = Kernel(program, "iterate");
}


void ClGeneratorDoubleDouble::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);

    mnd::DoubleDouble x = mnd::convert<mnd::DoubleDouble>(info.view.x);
    mnd::DoubleDouble y = mnd::convert<mnd::DoubleDouble>(info.view.y);

    mnd::DoubleDouble psx = mnd::convert<mnd::DoubleDouble>(info.view.width / info.bWidth);
    mnd::DoubleDouble psy = mnd::convert<mnd::DoubleDouble>(info.view.height / info.bHeight);

    mnd::DoubleDouble juliaX = mnd::convert<mnd::DoubleDouble>(info.juliaX);
    mnd::DoubleDouble juliaY = mnd::convert<mnd::DoubleDouble>(info.juliaY);

    kernel.setArg(0, buffer_A);
    kernel.setArg(1, int(info.bWidth));
    kernel.setArg(2, x.x[0]);
    kernel.setArg(3, x.x[1]);
    kernel.setArg(4, y.x[0]);
    kernel.setArg(5, y.x[1]);
    kernel.setArg(6, psx.x[0]);
    kernel.setArg(7, psx.x[1]);
    kernel.setArg(8, psy.x[0]);
    kernel.setArg(9, psy.x[1]);
    kernel.setArg(10, int(info.maxIter));
    kernel.setArg(11, int(info.smooth ? 1 : 0));
    kernel.setArg(12, info.julia ? 1 : 0);
    kernel.setArg(13, juliaX.x[0]);
    kernel.setArg(14, juliaX.x[1]);
    kernel.setArg(15, juliaY.x[0]);
    kernel.setArg(16, juliaY.x[1]);

    cl_int result = queue.enqueueNDRangeKernel(kernel, 0, NDRange(info.bWidth * info.bHeight));
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);
}


std::string ClGeneratorDoubleDouble::getKernelCode(bool smooth) const
{
    return getDoubleDouble_cl();
}


ClGeneratorTripleDouble::ClGeneratorTripleDouble(mnd::MandelDevice& device) :
    ClGenerator{ device, getTripleDouble_cl(), mnd::Precision::TRIPLE_DOUBLE }
{
    kernel = Kernel(program, "iterate");
}


void ClGeneratorTripleDouble::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);

    mnd::TripleDouble x = mnd::convert<mnd::TripleDouble>(info.view.x);
    mnd::TripleDouble y = mnd::convert<mnd::TripleDouble>(info.view.y);

    mnd::TripleDouble psx = mnd::convert<mnd::TripleDouble>(info.view.width / info.bWidth);
    mnd::TripleDouble psy = mnd::convert<mnd::TripleDouble>(info.view.height / info.bHeight);

    mnd::TripleDouble juliaX = mnd::convert<mnd::TripleDouble>(info.juliaX);
    mnd::TripleDouble juliaY = mnd::convert<mnd::TripleDouble>(info.juliaY);

    kernel.setArg(0, buffer_A);
    kernel.setArg(1, int(info.bWidth));
    kernel.setArg(2, x.x[0]);
    kernel.setArg(3, x.x[1]);
    kernel.setArg(4, x.x[2]);
    kernel.setArg(5, y.x[0]);
    kernel.setArg(6, y.x[1]);
    kernel.setArg(7, y.x[2]);
    kernel.setArg(8, psx.x[0]);
    kernel.setArg(9, psx.x[1]);
    kernel.setArg(10, psx.x[2]);
    kernel.setArg(11, psy.x[0]);
    kernel.setArg(12, psy.x[1]);
    kernel.setArg(13, psy.x[2]);
    kernel.setArg(14, int(info.maxIter));
    kernel.setArg(15, int(info.smooth ? 1 : 0));
    kernel.setArg(16, info.julia ? 1 : 0);
    kernel.setArg(17, juliaX.x[0]);
    kernel.setArg(18, juliaX.x[1]);
    kernel.setArg(19, juliaX.x[2]);
    kernel.setArg(20, juliaY.x[0]);
    kernel.setArg(21, juliaY.x[1]);
    kernel.setArg(22, juliaY.x[2]);

    cl_int result = queue.enqueueNDRangeKernel(kernel, 0, NDRange(info.bWidth * info.bHeight));
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);
}


std::string ClGeneratorTripleDouble::getKernelCode(bool smooth) const
{
    return getTripleDouble_cl();
}


ClGeneratorQuadDouble::ClGeneratorQuadDouble(mnd::MandelDevice& device) :
    ClGenerator{ device, getQuadDouble_cl(), mnd::Precision::QUAD_DOUBLE }
{
    kernel = Kernel(program, "iterate");
}


void ClGeneratorQuadDouble::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);

    mnd::QuadDouble x = mnd::convert<mnd::QuadDouble>(info.view.x);
    mnd::QuadDouble y = mnd::convert<mnd::QuadDouble>(info.view.y);

    mnd::QuadDouble psx = mnd::convert<mnd::QuadDouble>(info.view.width / info.bWidth);
    mnd::QuadDouble psy = mnd::convert<mnd::QuadDouble>(info.view.height / info.bHeight);

    mnd::QuadDouble jx = mnd::convert<mnd::QuadDouble>(info.juliaX);
    mnd::QuadDouble jy = mnd::convert<mnd::QuadDouble>(info.juliaY);

    kernel.setArg(0, buffer_A);
    kernel.setArg(1, int(info.bWidth));
    kernel.setArg(2, x.x[0]);
    kernel.setArg(3, x.x[1]);
    kernel.setArg(4, x.x[2]);
    kernel.setArg(5, x.x[3]);
    kernel.setArg(6, y.x[0]);
    kernel.setArg(7, y.x[1]);
    kernel.setArg(8, y.x[2]);
    kernel.setArg(9, y.x[3]);
    kernel.setArg(10, psx.x[0]);
    kernel.setArg(11, psx.x[1]);
    kernel.setArg(12, psx.x[2]);
    kernel.setArg(13, psx.x[3]);
    kernel.setArg(14, psy.x[0]);
    kernel.setArg(15, psy.x[1]);
    kernel.setArg(16, psy.x[2]);
    kernel.setArg(17, psy.x[3]);
    kernel.setArg(18, int(info.maxIter));
    kernel.setArg(19, int(info.smooth ? 1 : 0));
    kernel.setArg(20, int(info.julia ? 1 : 0));
    kernel.setArg(21, jx.x[0]);
    kernel.setArg(22, jx.x[1]);
    kernel.setArg(23, jx.x[2]);
    kernel.setArg(24, jx.x[3]);
    kernel.setArg(25, jy.x[0]);
    kernel.setArg(26, jy.x[1]);
    kernel.setArg(27, jy.x[2]);
    kernel.setArg(28, jy.x[3]);

    cl_int result = queue.enqueueNDRangeKernel(kernel, 0, NDRange(info.bWidth * info.bHeight));
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);

}


std::string ClGeneratorQuadDouble::getKernelCode(bool smooth) const
{
    return getQuadDouble_cl();
}


ClGeneratorHexDouble::ClGeneratorHexDouble(mnd::MandelDevice& device) :
    ClGenerator{ device, getHexDouble_cl(), mnd::Precision::HEX_DOUBLE }
{
    kernel = Kernel(program, "iterate");
}


void ClGeneratorHexDouble::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);

    mnd::HexDouble x = mnd::convert<mnd::HexDouble>(info.view.x);
    mnd::HexDouble y = mnd::convert<mnd::HexDouble>(info.view.y);

    mnd::HexDouble psx = mnd::convert<mnd::HexDouble>(info.view.width / info.bWidth);
    mnd::HexDouble psy = mnd::convert<mnd::HexDouble>(info.view.height / info.bHeight);

    mnd::HexDouble jx = mnd::convert<mnd::HexDouble>(info.juliaX);
    mnd::HexDouble jy = mnd::convert<mnd::HexDouble>(info.juliaY);

    const size_t argBufSize = 6 * sizeof(double);
    Buffer xbuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, argBufSize, x.x);
    Buffer ybuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, argBufSize, y.x);
    Buffer psxbuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, argBufSize, psx.x);
    Buffer psybuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, argBufSize, psy.x);
    Buffer jxbuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, argBufSize, jx.x);
    Buffer jybuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, argBufSize);

    kernel.setArg(0, buffer_A);
    kernel.setArg(1, int(info.bWidth));
    kernel.setArg(2, xbuf);
    kernel.setArg(3, ybuf);
    kernel.setArg(4, psxbuf);
    kernel.setArg(5, psybuf);
    kernel.setArg(6, int(info.maxIter));
    kernel.setArg(7, int(info.smooth ? 1 : 0));
    kernel.setArg(8, int(info.julia ? 1 : 0));
    kernel.setArg(9, jxbuf);
    kernel.setArg(10, jybuf);

    cl_int result = queue.enqueueNDRangeKernel(kernel, 0, NDRange(info.bWidth * info.bHeight));
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);

}


std::string ClGeneratorHexDouble::getKernelCode(bool smooth) const
{
    return getHexDouble_cl();
}


ClGeneratorOctaDouble::ClGeneratorOctaDouble(mnd::MandelDevice& device) :
    ClGenerator{ device, getOctaDouble_cl(), mnd::Precision::OCTA_DOUBLE }
{
    kernel = Kernel(program, "iterate");
}


void ClGeneratorOctaDouble::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);

    mnd::OctaDouble x = mnd::convert<mnd::OctaDouble>(info.view.x);
    mnd::OctaDouble y = mnd::convert<mnd::OctaDouble>(info.view.y);

    mnd::OctaDouble psx = mnd::convert<mnd::OctaDouble>(info.view.width / info.bWidth);
    mnd::OctaDouble psy = mnd::convert<mnd::OctaDouble>(info.view.height / info.bHeight);

    mnd::OctaDouble jx = mnd::convert<mnd::OctaDouble>(info.juliaX);
    mnd::OctaDouble jy = mnd::convert<mnd::OctaDouble>(info.juliaY);

    const size_t argBufSize = 8 * sizeof(double);
    Buffer xbuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, argBufSize, x.x);
    Buffer ybuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, argBufSize, y.x);
    Buffer psxbuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, argBufSize, psx.x);
    Buffer psybuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, argBufSize, psy.x);
    Buffer jxbuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, argBufSize, jx.x);
    Buffer jybuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, argBufSize);

    kernel.setArg(0, buffer_A);
    kernel.setArg(1, int(info.bWidth));
    kernel.setArg(2, xbuf);
    kernel.setArg(3, ybuf);
    kernel.setArg(4, psxbuf);
    kernel.setArg(5, psybuf);
    kernel.setArg(6, int(info.maxIter));
    kernel.setArg(7, int(info.smooth ? 1 : 0));
    kernel.setArg(8, int(info.julia ? 1 : 0));
    kernel.setArg(9, jxbuf);
    kernel.setArg(10, jybuf);

    cl_int result = queue.enqueueNDRangeKernel(kernel, 0, NDRange(info.bWidth * info.bHeight));
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);

}


std::string ClGeneratorOctaDouble::getKernelCode(bool smooth) const
{
    return getOctaDouble_cl();
}

ClGenerator128::ClGenerator128(mnd::MandelDevice& device) :
    ClGenerator{ device, getFixed512_cl(), mnd::Precision::FIXED128 }
{
    kernel = Kernel(program, "iterate");
}


void ClGenerator128::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);
    float pixelScaleX = float(info.view.width / info.bWidth);
    float pixelScaleY = float(info.view.height / info.bHeight);

    using ull = unsigned long long;
    ull x1 = ull(double(info.view.x) * 0x10000ULL);
    ull x2 = 0;
    ull y1 = ull(double(info.view.y) * 0x10000ULL);
    ull y2 = 0;
    ull w1 = ull(double(pixelScaleX) * 0x10000ULL);
    ull w2 = 0;
    ull h1 = ull(double(pixelScaleY) * 0x10000ULL);
    ull h2 = 0;
    ull jx1 = ull(double(info.juliaX) * 0x10000ULL);
    ull jx2 = 0;
    ull jy1 = ull(double(info.juliaY) * 0x10000ULL);
    ull jy2 = 0;

    kernel.setArg(0, buffer_A);
    kernel.setArg(1, int(info.bWidth));
    kernel.setArg(2, ull(x1));
    kernel.setArg(3, ull(x2));
    kernel.setArg(4, ull(y1));
    kernel.setArg(5, ull(y2));
    kernel.setArg(6, ull(w1));
    kernel.setArg(7, ull(w2));
    kernel.setArg(8, ull(h1));
    kernel.setArg(9, ull(h2));
    kernel.setArg(10, int(info.maxIter));
    kernel.setArg(11, int(info.smooth ? 1 : 0));
    kernel.setArg(12, int(info.julia ? 1 : 0));
    kernel.setArg(13, ull(jx1));
    kernel.setArg(14, ull(jx2));
    kernel.setArg(15, ull(jy1));
    kernel.setArg(16, ull(jy2));

    queue.enqueueNDRangeKernel(kernel, 0, NDRange(info.bWidth * info.bHeight));
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);
}


std::string ClGenerator128::getKernelCode(bool smooth) const
{
    /*//fprintf(stderr, "starting file read\n");
    std::ifstream t("mandel128.cl");
    std::string str((std::istreambuf_iterator<char>(t)),
        std::istreambuf_iterator<char>());
    //fprintf(stderr, "%s\n", str);
    return str;*/
    return getFixed128_cl();
}


ClGenerator64::ClGenerator64(mnd::MandelDevice& device) :
    ClGenerator{ device, getFixed64_cl(), mnd::Precision::FIXED64 }
{
    kernel = Kernel(program, "iterate");
}


#include "CpuGenerators.h"
void ClGenerator64::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);
    float pixelScaleX = float(info.view.width / info.bWidth);
    float pixelScaleY = float(info.view.height / info.bHeight);

    using ull = uint64_t;
    ull x = ull(::round(double(info.view.x) * (1LL << 48)));
    ull y = ull(::round(double(info.view.y) * (1LL << 48)));
    ull w = ull(::round(double(pixelScaleX) * (1LL << 48)));
    ull h = ull(::round(double(pixelScaleY) * (1LL << 48)));
    ull jx = ull(::round(double(info.juliaX) * (1LL << 48)));
    ull jy = ull(::round(double(info.juliaY) * (1LL << 48)));
    //x = 0;
    //y = 0;
    
    kernel.setArg(0, buffer_A);
    kernel.setArg(1, int(info.bWidth));
    kernel.setArg(2, ull(x));
    kernel.setArg(3, ull(y));
    kernel.setArg(4, ull(w));
    kernel.setArg(5, ull(h));
    kernel.setArg(6, int(info.maxIter));
    kernel.setArg(7, int(info.smooth ? 1 : 0));
    kernel.setArg(8, int(info.julia ? 1 : 0));
    kernel.setArg(9, ull(jx));
    kernel.setArg(10, ull(jy));

    queue.enqueueNDRangeKernel(kernel, 0, NDRange(info.bWidth * info.bHeight));
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);
    //CpuGenerator<Fixed64> fx;
    //fx.generate(info, data);
}


std::string ClGenerator64::getKernelCode(bool smooth) const
{
    /*//fprintf(stderr, "starting file read\n");
    std::ifstream t("mandel128.cl");
    std::string str((std::istreambuf_iterator<char>(t)),
    std::istreambuf_iterator<char>());
    //fprintf(stderr, "%s\n", str);
    return str;*/
    return getFixed64_cl();
}

#endif // WITH_OPENCL

