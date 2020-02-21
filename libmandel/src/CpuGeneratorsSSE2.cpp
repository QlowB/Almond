#include "CpuGenerators.h"

#include <immintrin.h>
#include <omp.h>

#include <memory>

using mnd::CpuGenerator;

namespace mnd
{
    template class CpuGenerator<float, mnd::X86_SSE2, false>;
    template class CpuGenerator<float, mnd::X86_SSE2, true>;

    template class CpuGenerator<double, mnd::X86_SSE2, false>;
    template class CpuGenerator<double, mnd::X86_SSE2, true>;
}

template<bool parallel>
void CpuGenerator<float, mnd::X86_SSE2, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = float;
    const MandelViewport& view = info.view;

    if constexpr(parallel)
    omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for schedule(static, 1) if (parallel)
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y) + T(j) * T(view.height / info.bHeight);
        long i = 0;
        for (i; i < info.bWidth; i += 4) {
            __m128 xs = {
                float(view.x + double(i) * view.width / info.bWidth),
                float(view.x + double(i + 1) * view.width / info.bWidth),
                float(view.x + double(i + 2) * view.width / info.bWidth),
                float(view.x + double(i + 3) * view.width / info.bWidth)
            };

            __m128 counter = { 0, 0, 0, 0 };
            __m128 adder = { 1, 1, 1, 1 };

            __m128 threshold = { 16.0f, 16.0f, 16.0f, 16.0f };

            __m128 ys = {y, y, y, y};
            __m128 a = xs;
            __m128 b = ys;

            __m128 resulta = { 0, 0, 0, 0 };
            __m128 resultb = { 0, 0, 0, 0 };

            for (int k = 0; k < info.maxIter; k++) {
                __m128 aa = _mm_mul_ps(a, a);
                __m128 bb = _mm_mul_ps(b, b);
                __m128 abab = _mm_mul_ps(a, b); abab = _mm_add_ps(abab, abab);
                a = _mm_add_ps(_mm_sub_ps(aa, bb), xs);
                b = _mm_add_ps(abab, ys);
                __m128 cmp = _mm_cmple_ps(_mm_add_ps(aa, bb), threshold);
                if (info.smooth) {
                    resulta = _mm_or_ps(_mm_andnot_ps(cmp, resulta), _mm_and_ps(cmp, a));
                    resultb = _mm_or_ps(_mm_andnot_ps(cmp, resultb), _mm_and_ps(cmp, b));
                }
                adder = _mm_and_ps(adder, cmp);
                counter = _mm_add_ps(counter, adder);
                if (_mm_movemask_epi8(_mm_castps_si128(cmp)) == 0) {
                    break;
                }
            }

            auto alignVec = [](float* data) -> float* {
                void* aligned = data;
                ::size_t length = 64;
                std::align(32, 8 * sizeof(float), aligned, length);
                return static_cast<float*>(aligned);
            };

            float resData[32];
            float* ftRes = alignVec(resData);
            float* resa = ftRes + 4;
            float* resb = ftRes + 8;

            _mm_store_ps(ftRes, counter);
            _mm_store_ps(resa, resulta);
            _mm_store_ps(resb, resultb);
            for (int k = 0; k < 4 && i + k < info.bWidth; k++) {
                if (info.smooth)
                    data[i + k + j * info.bWidth] = ftRes[k] <= 0 ? info.maxIter :
                    ftRes[k] >= info.maxIter ? info.maxIter :
                    ((float)ftRes[k]) + 1 - ::log(::log(resa[k] * resa[k] + resb[k] * resb[k]) / 2) / ::log(2.0f);
                else
                    data[i + k + j * info.bWidth] = ftRes[k] > 0 ? float(ftRes[k]) : info.maxIter;
            }
        }
    }
}


template<bool parallel>
void CpuGenerator<double, mnd::X86_SSE2, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = double;
    const MandelViewport& view = info.view;
    if constexpr(parallel)
        omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for schedule(static, 1) if (parallel)
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y) + T(j) * T(view.height / info.bHeight);
        long i = 0;
        for (i; i < info.bWidth; i += 2) {
            __m128d xs = {
                double(view.x + double(i) * view.width / info.bWidth),
                double(view.x + double(i + 1) * view.width / info.bWidth)
            };

            __m128d counter = { 0, 0 };
            __m128d adder = { 1, 1 };

            __m128d threshold = { 16.0f, 16.0f };

            __m128d ys = { y, y };
            __m128d a = xs;
            __m128d b = ys;
            __m128d resulta = { 0, 0 };
            __m128d resultb = { 0, 0 };

            for (int k = 0; k < info.maxIter; k++) {
                __m128d aa = _mm_mul_pd(a, a);
                __m128d bb = _mm_mul_pd(b, b);
                __m128d abab = _mm_mul_pd(a, b); abab = _mm_add_pd(abab, abab);
                a = _mm_add_pd(_mm_sub_pd(aa, bb), xs);
                b = _mm_add_pd(abab, ys);
                __m128d cmp = _mm_cmple_pd(_mm_add_pd(aa, bb), threshold);
                if (info.smooth) {
                    resulta = _mm_or_pd(_mm_andnot_pd(cmp, resulta), _mm_and_pd(cmp, a));
                    resultb = _mm_or_pd(_mm_andnot_pd(cmp, resultb), _mm_and_pd(cmp, b));
                }
                adder = _mm_and_pd(adder, cmp);
                counter = _mm_add_pd(counter, adder);
                if (_mm_movemask_epi8(_mm_castpd_si128(cmp)) == 0) {
                    break;
                }
            }

            auto alignVec = [](double* data) -> double* {
                void* aligned = data;
                ::size_t length = 64;
                std::align(32, 4 * sizeof(double), aligned, length);
                return static_cast<double*>(aligned);
            };

            double resData[16];
            double* ftRes = alignVec(resData);
            double* resa = ftRes + 2;
            double* resb = ftRes + 4;

            _mm_store_pd(ftRes, counter);
            _mm_store_pd(resa, resulta);
            _mm_store_pd(resb, resultb);
            for (int k = 0; k < 2 && i + k < info.bWidth; k++)
                data[i + k + j * info.bWidth] = ftRes[k] > 0 ? ftRes[k] : info.maxIter;

            for (int k = 0; k < 2 && i + k < info.bWidth; k++) {
                if (info.smooth)
                    data[i + k + j * info.bWidth] = ftRes[k] <= 0 ? info.maxIter :
                    ftRes[k] >= info.maxIter ? info.maxIter :
                    ((float)ftRes[k]) + 1 - ::log(::log(resa[k] * resa[k] + resb[k] * resb[k]) / 2) / ::log(2.0f);
                else
                    data[i + k + j * info.bWidth] = ftRes[k] > 0 ? float(ftRes[k]) : info.maxIter;
            }
        }
    }
}