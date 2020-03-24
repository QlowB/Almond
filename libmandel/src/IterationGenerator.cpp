#include "IterationGenerator.h"

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
            data[i + j * info.bWidth] = k;
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
            result = ex.value;
        }
        else if constexpr (std::is_same<T, mnd::Variable>::value) {
            if (ex.name == "z")
                result = z;
            else if (ex.name == "c")
                result = c;
            else if (ex.name == "i")
                result = std::complex{ 0.0, 1.0 };
        }
        else if constexpr (std::is_same<T, mnd::UnaryOperation>::value) {
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


