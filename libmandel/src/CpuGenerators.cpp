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

    template class CpuGenerator<mnd::TripleDouble, mnd::NONE, false>;
    template class CpuGenerator<mnd::TripleDouble, mnd::NONE, true>;

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

namespace mnd
{
    template class CpuGenerator<float, mnd::X86_AVX, false>;
    template class CpuGenerator<float, mnd::X86_AVX, true>;

    template class CpuGenerator<double, mnd::X86_AVX, false>;
    template class CpuGenerator<double, mnd::X86_AVX, true>;

    template class CpuGenerator<DoubleDouble, mnd::X86_AVX, false>;
    template class CpuGenerator<DoubleDouble, mnd::X86_AVX, true>;

    template class CpuGenerator<TripleDouble, mnd::X86_AVX, false>;
    template class CpuGenerator<TripleDouble, mnd::X86_AVX, true>;
}

extern void generateFloatAvx(long width, long height, float* data, bool parallel,
    float vx, float vy, float vw, float vh, int maxIter, bool smooth,
    bool julia, float jX, float jY);

extern void generateDoubleAvx(long width, long height, float* data, bool parallel,
    double vx, double vy, double vw, double vh, int maxIter, bool smooth,
    bool julia, double jX, double jY);

extern void generateDoubleDoubleAvx(long width, long height, float* data, bool parallel,
    double vx1, double vx2, double vy1, double vy2, double vw1, double vw2, double vh1, double vh2, int maxIter, bool smooth,
    bool julia, double jX1, double jX2, double jY1, double jY2);

extern void generateTripleDoubleAvx(long width, long height, float* data, bool parallel,
    double vx1, double vx2, double vx3, double vy1, double vy2, double vy3,
    double vw1, double vw2,  double vw3, double vh1, double vh2, double vh3,
    int maxIter, bool smooth, bool julia,
    double jX1, double jX2, double jX3, double jY1, double jY2, double jY3);


template<bool parallel>
void CpuGenerator<float, mnd::X86_AVX, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = float;
    const MandelViewport& view = info.view;

    const T vx = mnd::convert<T>(view.x);
    const T vy = mnd::convert<T>(view.y);
    const T vw = mnd::convert<T>(view.width);
    const T vh = mnd::convert<T>(view.height);

    T jX = mnd::convert<T>(info.juliaX);
    T jY = mnd::convert<T>(info.juliaY);

    generateFloatAvx(info.bWidth, info.bHeight, data, parallel, vx, vy, vw, vh, info.maxIter, info.smooth, info.julia, jX, jY);
}


template<bool parallel>
void CpuGenerator<double, mnd::X86_AVX, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = double;
    const MandelViewport& view = info.view;

    const T vx = mnd::convert<T>(view.x);
    const T vy = mnd::convert<T>(view.y);
    const T vw = mnd::convert<T>(view.width);
    const T vh = mnd::convert<T>(view.height);

    T jX = mnd::convert<T>(info.juliaX);
    T jY = mnd::convert<T>(info.juliaY);

    generateDoubleAvx(info.bWidth, info.bHeight, data, parallel, vx, vy, vw, vh, info.maxIter, info.smooth, info.julia, jX, jY);
}


template<bool parallel>
void CpuGenerator<mnd::DoubleDouble, mnd::X86_AVX, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = mnd::DoubleDouble;
    const MandelViewport& view = info.view;

    const T vx = mnd::convert<T>(view.x);
    const T vy = mnd::convert<T>(view.y);
    const T vw = mnd::convert<T>(view.width);
    const T vh = mnd::convert<T>(view.height);

    T jX = mnd::convert<T>(info.juliaX);
    T jY = mnd::convert<T>(info.juliaY);

    generateDoubleDoubleAvx(info.bWidth, info.bHeight, data, parallel,
        vx.x[0], vx.x[1], vy.x[0], vy.x[1], vw.x[0], vw.x[1], vh.x[0], vh.x[1],
        info.maxIter, info.smooth, info.julia, jX.x[0], jX.x[1], jY.x[0], jY.x[1]);
}


template<bool parallel>
void CpuGenerator<mnd::TripleDouble, mnd::X86_AVX, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = mnd::TripleDouble;
    const MandelViewport& view = info.view;

    const T vx = mnd::convert<T>(view.x);
    const T vy = mnd::convert<T>(view.y);
    const T vw = mnd::convert<T>(view.width);
    const T vh = mnd::convert<T>(view.height);

    T jX = mnd::convert<T>(info.juliaX);
    T jY = mnd::convert<T>(info.juliaY);

    generateTripleDoubleAvx(info.bWidth, info.bHeight, data, parallel,
        vx.x[0], vx.x[1], vx.x[2], vy.x[0], vy.x[1], vy.x[2],
        vw.x[0], vw.x[1], vw.x[2], vh.x[0], vh.x[1], vh.x[2],
        info.maxIter, info.smooth, info.julia,
        jX.x[0], jX.x[1], jX.x[2], jY.x[0], jY.x[1], jY.x[2]);
}


#ifdef WITH_AVX512

namespace mnd
{
    template class CpuGenerator<float, mnd::X86_AVX_512, false>;
    template class CpuGenerator<float, mnd::X86_AVX_512, true>;

    //template class CpuGenerator<double, mnd::X86_AVX_512, false>;
    //template class CpuGenerator<double, mnd::X86_AVX_512, true>;
}

extern void generateFloatAvx512(long width, long height, float* data, bool parallel,
    float vx, float vy, float vw, float vh, int maxIter, bool smooth,
    bool julia, float jX, float jY);

template<bool parallel>
void CpuGenerator<float, mnd::X86_AVX_512, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = float;
    const MandelViewport& view = info.view;

    const T vx = mnd::convert<T>(view.x);
    const T vy = mnd::convert<T>(view.y);
    const T vw = mnd::convert<T>(view.width);
    const T vh = mnd::convert<T>(view.height);

    T jX = mnd::convert<T>(info.juliaX);
    T jY = mnd::convert<T>(info.juliaY);

    generateFloatAvx512(info.bWidth, info.bHeight, data, parallel, vx, vy, vw, vh, info.maxIter, info.smooth, info.julia, jX, jY);
}

#endif // WITH_AVX512

