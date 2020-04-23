#include "IterationGenerator.h"
#include "ExecData.h"
#include "Mandel.h"

#include "OpenClInternal.h"


#include <omp.h>

using mnd::IterationGenerator;
using mnd::NaiveGenerator;
using mnd::NaiveIRGenerator;
using mnd::IterationFormula;


namespace mnd
{
    template class NaiveIRGenerator<float>;
    template class NaiveIRGenerator<double>;
    template class NaiveIRGenerator<mnd::DoubleDouble>;
    template class NaiveIRGenerator<mnd::QuadDouble>;
}


IterationGenerator::IterationGenerator(IterationFormula z0, IterationFormula zi,
            mnd::Precision prec, mnd::CpuExtension ex) :
    mnd::MandelGenerator{ prec, ex },
    z0{ std::move(z0) },
    zi{ std::move(zi) }
{
}


NaiveGenerator::NaiveGenerator(IterationFormula z0, IterationFormula zi,
            mnd::Precision prec, mnd::CpuExtension ex) :
    IterationGenerator{ std::move(z0), std::move(zi), prec, ex }
{
    this->z0.optimize();
    this->zi.optimize();
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

    if constexpr (parallel)
        omp_set_num_threads(omp_get_num_procs());
//#pragma omp parallel for schedule(static, 1) if (parallel)
    for (long j = 0; j < info.bHeight; j++) {
        T y = viewy + T(double(j)) * hpp;
        long i = 0;
        for (i; i < info.bWidth; i++) {
            T x = viewx + T(double(i)) * wpp;

            T cx = x;
            T cy = y;
            std::complex<double> z = calc(*z0.expr, { 0, 0 }, { x, y });
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
    auto& expr = *zi.expr;
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


template<typename T>
NaiveIRGenerator<T>::NaiveIRGenerator(const mnd::ir::Formula& irf,
            mnd::Precision prec) :
    mnd::MandelGenerator{ prec },
    form{ irf }
{
}


template<typename U>
void NaiveIRGenerator<U>::generate(const mnd::MandelInfo& info, float* data)
{
    const MandelViewport& view = info.view;

    const bool parallel = true;
    using T = double;
    T viewx = mnd::convert<T>(view.x);
    T viewy = mnd::convert<T>(view.y);
    T wpp = mnd::convert<T>(view.width / info.bWidth);
    T hpp = mnd::convert<T>(view.height / info.bHeight);

    if constexpr (parallel)
        omp_set_num_threads(omp_get_num_procs());
//#pragma omp parallel for schedule(static, 1) if (parallel)
    for (long j = 0; j < info.bHeight; j++) {
        T y = viewy + T(double(j)) * hpp;
        long i = 0;
        for (i; i < info.bWidth; i++) {
            T x = viewx + T(double(i)) * wpp;

            T a = calc(form.startA, x, y, 0, 0);
            T b = calc(form.startB, x, y, 0, 0);

            int k = 0;
            for (k = 0; k < info.maxIter; k++) {
                double newA = calc(form.newA, a, b, x, y);
                double newB = calc(form.newB, a, b, x, y);
                a = newA;
                b = newB;
                if (a * a + b * b >= 16.0)
                    break;
            }
            data[i + j * info.bWidth] = float(k);
        }
    }
}


template<typename T>
struct EvalNode
{

};


template<typename T>
double NaiveIRGenerator<T>::calc(mnd::ir::Node* expr, double a, double b, double x, double y)
{
    struct DoubleVisitor
    {
        double a, b, x, y;
        double visitNode(ir::Node* n) {
            auto& nodeData = getNodeData(n);
            return std::visit(*this, *n);
        }

        std::any& getNodeData(ir::Node* n) {
            return std::visit([](auto& n) {
                return n.nodeData;
            }, *n);
        }

        double operator()(const ir::Constant& c) {
            return mnd::convert<double>(c.value);
        }

        double operator()(const ir::Variable& v) {
            if (v.name == "z_re")
                return a;
            else if (v.name == "z_im")
                return b;
            else if (v.name == "c_re")
                return x;
            else if (v.name == "c_im")
                return y;
            else
                return 0.0;
        }

        double operator()(const ir::Negation& n) {
            return -visitNode(n.value);
        }

        double operator()(const ir::Addition& n) {
            return visitNode(n.left) + visitNode(n.right);
        }

        double operator()(const ir::Subtraction& n) {
            return visitNode(n.left) - visitNode(n.right);
        }

        double operator()(const ir::Multiplication& n) {
            return visitNode(n.left) * visitNode(n.right);
        }

        double operator()(const ir::Division& n) {
            return visitNode(n.left) / visitNode(n.right);
        }

        double operator()(const ir::Atan2& n) {
            return ::atan2(visitNode(n.left), visitNode(n.right));
        }

        double operator()(const ir::Pow& n) {
            return ::pow(visitNode(n.left), visitNode(n.right));
        }

        double operator()(const ir::Cos& n) {
            return ::cos(visitNode(n.value));
        }

        double operator()(const ir::Sin& n) {
            return ::sin(visitNode(n.value));
        }

        double operator()(const ir::Exp& n) {
            return ::exp(visitNode(n.value));
        }

        double operator()(const ir::Ln& n) {
            return ::log(visitNode(n.value));
        }
    };

    DoubleVisitor dv{ a, b, x, y };
    return dv.visitNode(expr);
}


using mnd::CompiledGenerator;
using mnd::CompiledGeneratorVec;
using mnd::CompiledClGenerator;
using mnd::CompiledClGeneratorDouble;


CompiledGenerator::CompiledGenerator(std::unique_ptr<mnd::ExecData> execData,
    mnd::Precision prec, mnd::CpuExtension ex) :
    MandelGenerator{ prec, ex },
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
            data[i * info.bWidth + j] = float(k);
        }
    }
}


std::string CompiledGenerator::dump(void) const
{
    asmjit::String d;
    execData->compiler->dump(d);
    return d.data();
}


CompiledGeneratorVec::CompiledGeneratorVec(std::unique_ptr<mnd::ExecData> execData) :
    CompiledGenerator{ std::move(execData), mnd::Precision::FLOAT, mnd::CpuExtension::X86_AVX }
{
}


CompiledGeneratorVec::CompiledGeneratorVec(CompiledGeneratorVec&&) = default;


CompiledGeneratorVec::~CompiledGeneratorVec(void)
{
}


void CompiledGeneratorVec::generate(const mnd::MandelInfo& info, float* data)
{
    using IterFunc = int (*)(float, float, float, int, float*);

    double dx = mnd::convert<double>(info.view.width / info.bWidth);

    omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for schedule(static, 1)
    for (int i = 0; i < info.bHeight; i++) {
        double y = mnd::convert<double>(info.view.y + info.view.height * i / info.bHeight);
        for (int j = 0; j < info.bWidth; j += 8) {
            double x = mnd::convert<double>(info.view.x + info.view.width * j / info.bWidth);
            float result[8];
            IterFunc iterFunc = asmjit::ptr_as_func<IterFunc>(this->execData->iterationFunc);
            int k = iterFunc(x, y, dx, info.maxIter-1, result);

            for (int k = 0; k < 8 && j + k < info.bWidth; k++)
                data[i * info.bWidth + j + k] = result[k];
        }
    }
}


#ifdef WITH_OPENCL
CompiledClGenerator::CompiledClGenerator(mnd::MandelDevice& device, const std::string& code) :
    ClGeneratorFloat{ device, code }
{
    kernel = cl::Kernel(program, "iterate");
}


void CompiledClGenerator::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    cl::Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);
    float pixelScaleX = float(info.view.width / info.bWidth);
    float pixelScaleY = float(info.view.height / info.bHeight);

    //static cl::Kernel iterate = cl::Kernel(program, "iterate");
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

    queue.enqueueNDRangeKernel(kernel, 0, cl::NDRange(info.bWidth * info.bHeight));

    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);
}


CompiledClGeneratorDouble::CompiledClGeneratorDouble(mnd::MandelDevice& device, const std::string& code) :
    ClGeneratorDouble{ device, code }
{
}

#endif // WITH_OPENCL


