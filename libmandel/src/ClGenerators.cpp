#include "ClGenerators.h"

#include <iostream>
#include <iterator>


using namespace cl;

using mnd::ClGenerator;
using mnd::ClGeneratorFloat;
using mnd::ClGeneratorDouble;

Platform getPlatform() {
    /* Returns the first platform found. */
    std::vector<Platform> all_platforms;
    Platform::get(&all_platforms);

    if (all_platforms.size()==0) {
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
    float pixelScaleX = info.view.width / info.bWidth;
    float pixelScaleY = info.view.height / info.bHeight;

    Kernel iterate = Kernel(program, "iterate");
    iterate.setArg(0, buffer_A);
    iterate.setArg(1, int(info.bWidth));
    iterate.setArg(2, float(info.view.x));
    iterate.setArg(3, float(info.view.y));
    iterate.setArg(4, float(pixelScaleX));
    iterate.setArg(5, float(pixelScaleY));
    iterate.setArg(6, int(info.maxIter));

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
    /*Platform p = getPlatform();
    device = getDevice(p, 0, true);*/
    context = Context{ device };
    Program::Sources sources;

    std::string kcode = this->getKernelCode();

    sources.push_back({ kcode.c_str(), kcode.length() });

    program = Program{ context, sources };
    if (program.build({ device }) != CL_SUCCESS) {
        throw std::string(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
    }

    queue = CommandQueue(context, device);
}


std::string ClGeneratorFloat::getKernelCode(void) const
{
    if (false && device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT>() == 4) {
        return
            "__kernel void iterate(__global float* A, const int width, float xl, float yt, float pixelScaleX, float pixelScaleY, int max) {\n"
            "   int index = get_global_id(0) * 4;\n"
            "   int x = index % (width);\n"
            "   int y = index / (width);\n"
            "   float4 av = (float4)(x * pixelScaleX + xl, (x + 1) * pixelScaleX + xl, (x + 2) * pixelScaleX + xl, (x + 3) * pixelScaleX + xl);\n"
    //                        "(x + 4) * pixelScale + xl, (x + 5) * pixelScale + xl, (x + 6) * pixelScale + xl, (x + 7) * pixelScale + xl);\n"
            "   float4 bv = (float4)(y * pixelScaleY + yt);\n"
            "   float4 ca = av;\n"
            "   float4 cb = bv;\n"
            ""
            "   int4 counter = (int4) 1;"
            "   float4 threshold = (float4) 16;"
            "   int n = 0;\n"
            "   while (n < max) {\n"
            "       float4 aa = av * av;\n"
            "       float4 bb = bv * bv;\n"
            "       float4 ab = av * bv;\n"
            "       av = aa - bb + ca;\n"
            "       bv = 2 * ab + cb;\n"
            "       counter += -(threshold > (aa + bb));\n"
            "       if(all(threshold < (aa + bb))) break;\n"
            "       //if (aa + bb > 16) break;\n"
            "       n++;\n"
            "   }\n\n"
            "   A[index] = (float) counter[0];\n"
            "   A[index + 1] = (float) counter[1];\n"
            "   A[index + 2] = (float) counter[2];\n"
            "   A[index + 3] = (float) counter[3];\n"
    /*        "   A[index + 4] = (float) counter[4];\n"
            "   A[index + 5] = (float) counter[5];\n"
            "   A[index + 6] = (float) counter[6];\n"
            "   A[index + 7] = (float) counter[7];\n"*/
    //        "   A[get_global_id(0)] = 1;\n"
            "}\n";
    }
    else {

        return 
    //        "#pragma OPENCL EXTENSION cl_khr_fp64 : enable"
            "__kernel void iterate(__global float* A, const int width, float xl, float yt, float pixelScaleX, float pixelScaleY, int max) {"
            "   int index = get_global_id(0);\n"
            "   int x = index % width;"
            "   int y = index / width;"
            "   float a = x * pixelScaleX + xl;"
            "   float b = y * pixelScaleY + yt;"
            "   float ca = a;"
            "   float cb = b;"
            ""
            "   int n = 0;"
            "   while (n < max) {"
            "       float aa = a * a;"
            "       float bb = b * b;"
            "       float ab = a * b;"
            "       if (aa + bb > 16) break;"
            "       a = aa - bb + ca;"
            "       b = 2 * ab + cb;"
            "       n++;"
            "   }\n"
                // N + 1 - log (log  |Z(N)|) / log 2
            "   if (n >= max)\n"
            "       A[index] = max;\n"
            "   else"
            "       A[index] = ((float)n) + 1 - log(log(a * a + b * b) / 2) / log(2.0f);\n"
//            "   A[index] = ((float)n) + 1 - (a * a + b * b - 16) / (256 - 16);\n"
    //        "   A[get_global_id(0)] = 5;"
            "}";
    }
}


ClGeneratorDouble::ClGeneratorDouble(cl::Device device) :
    ClGenerator{ device }
{
    context = Context{ device };
    Program::Sources sources;

    std::string kcode = this->getKernelCode();

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
    float pixelScaleX = info.view.width / info.bWidth;
    float pixelScaleY = info.view.height / info.bHeight;

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


std::string ClGeneratorDouble::getKernelCode(void) const
{
    return 
        "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
        "__kernel void iterate(__global float* A, const int width, double xl, double yt, double pixelScaleX, double pixelScaleY, int max) {\n"
        "   int index = get_global_id(0);\n"
        "   int x = index % width;"
        "   int y = index / width;"
        "   double a = x * pixelScaleX + xl;"
        "   double b = y * pixelScaleY + yt;"
        "   double ca = a;"
        "   double cb = b;"
        ""
        "   int n = 0;"
        "   while (n < max) {"
        "       double aa = a * a;"
        "       double bb = b * b;"
        "       double ab = a * b;"
        "       if (aa + bb > 16) break;"
        "       a = aa - bb + ca;"
        "       b = 2 * ab + cb;"
        "       n++;"
        "   }\n"
        // N + 1 - log (log  |Z(N)|) / log 2
        "   if (n >= max)\n"
        "       A[index] = max;\n"
        "   else"
        "       A[index] = ((float)n) + 1 - log(log(a * a + b * b) / 2) / log(2.0f);\n"
        //            "   A[index] = ((float)n) + 1 - (a * a + b * b - 16) / (256 - 16);\n"
        //        "   A[get_global_id(0)] = 5;"
        "}";
}