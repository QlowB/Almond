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
    platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if (all_devices.size() == 0) {
        std::cout << "No devices found. Check OpenCL installation!\n";
        exit(1);
    }

    if (display) {
        for (::size_t j = 0; j < all_devices.size(); j++)
            printf("Device %d: %s\n", int(j), all_devices[j].getInfo<CL_DEVICE_NAME>().c_str());
    }
    return all_devices[i];
}


ClGenerator::ClGenerator(void)
{
    Platform p = getPlatform();
    device = getDevice(p, 0, false);
    context = Context{ device };
    Program::Sources sources;

    std::string kcode_alt =
        "void kernel iterate(global float* A, const int width, float xl, float yt, float pixelScale, int max) {"
        "   int x = get_global_id(0) % width;"
        "   int y = get_global_id(0) / width;"
        "   float a = x * pixelScale + xl;"
        "   float b = y * pixelScale + yt;"
        "   float ca = a;"
        "   float cb = b;"
        ""
        "   int n = 0;"
        "   while (n < max) {"
        "       float aa = a * a;"
        "       float bb = b * b;"
        "       float ab = a * b;"
        "       a = aa - bb + ca;"
        "       b = 2 * ab + cb;"
        "       if (aa + bb > 16) break;"
        "       n++;"
        "   }\n"
        "   A[get_global_id(0)] = n;//((float)n) + (a + b - 16) / (256 - 16);\n"
//        "   A[get_global_id(0)] = 5;"
        "}";

    std::string kcode =
        "void kernel iterate(global float* A, const int width, float xl, float yt, float pixelScale, int max) {"
        "   int x = get_global_id(0) % (width);"
        "   int y = get_global_id(0) / (width);"
        "   float a = x * pixelScale + xl;"
        "   float b = y * pixelScale + yt;"
        "   float ca = a;"
        "   float cb = b;"
        ""
        "   int n = 0;"
        "   while (n < max) {"
        "       float aa = a * a;"
        "       float bb = b * b;"
        "       float ab = a * b;"
        "       a = aa - bb + ca;"
        "       b = 2 * ab + cb;"
        "       if (aa + bb > 16) break;"
        "       n++;"
        "   }\n"
        "   A[get_global_id(0)] = n;//((float)n) + (a + b - 16) / (256 - 16);\n"
//        "   A[get_global_id(0)] = 5;"
        "}";

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
    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);
    float pixelScale = info.view.width / info.bWidth;

    Kernel iterate = Kernel(program, "iterate");
    iterate.setArg(0, buffer_A);
    iterate.setArg(1, info.bWidth);
    iterate.setArg(2, float(info.view.x));
    iterate.setArg(3, float(info.view.y));
    iterate.setArg(4, pixelScale);
    iterate.setArg(5, info.maxIter);

    queue.enqueueNDRangeKernel(iterate, NullRange, NDRange(info.bWidth * info.bHeight), NDRange(32));
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
