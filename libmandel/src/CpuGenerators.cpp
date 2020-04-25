#include "CpuGenerators.h"
#include "Types.h"

#ifdef WITH_MPFR
#include "MpfrWrapper.h"
#endif // WITH_MPFR

#include <omp.h>
#include <cmath>

#include <memory>

using mnd::CpuGenerator;

namespace mnd
{
    template class CpuGenerator<float, mnd::NONE, false>;
    template class CpuGenerator<float, mnd::NONE, true>;

    template class CpuGenerator<double, mnd::NONE, false>;
    template class CpuGenerator<double, mnd::NONE, true>;

    template class CpuGenerator<Fixed64, mnd::NONE, false>;
    template class CpuGenerator<Fixed64, mnd::NONE, true>;

    template class CpuGenerator<Fixed128, mnd::NONE, false>;
    template class CpuGenerator<Fixed128, mnd::NONE, true>;

    //template class CpuGenerator<Fixed128, mnd::NONE, false>;
    //template class CpuGenerator<Fixed128, mnd::NONE, false, true>;
    //template class CpuGenerator<Fixed128, mnd::NONE, true>;
    //template class CpuGenerator<Fixed128, mnd::NONE, true, true>;


#ifdef WITH_BOOST
#include <boost/multiprecision/cpp_bin_float.hpp>
    template class CpuGenerator<mnd::Float128, mnd::NONE, false>;
    template class CpuGenerator<mnd::Float128, mnd::NONE, true>;

    template class CpuGenerator<mnd::Float256, mnd::NONE, false>;
    template class CpuGenerator<mnd::Float256, mnd::NONE, true>;

    template class CpuGenerator<Fixed512, mnd::NONE, false>;
    template class CpuGenerator<Fixed512, mnd::NONE, true>;
#endif

#ifdef WITH_QD
    template class CpuGenerator<mnd::DoubleDouble, mnd::NONE, false>;
    template class CpuGenerator<mnd::DoubleDouble, mnd::NONE, true>;

    template class CpuGenerator<mnd::QuadDouble, mnd::NONE, false>;
    template class CpuGenerator<mnd::QuadDouble, mnd::NONE, true>;
#endif
}


template<typename T, bool parallel>
void CpuGenerator<T, mnd::NONE, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    const MandelViewport& view = info.view;

    T viewx = mnd::convert<T>(view.x);
    T viewy = mnd::convert<T>(view.y);
    T wpp = mnd::convert<T>(view.width / info.bWidth);
    T hpp = mnd::convert<T>(view.height / info.bHeight);

    T juliaX = mnd::convert<T>(info.juliaX);
    T juliaY = mnd::convert<T>(info.juliaY);

#if defined(_OPENMP)
    if constexpr (parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < info.bHeight; j++) {
        T y = viewy + T(double(j)) * hpp;
        for (long i = 0; i < info.bWidth; i++) {
            T x = viewx + T(double(i)) * wpp;

            T a = x;
            T b = y;

            T cx = info.julia ? juliaX : x;
            T cy = info.julia ? juliaY : y;

            int k = 0;
            for (k = 0; k < info.maxIter; k++) {
                T aa = a * a;
                T bb = b * b;
                T ab = a * b;
                a = aa - bb + cx;
                b = ab + ab + cy;
                if (aa + bb > T(16.0)) {
                    break;
                }
            }
            if (info.smooth) {
                if (k >= info.maxIter)
                    data[i + j * info.bWidth] = float(info.maxIter);
                else {
                    float aapp = mnd::convert<float>(a);
                    float bapp = mnd::convert<float>(b);
                    data[i + j * info.bWidth] = ((float) k) + 1 - ::logf(::logf(aapp * aapp + bapp * bapp) / 2) / ::logf(2.0f);
                }
            }
            else
                data[i + j * info.bWidth] = float(k);
        }
    }
}


/*
template<bool parallel>
void CpuGenerator<double, mnd::NONE, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    const MandelViewport& view = info.view;

    T viewx = mnd::convert<T>(view.x);
    T viewy = mnd::convert<T>(view.y);
    T wpp = mnd::convert<T>(view.width / info.bWidth);
    T hpp = mnd::convert<T>(view.height / info.bHeight);

    if constexpr (parallel)
        omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for schedule(static, 1) if (parallel)
    for (long j = 0; j < info.bHeight; j++) {
        T y = viewy + T(double(j)) * hpp;
        long i = 0;
        for (i; i < info.bWidth; i++) {
            T x = viewx + T(double(i)) * wpp;

            T a = x;
            T b = y;

            int k = 0;
            for (k = 0; k < info.maxIter; k++) {
                T aa = a * a;
                T bb = b * b;
                T ab = a * b;
                a = aa - bb + x;
                b = ab + ab + y;
                if (aa + bb > T(16.0)) {
                    break;
                }
            }
            if (info.smooth) {
                if (k >= info.maxIter)
                    data[i + j * info.bWidth] = float(info.maxIter);
                else
                    data[i + j * info.bWidth] = ((float) k) + 1 - ::logf(::logf(mnd::convert<float>(a * a + b * b)) / 2) / ::logf(2.0f);
            }
            else
                data[i + j * info.bWidth] = k;
        }
    }
}*/

/*
#if defined(WITH_BOOST) || 1
template<bool parallel>
void CpuGenerator<Fixed128, mnd::NONE, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = Fixed128;
    const MandelViewport& view = info.view;

    const auto fixedFromFloat = [] (const mnd::Float128& f) {
        boost::multiprecision::int128_t frac = boost::multiprecision::int128_t(f * 4294967296.0 * 4294967296.0 * 4294967296.0);
        std::vector<uint32_t> bits;
        export_bits(frac, std::back_inserter(bits), 32);
        bits.clear();
        while (bits.size() < 4) bits.push_back(0);
        return Fixed128{ bits[0], bits[1], bits[2], bits[3] };
    };

    if constexpr (parallel)
        omp_set_num_threads(2 * omp_get_num_procs());
#pragma omp parallel for if (parallel)
    for (long j = 0; j < info.bHeight; j++) {
        T y = fixedFromFloat(view.y + mnd::Real(j) * view.height / info.bHeight);
        long i = 0;
        for (i; i < info.bWidth; i++) {
            T x = fixedFromFloat(view.x + mnd::Real(i) * view.width / info.bWidth);

            T a = x;
            T b = y;

            int k = 0;
            for (k = 0; k < info.maxIter; k++) {
                T aa = a * a;
                T bb = b * b;
                T ab = a * b;
                a = aa - bb + x;
                b = ab + ab + y;
                if (aa + bb > T(16)) {
                    break;
                }
            }
            if constexpr (smooth) {
                if (k >= info.maxIter)
                    data[i + j * info.bWidth] = info.maxIter;
                else
                    data[i + j * info.bWidth] = ((float) k) + 1 - ::logf(::logf(float(a * a + b * b)) / 2) / ::logf(2.0f);
            }
            else
                data[i + j * info.bWidth] = k;
        }
    }
}
#endif // WITH_BOOST
*/

#ifdef WITH_MPFR
template<unsigned int bits, bool parallel>
void CpuGenerator<mnd::MpfrFloat<bits>, mnd::NONE, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    const MandelViewport& view = info.view;
    using T = mnd::MpfrFloat<bits>;

#if defined(_OPENMP)
    if constexpr (parallel)
        omp_set_num_threads(2 * omp_get_num_procs());
#   pragma omp parallel for if (parallel)
#endif
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y) + T(j) * T(view.height / info.bHeight);
        long i = 0;
        for (i; i < info.bWidth; i++) {
            T x = T(view.x + T(i) * T(view.width / info.bWidth));

            T a = x;
            T b = y;

            int k = 0;
            for (k = 0; k < info.maxIter; k++) {
                T aa = a * a;
                T bb = b * b;
                T ab = a * b;
                a = aa - bb + x;
                b = ab + ab + y;
                if (aa + bb > T(16)) {
                    break;
                }
            }
            if (info.smooth) {
                if (k >= info.maxIter)
                    data[i + j * info.bWidth] = info.maxIter;
                else
                    data[i + j * info.bWidth] = ((float) k) + 1 - ::log(::log(a * a + b * b) / 2) / ::log(2.0f);
            }
            else
                data[i + j * info.bWidth] = k;
        }
    }
}
#endif // WITH_MPFR


/*
void CpuGeneratorDouble::generate(const mnd::MandelInfo& info, float* data)
{
    const MandelViewport& view = info.view;
    omp_set_num_threads(2 * omp_get_num_procs());
#pragma omp parallel for
    for (long j = 0; j < info.bHeight; j++) {
        double y = double(view.y) + double(j) * double(view.height / info.bHeight);
        long i = 0;
        for (i; i < info.bWidth; i++) {
            double x = view.x + double(i) * view.width / info.bWidth;

            double a = x;
            double b = y;

            int k = 0;
            for (k = 0; k < info.maxIter; k++) {
                double aa = a * a;
                double bb = b * b;
                double ab = a * b;
                a = aa - bb + x;
                b = ab + ab + y;
                if (aa + bb > 16) {
                    break;
                }
            }
            data[i + j * info.bWidth] = k;
        }
    }
}


void CpuGenerator128::generate(const mnd::MandelInfo& info, float* data)
{
    const MandelViewport& view = info.view;
    omp_set_num_threads(2 * omp_get_num_procs());
#pragma omp parallel for
    for (long j = 0; j < info.bHeight; j++) {
        Fixed128 y = Fixed128(view.y) + Fixed128(j) * Fixed128(view.height / info.bHeight);
        long i = 0;
        for (i; i < info.bWidth; i++) {
            Fixed128 x = view.x + Fixed128(i) * Fixed128(view.width / info.bWidth);

            Fixed128 a = x;
            Fixed128 b = y;

            int k = 0;
            for (k = 0; k < info.maxIter; k++) {
                Fixed128 aa = a * a;
                Fixed128 bb = b * b;
                Fixed128 ab = a * b;
                a = aa - bb + x;
                b = ab + ab + y;
                if (aa + bb > Fixed128(16)) {
                    break;
                }
            }

            data[i + j * info.bWidth] = k;
        }
    }
}
*/
