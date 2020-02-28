#include "ClGenerators.h"
#include "OpenClCode.h"

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


ClGenerator::ClGenerator(cl::Device device, const mnd::Real& precision) :
    MandelGenerator{ precision },
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

    bool useVec = device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT>() >= 4;

    Kernel iterate = Kernel(program, useVec ? "iterate_vec4" : "iterate");
    iterate.setArg(0, buffer_A);
    iterate.setArg(1, int(info.bWidth));
    iterate.setArg(2, float(info.view.x));
    iterate.setArg(3, float(info.view.y));
    iterate.setArg(4, float(pixelScaleX));
    iterate.setArg(5, float(pixelScaleY));
    iterate.setArg(6, int(info.maxIter));
    iterate.setArg(7, int(info.smooth ? 1 : 0));
    iterate.setArg(8, int(info.julia ? 1 : 0));
    iterate.setArg(9, float(info.juliaX));
    iterate.setArg(10, float(info.juliaY));

    if (useVec) {
        queue.enqueueNDRangeKernel(iterate, 0, NDRange(info.bWidth * info.bHeight / 4));
    } else {
        queue.enqueueNDRangeKernel(iterate, 0, NDRange(info.bWidth * info.bHeight));
    }
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);
}


ClGeneratorFloat::ClGeneratorFloat(cl::Device device) :
    ClGenerator{ device, mnd::getPrecision<float>() }
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
    return mnd::getFloat_cl();
}


ClGeneratorDoubleFloat::ClGeneratorDoubleFloat(cl::Device device) :
    ClGenerator{ device, mnd::getPrecision(mnd::Precision::DOUBLE_FLOAT)  }
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


std::pair<float, float> twoSum(float a, float b) {
    float s = a + b;
    float v = s - a;
    float r = (a - (s - v)) + (b - v);
    return { s, r };
}

std::pair<float, float> split(float a) {
    float c = (4096 + 1) * a;
    float abig = c - a;
    float ahi = c - abig;
    float alo = a - ahi;
    return { ahi, alo };
}

std::pair<float, float> twoProd(float a, float b) {
    float x = a * b;
    auto aex = split(a);
    auto bex = split(b);
    float errx = x - (aex.first * bex.first);
    float erry = errx - (aex.second * bex.first);
    float errz = erry - (aex.first * bex.second);
    float y = (aex.second * bex.second) - errz;
    return { x, y };
}

std::pair<float, float> add(std::pair<float, float> a, std::pair<float, float> b) {
    float r = a.first + b.first;
    float s;
    if (fabs(a.first) >= fabs(b.first)) {
        s = (((a.first - r) + b.first) + b.second) + a.second;
    }
    else {
        s = (((b.first - r) + a.first) + a.second) + b.second;
    }
    return twoSum(r, s);
}

std::pair<float, float> mul(std::pair<float, float> a, std::pair<float, float> b) {
    auto t = twoProd(a.first, b.first);
    t.second += ((a.first * b.second) + (a.second * b.first));
    return twoSum(t.first, t.second);
}

std::pair<float, float> mulFloat(std::pair<float, float> a, float b) {
    std::pair<float, float> t = twoProd(a.first, b);
    float t3 = (a.second * b) + t.second;
    return twoSum(t.first, t.second);
}


void ClGeneratorDoubleFloat::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    auto splitDouble = [] (double x) {
        /*uint64_t xl = *((uint64_t*)&x);
        uint64_t mantissa = xl & 0x000FFFFFFFFFFFFFULL;
        uint64_t exp = (xl & 0x7FF0000000000000ULL) >> 53;
        bool sign = (xl & 0x1000000000000000ULL) != 0;

        uint32_t floathi = exp << 23;*/

        float hi = float(x);
        float lo = float(x - double(hi));
        if (abs(lo) >= 1.0e-10f) {
            //printf("hi: %.10ef, lo: %.10ef\n", hi, lo);
            //fflush(stdout);
        }
        return std::pair{ hi, lo };
    };

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);
    double pixelScX = double(info.view.width / info.bWidth);
    double pixelScY = double(info.view.height / info.bHeight);

    auto[x1, x2] = splitDouble(double(info.view.x));
    auto[y1, y2] = splitDouble(double(info.view.y));
    auto[w1, w2] = splitDouble(pixelScX);
    auto[h1, h2] = splitDouble(pixelScY);

    /*
    for (int px = 0; px < info.bWidth; px++) {
        for (int py = 0; py < info.bHeight; py++) {
            std::pair<float, float> xl = { x1, x2 };
            std::pair<float, float> yt = { y1, y2 };
            std::pair<float, float> pixelScaleX = { w1, w2 };
            std::pair<float, float> pixelScaleY = { h1, h2 };

            std::pair<float, float> a = add(mulFloat(pixelScaleX, (float) px), xl); // pixelScaleX * px + xl
            std::pair<float, float> b = add(mulFloat(pixelScaleY, (float) py), yt); // pixelScaleY * py + yt
            std::pair<float, float> ca = a;
            std::pair<float, float> cb = b;

            int n = 0;
            while (n < info.maxIter - 1) {
                std::pair<float, float> aa = mul(a, a);
                std::pair<float, float> bb = mul(b, b);
                std::pair<float, float> ab = mul(a, b);
                if (aa.first + bb.first > 16) break;
                std::pair<float, float> minusbb = { -bb.first, -bb.second };
                a = add(add(aa, minusbb), ca);
                b = add(add(ab, ab), cb);
                n++;
            }

            // N + 1 - log (log  |Z(N)|) / log 2
            if (n >= info.maxIter - 1)
                data[px + py * info.bWidth] = info.maxIter;
            else {
                if (info.smooth)
                    data[px + py * info.bWidth] = ((float) n) + 1 - log(log(a.first * a.first + b.first * b.first ) / 2) / log(2.0f);
                else
                    data[px + py * info.bWidth] = ((float)n);
            }
        }
    }
    return;
    */
    
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
    return getDoubleFloat_cl();
}


ClGeneratorDouble::ClGeneratorDouble(cl::Device device) :
    ClGenerator{ device, mnd::getPrecision<double>() }
{
    context = Context{ device };
    Program::Sources sources;

    std::string kcode = getDouble_cl();

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
    ClGenerator{ device, mnd::getPrecision<DoubleDouble>() }
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
    return getDoubleDouble_cl();
}


ClGeneratorQuadDouble::ClGeneratorQuadDouble(cl::Device device) :
    ClGenerator{ device, mnd::getPrecision<QuadDouble>() }
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

    mnd::QuadDouble x = mnd::convert<mnd::QuadDouble>(info.view.x);
    mnd::QuadDouble y = mnd::convert<mnd::QuadDouble>(info.view.y);

    mnd::QuadDouble psx = mnd::convert<mnd::QuadDouble>(info.view.width / info.bWidth);
    mnd::QuadDouble psy = mnd::convert<mnd::QuadDouble>(info.view.height / info.bHeight);

    Kernel iterate = Kernel(program, "iterate");
    iterate.setArg(0, buffer_A);
    iterate.setArg(1, int(info.bWidth));
    iterate.setArg(2, x.x[0]);
    iterate.setArg(3, x.x[1]);
    iterate.setArg(4, x.x[2]);
    iterate.setArg(5, x.x[3]);
    iterate.setArg(6, y.x[0]);
    iterate.setArg(7, y.x[1]);
    iterate.setArg(8, y.x[2]);
    iterate.setArg(9, y.x[3]);
    iterate.setArg(10, psx.x[0]);
    iterate.setArg(11, psx.x[1]);
    iterate.setArg(12, psx.x[2]);
    iterate.setArg(13, psx.x[3]);
    iterate.setArg(14, psy.x[0]);
    iterate.setArg(15, psy.x[1]);
    iterate.setArg(16, psy.x[2]);
    iterate.setArg(17, psy.x[3]);
    iterate.setArg(18, int(info.maxIter));
    iterate.setArg(19, int(info.smooth ? 1 : 0));

    cl_int result = queue.enqueueNDRangeKernel(iterate, 0, NDRange(info.bWidth * info.bHeight));
    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);

}


std::string ClGeneratorQuadDouble::getKernelCode(bool smooth) const
{
    return getQuadDouble_cl();
}


ClGenerator128::ClGenerator128(cl::Device device) :
    ClGenerator{ device, mnd::getPrecision<Fixed128>() }
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

    using ull = unsigned long long;
    ull x1 = ull(double(info.view.x) * 0x100000000ULL);
    ull x2 = 0;
    ull y1 = ull(double(info.view.y) * 0x100000000ULL);
    ull y2 = 0;
    ull w1 = ull(double(pixelScaleX) * 0x100000000ULL);
    ull w2 = 0;
    ull h1 = ull(double(pixelScaleY) * 0x100000000ULL);
    ull h2 = 0;

    Kernel iterate = Kernel(program, "iterate");
    iterate.setArg(0, buffer_A);
    iterate.setArg(1, int(info.bWidth));
    iterate.setArg(2, ull(x1));
    iterate.setArg(3, ull(x2));
    iterate.setArg(4, ull(y1));
    iterate.setArg(5, ull(y2));
    iterate.setArg(6, ull(w1));
    iterate.setArg(7, ull(w2));
    iterate.setArg(8, ull(h1));
    iterate.setArg(9, ull(h2));
    iterate.setArg(10, int(info.maxIter));
    iterate.setArg(11, int(info.smooth ? 1 : 0));

    queue.enqueueNDRangeKernel(iterate, 0, NDRange(info.bWidth * info.bHeight));
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
    return getFixed512_cl();
}


ClGenerator64::ClGenerator64(cl::Device device) :
    ClGenerator{ device, mnd::getPrecision<Fixed64>() }
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


#include "CpuGenerators.h"
void ClGenerator64::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);
    float pixelScaleX = float(info.view.width / info.bWidth);
    float pixelScaleY = float(info.view.height / info.bHeight);

    using ull = unsigned long long;
    ull x = ull(::round(double(info.view.x) * (1LL << 48)));
    ull y = ull(::round(double(info.view.y) * (1LL << 48)));
    ull w = ull(::round(double(pixelScaleX) * (1LL << 48)));
    ull h = ull(::round(double(pixelScaleY) * (1LL << 48)));
    //x = 0;
    //y = 0;
    
    Kernel iterate = Kernel(program, "iterate");
    iterate.setArg(0, buffer_A);
    iterate.setArg(1, int(info.bWidth));
    iterate.setArg(2, ull(x));
    iterate.setArg(3, ull(y));
    iterate.setArg(4, ull(w));
    iterate.setArg(5, ull(h));
    iterate.setArg(6, int(info.maxIter));
    iterate.setArg(7, int(info.smooth ? 1 : 0));

    queue.enqueueNDRangeKernel(iterate, 0, NDRange(info.bWidth * info.bHeight));
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

