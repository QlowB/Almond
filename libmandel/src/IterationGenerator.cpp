#include "IterationGenerator.h"
#include "ExecData.h"
#include "Mandel.h"

#include "OpenClInternal.h"


#include <omp.h>

using mnd::IterationGenerator;
using mnd::NaiveGenerator;
using mnd::IterationFormula;


IterationGenerator::IterationGenerator(IterationFormula itf,
                                   const mnd::Real& prec) :
    mnd::MandelGenerator{ prec },
    itf{ std::move(itf) }
{
}


NaiveGenerator::NaiveGenerator(IterationFormula itf,
                                   const mnd::Real& prec) :
    IterationGenerator{ std::move(itf), prec }
{
    this->itf.optimize();
}


void NaiveGenerator::generate(const mnd::MandelInfo& info, float* data)
{
    const MandelViewport& view = info.view;

    const bool parallel = true;
    using T = double;
    T viewx = mnd::convert<T>(view.x);
    T viewy = mnd::convert<T>(view.y);
    T wpp = mnd::convert<T>(view.width / info.bWidth);
    T hpp = mnd::convert<T>(view.height / info.bHeight);

    T juliaX = mnd::convert<T>(info.juliaX);
    T juliaY = mnd::convert<T>(info.juliaY);

    if constexpr (parallel)
        omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for schedule(static, 1) if (parallel)
    for (long j = 0; j < info.bHeight; j++) {
        T y = viewy + T(double(j)) * hpp;
        long i = 0;
        for (i; i < info.bWidth; i++) {
            T x = viewx + T(double(i)) * wpp;

            T cx = info.julia ? juliaX : x;
            T cy = info.julia ? juliaY : y;
            std::complex<double> z{ x, y };
            if (!info.julia) {
                z = 0;
            }
            std::complex<double> c{ cx, cy };

            int k = 0;
            for (k = 0; k < info.maxIter; k++) {
                z = this->iterate(z, c);
                if (std::abs(z) >= 4)
                    break;
            }
            data[i + j * info.bWidth] = float(k);
            /*if (info.smooth) {
                if (k >= info.maxIter)
                    data[i + j * info.bWidth] = float(info.maxIter);
                else {
                    float aapp = mnd::convert<float>(a);
                    float bapp = mnd::convert<float>(b);
                    data[i + j * info.bWidth] = ((float) k) + 1 - ::logf(::logf(aapp * aapp + bapp * bapp) / 2) / ::logf(2.0f);
                }
            }
            else
                data[i + j * info.bWidth] = k;*/
        }
    }
}


std::complex<double> NaiveGenerator::iterate(std::complex<double> z, std::complex<double> c)
{
    auto& expr = *itf.expr;
    return calc(expr, z, c);
}


std::complex<double> NaiveGenerator::calc(mnd::Expression& expr, std::complex<double> z, std::complex<double> c)
{
    std::complex<double> result = 0;
    std::visit([this, &result, z, c] (auto&& ex) {
        using T = std::decay_t<decltype(ex)>;
        if constexpr (std::is_same<T, mnd::Constant>::value) {
            result = std::complex{ mnd::convert<double>(ex.re), mnd::convert<double>(ex.im) };
        }
        else if constexpr (std::is_same<T, mnd::Variable>::value) {
            if (ex.name == "z")
                result = z;
            else if (ex.name == "c")
                result = c;
            else if (ex.name == "i")
                result = std::complex{ 0.0, 1.0 };
        }
        else if constexpr (std::is_same<T, mnd::Negation>::value) {
            result = -calc(*ex.operand, z, c);
        }
        else if constexpr (std::is_same<T, mnd::Addition>::value) {
            if (ex.subtraction)
                result = calc(*ex.left, z, c) - calc(*ex.right, z, c);
            else
                result = calc(*ex.left, z, c) + calc(*ex.right, z, c);
        }
        else if constexpr (std::is_same<T, mnd::Multiplication>::value) {
            result = calc(*ex.left, z, c) * calc(*ex.right, z, c);
        }
        else if constexpr (std::is_same<T, mnd::Division>::value) {
            result = calc(*ex.left, z, c) / calc(*ex.right, z, c);
        }
        else if constexpr (std::is_same<T, mnd::Pow>::value) {
            result = std::pow(calc(*ex.left, z, c), calc(*ex.right, z, c));
        }
    }, expr);
    return result;
}



using mnd::CompiledGenerator;
using mnd::CompiledClGenerator;


CompiledGenerator::CompiledGenerator(std::unique_ptr<mnd::ExecData> execData) :
    MandelGenerator{ 1.0e-15 },
    execData{ std::move(execData) }
{
}


CompiledGenerator::CompiledGenerator(CompiledGenerator&&) = default;


CompiledGenerator::~CompiledGenerator(void)
{
}


/*__declspec(noinline)
int iter(double x, double y, int maxIter)
{
int k = 0;

double a = x;
double b = y;

for (k = 0; k < maxIter; k++) {
double aa = a * a;
double bb = b * b;
double abab = a * b + a * b;
a = aa - bb + x;
b = abab + y;
if (aa + bb >= 16)
break;
}

return k;
}*/



void CompiledGenerator::generate(const mnd::MandelInfo& info, float* data)
{
    using IterFunc = int (*)(double, double, int);

    omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for schedule(static, 1)
    for (int i = 0; i < info.bHeight; i++) {
        double y = mnd::convert<double>(info.view.y + info.view.height * i / info.bHeight);
        for (int j = 0; j < info.bWidth; j++) {
            double x = mnd::convert<double>(info.view.x + info.view.width * j / info.bWidth);
            IterFunc iterFunc = asmjit::ptr_as_func<IterFunc>(this->execData->iterationFunc);
            int k = iterFunc(x, y, info.maxIter);
            data[i * info.bWidth + j] = k;
        }
    }
}


std::string CompiledGenerator::dump(void) const
{
    asmjit::String d;
    execData->compiler->dump(d);
    return d.data();
}


#ifdef WITH_OPENCL
CompiledClGenerator::CompiledClGenerator(const mnd::MandelDevice& device, const std::string& code) :
    ClGeneratorFloat{ device.getClDevice().device, code }
{
}


std::string CompiledClGenerator::getKernelCode(bool smooth) const
{
    return "";
}

void CompiledClGenerator::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    cl::Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);
    float pixelScaleX = float(info.view.width / info.bWidth);
    float pixelScaleY = float(info.view.height / info.bHeight);

    static cl::Kernel iterate = cl::Kernel(program, "iterate");
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

    queue.enqueueNDRangeKernel(iterate, 0, cl::NDRange(info.bWidth * info.bHeight));

    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);
}

#endif // WITH_OPENCL


