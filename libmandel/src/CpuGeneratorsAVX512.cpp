#include "CpuGenerators.h"

#include <immintrin.h>
#include <omp.h>

#include <memory>

using mnd::CpuGenerator;

namespace mnd
{
    template class CpuGenerator<float, mnd::X86_AVX_512, false>;
    template class CpuGenerator<float, mnd::X86_AVX_512, true>;

    template class CpuGenerator<double, mnd::X86_AVX_512, false>;
    template class CpuGenerator<double, mnd::X86_AVX_512, true>;
}

template<bool parallel>
void CpuGenerator<float, mnd::X86_AVX_512, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = float;
    const MandelViewport& view = info.view;

    const float dppf = float(view.width / info.bWidth);
    const float viewxf = float(view.x);
    __m512 viewx = { viewxf, viewxf, viewxf, viewxf, viewxf, viewxf, viewxf, viewxf,
                     viewxf, viewxf, viewxf, viewxf, viewxf, viewxf, viewxf, viewxf };
    __m512 dpp = { dppf, dppf, dppf, dppf, dppf, dppf, dppf, dppf,
                   dppf, dppf, dppf, dppf, dppf, dppf, dppf, dppf };

    if constexpr(parallel)
        omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for schedule(static, 1) if (parallel)
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y + double(j) * view.height / info.bHeight);
        __m512 ys = { y, y, y, y, y, y, y, y, y, y, y, y, y, y, y, y };
        long i = 0;
        for (i; i < info.bWidth; i += 16) {
            __m512 pixc = { float(i), float(i + 1), float(i + 2), float(i + 3), float(i + 4), float(i + 5), float(i + 6), float(i + 7),
                            float(i + 8), float(i + 9), float(i + 10), float(i + 11), float(i + 12), float(i + 13), float(i + 14), float(i + 15) };
            __m512 xs = _mm512_fmadd_ps(dpp, pixc, viewx);

            __m512 counter = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
            __m512 adder = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
            __m512 two = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2  };
            __m512 resultsa = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
            __m512 resultsb = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

            __m512 threshold = { 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f,
                                 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f };

            __m512 a = xs;
            __m512 b = ys;

            if (info.smooth) {
                for (int k = 0; k < info.maxIter; k++) {
                    __m512 aa = _mm512_mul_ps(a, a);
                    __m512 abab = _mm512_mul_ps(a, b);
                    __mmask16 cmp = _mm512_cmp_ps_mask(_mm512_fmadd_ps(b, b, aa), threshold, _CMP_LE_OQ);
                    a = _mm512_sub_ps(aa, _mm512_fmsub_ps(b, b, xs));
                    b = _mm512_fmadd_ps(two, abab, ys);
                    counter = _mm512_mask_add_ps(counter, cmp, counter, adder);
                    resultsa = _mm512_mask_blend_ps(cmp, resultsa, a);
                    resultsb = _mm512_mask_blend_ps(cmp, resultsb, b);
                    if (cmp == 0) {
                        break;
                    }
                }
            }
            else {
                for (int k = 0; k < info.maxIter; k++) {
                    __m512 aa = _mm512_mul_ps(a, a);
                    __m512 abab = _mm512_mul_ps(a, b);
                    __mmask16 cmp = _mm512_cmp_ps_mask(_mm512_fmadd_ps(b, b, aa), threshold, _CMP_LE_OQ);
                    a = _mm512_sub_ps(aa, _mm512_fmsub_ps(b, b, xs));
                    b = _mm512_fmadd_ps(two, abab, ys);
                    counter = _mm512_mask_add_ps(counter, cmp, counter, adder);
                    if (cmp == 0) {
                        break;
                    }
                }
            }

            auto alignVec = [](float* data) -> float* {
                void* aligned = data;
                ::size_t length = 64 * sizeof(float);
                std::align(64, 48 * sizeof(float), aligned, length);
                return static_cast<float*>(aligned);
            };

            float resData[64];
            float* ftRes = alignVec(resData);
            float* resa = ftRes + 16;
            float* resb = ftRes + 32;
            _mm512_store_ps(ftRes, counter);
            if (info.smooth) {
                _mm512_store_ps(resa, resultsa);
                _mm512_store_ps(resb, resultsb);
            }
            for (int k = 0; k < 16 && i + k < info.bWidth; k++) {
                if (info.smooth) {
                    data[i + k + j * info.bWidth] = ftRes[k] <= 0 ? info.maxIter :
                        ftRes[k] >= info.maxIter ? info.maxIter :
                        ((float)ftRes[k]) + 1 - ::log(::log(resa[k] * resa[k] + resb[k] * resb[k]) / 2) / ::log(2.0f);
                }
                else {
                    data[i + k + j * info.bWidth] = ftRes[k] <= 0 ? info.maxIter : ftRes[k];
                }
            }
        }
    }
}



template<bool parallel>
void CpuGenerator<double, mnd::X86_AVX_512, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = double;
    const MandelViewport& view = info.view;

    const double dppf = double(view.width / info.bWidth);
    const double viewxf = double(view.x);
    __m512d viewx = { viewxf, viewxf, viewxf, viewxf, viewxf, viewxf, viewxf, viewxf };
    __m512d dpp = { dppf, dppf, dppf, dppf, dppf, dppf, dppf, dppf };

    if constexpr(parallel)
        omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for schedule(static, 1) if (parallel)
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y + double(j) * view.height / info.bHeight);
        __m512d ys = { y, y, y, y, y, y, y, y };
        long i = 0;
        for (i; i < info.bWidth; i += 8) {
            __m512d pixc = { double(i), double(i + 1), double(i + 2), double(i + 3), double(i + 4), double(i + 5), double(i + 6), double(i + 7) };
            __m512d xs = _mm512_fmadd_pd(dpp, pixc, viewx);

            __m512d counter = { 0, 0, 0, 0, 0, 0, 0, 0 };
            __m512d adder = { 1, 1, 1, 1, 1, 1, 1, 1 };
            __m512d two = { 2, 2, 2, 2, 2, 2, 2, 2 };
            __m512d resultsa = { 0, 0, 0, 0, 0, 0, 0, 0 };
            __m512d resultsb = { 0, 0, 0, 0, 0, 0, 0, 0 };

            __m512d threshold = { 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f };

            __m512d a = xs;
            __m512d b = ys;

            if (info.smooth) {
                for (int k = 0; k < info.maxIter; k++) {
                    __m512d aa = _mm512_mul_pd(a, a);
                    __m512d ab = _mm512_mul_pd(a, b);
                    __mmask8 cmp = _mm512_cmp_pd_mask(_mm512_fmadd_pd(b, b, aa), threshold, _CMP_LE_OQ);
                    a = _mm512_sub_pd(aa, _mm512_fmsub_pd(b, b, xs));
                    b = _mm512_fmadd_pd(two, ab, ys);
                    resultsa = _mm512_mask_blend_pd(cmp, resultsa, a);
                    resultsb = _mm512_mask_blend_pd(cmp, resultsb, b);
                    counter = _mm512_mask_add_pd(counter, cmp, counter, adder);
                    if (cmp == 0) {
                        break;
                    }
                }
            }
            else {
                for (int k = 0; k < info.maxIter; k++) {
                    __m512d aa = _mm512_mul_pd(a, a);
                    __m512d ab = _mm512_mul_pd(a, b);
                    __mmask8 cmp = _mm512_cmp_pd_mask(_mm512_fmadd_pd(b, b, aa), threshold, _CMP_LE_OQ);
                    a = _mm512_sub_pd(aa, _mm512_fmsub_pd(b, b, xs));
                    b = _mm512_fmadd_pd(two, ab, ys);
                    counter = _mm512_mask_add_pd(counter, cmp, counter, adder);
                    if (cmp == 0) {
                        break;
                    }
                }
            }

            auto alignVec = [](double* data) -> double* {
                void* aligned = data;
                ::size_t length = 32 * sizeof(double);
                std::align(64, 24 * sizeof(double), aligned, length);
                return static_cast<double*>(aligned);
            };

            double resData[64];
            double* ftRes = alignVec(resData);
            double* resa = ftRes + 8;
            double* resb = ftRes + 16;
            _mm512_store_pd(ftRes, counter);
            if (info.smooth) {
                _mm512_store_pd(resa, resultsa);
                _mm512_store_pd(resb, resultsb);
            }
            for (int k = 0; k < 8 && i + k < info.bWidth; k++) {
                if (info.smooth) {
                    data[i + k + j * info.bWidth] = ftRes[k] <= 0 ? info.maxIter :
                        ftRes[k] >= info.maxIter ? info.maxIter :
                        ((float)ftRes[k]) + 1 - ::log(::log((float) (resa[k] * resa[k] + resb[k] * resb[k])) / 2) / ::log(2.0f);
                }
                else {
                    data[i + k + j * info.bWidth] = ftRes[k] <= 0 ? info.maxIter : ftRes[k];
                }
            }
        }
    }
}


