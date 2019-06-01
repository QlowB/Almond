#include "CpuGenerators.h"
#include "Fixed.h"

#include <omp.h>
#include <cmath>

#include <memory>

using mnd::CpuGenerator;

template class CpuGenerator<float, mnd::NONE, false, false>;
template class CpuGenerator<float, mnd::NONE, false, true>;
template class CpuGenerator<float, mnd::NONE, true, false>;
template class CpuGenerator<float, mnd::NONE, true, true>;

template class CpuGenerator<double, mnd::NONE, false, false>;
template class CpuGenerator<double, mnd::NONE, false, true>;
template class CpuGenerator<double, mnd::NONE, true, false>;
template class CpuGenerator<double, mnd::NONE, true, true>;

template class CpuGenerator<Fixed128, mnd::NONE, false, false>;
template class CpuGenerator<Fixed128, mnd::NONE, false, true>;
template class CpuGenerator<Fixed128, mnd::NONE, true, false>;
template class CpuGenerator<Fixed128, mnd::NONE, true, true>;


template<typename T, bool parallel, bool smooth>
void CpuGenerator<T, mnd::NONE, parallel, smooth>::generate(const mnd::MandelInfo& info, float* data)
{
    const MandelViewport& view = info.view;

    if constexpr (parallel)
        omp_set_num_threads(2 * omp_get_num_procs());
#pragma omp parallel for if (parallel)
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
            if constexpr (smooth) {
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
