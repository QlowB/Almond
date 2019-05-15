#include "Generators.h"

#include "GenericMandelbrot.h"
#include "Fixed.h"

#include <iostream>
#include <iterator>


using namespace cl;

Platform getPlatform() {
    /* Returns the first platform found. */
    std::vector<Platform> all_platforms;
    Platform::get(&all_platforms);

    if (all_platforms.size()==0) {
        std::cout << "No platforms found. Check OpenCL installation!\n";
        exit(1);
    }
    return all_platforms[0];
}


Device getDevice(Platform platform, int i, bool display = false) {
    /* Returns the deviced specified by the index i on platform.
    * If display is true, then all of the platforms are listed.
    */
    std::vector<Device> all_devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &all_devices);
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


ClGenerator::ClGenerator(void)
{
    Platform p = getPlatform();
    device = getDevice(p, 0, true);
    context = Context{ device };
    Program::Sources sources;

    std::string kcode;
 

    if (device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT>() == 4) {
        kcode =
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

        kcode =
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
            "   A[index] = ((float)n) + 1 - (a * a + b * b - 16) / (256 - 16);\n"
    //        "   A[get_global_id(0)] = 5;"
            "}";
    }


    sources.push_back({ kcode.c_str(), kcode.length() });

    program = Program{ context, sources };
    if (program.build({ device }) != CL_SUCCESS) {
        std::cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
        exit(1);
    }

    queue = CommandQueue(context, device);
}

/*Bitmap<RGBColor> ClGenerator::generate(const MandelInfo& info)
{
    return enqueueMandelbrot(info.bWidth, info.bHeight, info.view.x, info.view.y, info.view.width).get();
}*/

Bitmap<float> ClGenerator::generateRaw(const MandelInfo& info)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);
    
    Bitmap<float> bitmap{ info.bWidth, info.bHeight };
    Buffer buffer_A(context, CL_MEM_READ_WRITE, bufferSize);
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

    if (device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT>() == 4) {
        queue.enqueueNDRangeKernel(iterate, 0, NDRange(info.bWidth * info.bHeight / 4));
    } else {
        queue.enqueueNDRangeKernel(iterate, 0, NDRange(info.bWidth * info.bHeight));
    }
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, bitmap.pixels.get());

    return bitmap;
}

std::future<Bitmap<RGBColor>> ClGenerator::enqueueMandelbrot(long width, long height, float x, float y, float fwidth)
{
    x = x - fwidth / 2;
    y = y - fwidth * height / width / 2;
    auto mandelCreator = [width, height, x, y, fwidth, this] () -> Bitmap<RGBColor> {
        ::size_t bufferSize = width * height * sizeof(float);
        Bitmap<float> bitmap{ width, height };
        Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);
        //CommandQueue queue(context, device);
        //queue.enqueueWriteBuffer(buffer_A, CL_TRUE, 0, bufferSize, A);

        /*float x = -2.3;
        float y = -1.5;*/
        float pixelScale = fwidth / width;

        Kernel iterate = Kernel(program, "iterate");
        iterate.setArg(0, buffer_A);
        iterate.setArg(1, width);
        iterate.setArg(2, x);
        iterate.setArg(3, y);
        iterate.setArg(4, pixelScale);

        queue.enqueueNDRangeKernel(iterate, NullRange, NDRange(width * height), NDRange(32));


        queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, bitmap.pixels.get());

        auto converted = bitmap.map<RGBColor>([](float i) { return i < 0 ? RGBColor{ 0,0,0 } : RGBColor{ uint8_t(cos(i * 0.015f) * 127 + 127), uint8_t(sin(i * 0.01f) * 127 + 127), uint8_t(i) }; });//uint8_t(::sin(i * 0.01f) * 100 + 100), uint8_t(i) }; });
        return converted;
    };
    //return std::future<Bitmap<RGBColor>(mandelCreator(), );
    return std::async(/*std::launch::deferred,*/ mandelCreator);
}


/*
std::future<Bitmap<RGBColor>> createMandelbrot()
{
    auto mandelCreator = [] () -> Bitmap<RGBColor> {
        Bitmap<int> bitmap{1024, 1024};
        calculateMandel(bitmap);
        return bitmap.map<RGBColor>([](int x) { return RGBColor{ unsigned char(x), unsigned char(x), unsigned char(x) }; });
    };
    return std::async(mandelCreator);
}

*/

std::future<Bitmap<RGBColor>> createHPM()
{
    /*auto mandelCreator = [] () -> Bitmap<RGBColor> {
        Fixed128 smallFact { 10000ULL, 0 };
        Bitmap<float> bitmap{ 128, 128 };
        for (::size_t y = 0; y < bitmap.height; y++) {
            for (::size_t x = 0; x < bitmap.width; x++) {
                Fixed128 a = Fixed128(x) * smallFact;
                Fixed128 b = Fixed128(y) * smallFact;
                bitmap.get(x, y) = iterate<Fixed128>(a, b, 250);
            }
        }
        return bitmap.map<RGBColor>([](float i) { return i < 0 ? RGBColor{ 0,0,0 } : RGBColor{ uint8_t(cos(i * 0.015f) * 127 + 127), uint8_t(sin(i * 0.01f) * 127 + 127), uint8_t(i) }; });//uint8_t(::sin(i * 0.01f) * 100 + 100), uint8_t(i) }; });
    };*/
    double xx = -10.6;
    double yy = 4.7;
    Fixed128 x = xx;
    Fixed128 y = yy;

    std::cout << double(-x) << " * " << double(-y) << " = " << double(x * y) << " --> " << (xx * yy) << std::endl;
    //exit(0);

    auto mandelCreator = [] () -> Bitmap<RGBColor> {
        Bitmap<float> bitmap{ 512, 512 };
        for (::size_t y = 0; y < bitmap.height; y++) {
            for (::size_t x = 0; x < bitmap.width; x++) {
                Fixed128 a = x * 2.0 / bitmap.width - 1;
                Fixed128 b = y * 2.0 / bitmap.height - 1;
                bitmap.get(x, y) = iterate<Fixed128>(a, b, 250);
            }
        }
        return bitmap.map<RGBColor>([](float i) { return i < 0 ? RGBColor{ 0,0,0 } : RGBColor{ uint8_t(cos(i * 0.015f) * 127 + 127), uint8_t(sin(i * 0.01f) * 127 + 127), uint8_t(i) }; });//uint8_t(::sin(i * 0.01f) * 100 + 100), uint8_t(i) }; });
    };

    return std::async(mandelCreator);
}
