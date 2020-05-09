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
    __m512 viewx = _mm512_set1_ps(viewxf);
    __m512 dpp = _mm512_set1_ps(dppf);
    __m512 enumerate = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    __m512 two = _mm512_set1_ps(2);

    T jX = mnd::convert<T>(info.juliaX);
    T jY = mnd::convert<T>(info.juliaY);
    __m512 juliaX = _mm512_set1_ps(jX);
    __m512 juliaY = _mm512_set1_ps(jY);

#if defined(_OPENMP)
    if constexpr(parallel)
        omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y + double(j) * view.height / info.bHeight);
        __m512 ys = _mm512_set1_ps(y);
        for (long i = 0; i < info.bWidth; i += 2 * 16) {
            __m512 pixc0 = _mm512_add_ps(_mm512_set1_ps(float(i)), enumerate);
            __m512 pixc1 = _mm512_add_ps(_mm512_set1_ps(float(i + 16)), enumerate);
            //__m512 pixc2 = _mm512_add_ps(_mm512_set1_ps(float(i + 32)), enumerate);
            __m512 xs0 = _mm512_fmadd_ps(dpp, pixc0, viewx);
            __m512 xs1 = _mm512_fmadd_ps(dpp, pixc1, viewx);
            //__m512 xs2 = _mm512_fmadd_ps(dpp, pixc2, viewx);

            __m512 counter0 = _mm512_setzero_ps();
            __m512 counter1 = _mm512_setzero_ps();
            //__m512 counter2 = _mm512_setzero_ps();
            __m512 adder0 = _mm512_set1_ps(1);
            __m512 adder1 = _mm512_set1_ps(1);
            //__m512 adder2 = _mm512_set1_ps(1);
            __m512 resultsa0 = _mm512_setzero_ps();
            __m512 resultsa1 = _mm512_setzero_ps();
            //__m512 resultsa2 = _mm512_setzero_ps();
            __m512 resultsb0 = _mm512_setzero_ps();
            __m512 resultsb1 = _mm512_setzero_ps();
            //__m512 resultsb2 = _mm512_setzero_ps();

            __m512 threshold = _mm512_set1_ps(16);

            __m512 cx0 = xs0;
            __m512 cx1 = xs1;
            __m512 cy = ys;
	    if (info.julia) {
		cx0 = juliaX;
		cx1 = juliaX;
		cy = juliaY;
	    }

            __m512 a0 = xs0;
            __m512 a1 = xs1;
            //__m512 a2 = xs2;
            __m512 b0 = ys;
            __m512 b1 = ys;
            //__m512 b2 = ys;

            if (info.smooth) {
                __mmask16 cmp0 = 0xFFFF;
                __mmask16 cmp1 = 0xFFFF;
                for (int k = 0; k < info.maxIter; k++) {
                    __m512 aa0 = _mm512_mul_ps(a0, a0);
                    __m512 aa1 = _mm512_mul_ps(a1, a1);
                    //__m512 aa2 = _mm512_mul_ps(a2, a2);
                    __m512 abab0 = _mm512_mul_ps(a0, b0);
                    __m512 abab1 = _mm512_mul_ps(a1, b1);
                    //__m512 abab2 = _mm512_mul_ps(a2, b2);

                    a0 = _mm512_sub_ps(aa0, _mm512_fmsub_ps(b0, b0, cx0));
                    a1 = _mm512_sub_ps(aa1, _mm512_fmsub_ps(b1, b1, cx1));
                    //a2 = _mm512_sub_ps(aa2, _mm512_fmsub_ps(b2, b2, xs2));
                    b0 = _mm512_fmadd_ps(two, abab0, cy);
                    b1 = _mm512_fmadd_ps(two, abab1, cy);
                    //b2 = _mm512_fmadd_ps(two, abab2, ys);

                    resultsa0 = _mm512_mask_blend_ps(cmp0, resultsa0, a0);
                    resultsa1 = _mm512_mask_blend_ps(cmp1, resultsa1, a1);
                    //resultsa2 = _mm512_mask_blend_ps(cmp2, resultsa2, a2);
                    resultsb0 = _mm512_mask_blend_ps(cmp0, resultsb0, b0);
                    resultsb1 = _mm512_mask_blend_ps(cmp1, resultsb1, b1);
                    //resultsb2 = _mm512_mask_blend_ps(cmp2, resultsb2, b2);

                    cmp0 = _mm512_cmp_ps_mask(_mm512_fmadd_ps(b0, b0, aa0), threshold, _CMP_LE_OQ);
                    cmp1 = _mm512_cmp_ps_mask(_mm512_fmadd_ps(b1, b1, aa1), threshold, _CMP_LE_OQ);
                    //__mmask16 cmp2 = _mm512_cmp_ps_mask(_mm512_fmadd_ps(b2, b2, aa2), threshold, _CMP_LE_OQ);

                    counter0 = _mm512_mask_add_ps(counter0, cmp0, counter0, adder0);
                    counter1 = _mm512_mask_add_ps(counter1, cmp1, counter1, adder1);
                    //counter2 = _mm512_mask_add_ps(counter2, cmp2, counter2, adder2);
                    if (cmp0 == 0 && cmp1 == 0 /*&& cmp2 == 0*/) {
                        break;
                    }
                }
            }
            else {
                for (int k = 0; k < info.maxIter; k++) {
                    __m512 aa0 = _mm512_mul_ps(a0, a0);
                    __m512 aa1 = _mm512_mul_ps(a1, a1);
                    //__m512 aa2 = _mm512_mul_ps(a2, a2);
                    __m512 abab0 = _mm512_mul_ps(a0, b0);
                    __m512 abab1 = _mm512_mul_ps(a1, b1);
                    //__m512 abab2 = _mm512_mul_ps(a2, b2);
                    __mmask16 cmp0 = _mm512_cmp_ps_mask(_mm512_fmadd_ps(b0, b0, aa0), threshold, _CMP_LE_OQ);
                    __mmask16 cmp1 = _mm512_cmp_ps_mask(_mm512_fmadd_ps(b1, b1, aa1), threshold, _CMP_LE_OQ);
                    //__mmask16 cmp2 = _mm512_cmp_ps_mask(_mm512_fmadd_ps(b2, b2, aa2), threshold, _CMP_LE_OQ);
                    a0 = _mm512_sub_ps(aa0, _mm512_fmsub_ps(b0, b0, cx0));
                    a1 = _mm512_sub_ps(aa1, _mm512_fmsub_ps(b1, b1, cx1));
                    //a2 = _mm512_sub_ps(aa2, _mm512_fmsub_ps(b2, b2, xs2));
                    b0 = _mm512_fmadd_ps(two, abab0, cy);
                    b1 = _mm512_fmadd_ps(two, abab1, cy);
                    //b2 = _mm512_fmadd_ps(two, abab2, ys);
                    counter0 = _mm512_mask_add_ps(counter0, cmp0, counter0, adder0);
                    counter1 = _mm512_mask_add_ps(counter1, cmp1, counter1, adder1);
                    //counter2 = _mm512_mask_add_ps(counter2, cmp2, counter2, adder2);
                    if (cmp0 == 0 && cmp1 == 0 /*&& cmp2 == 0*/) {
                        break;
                    }
                }
            }

            auto alignVec = [](float* data) -> float* {
                void* aligned = data;
                ::size_t length = 3 * 64 * sizeof(float);
                std::align(64, 48 * sizeof(float), aligned, length);
                return static_cast<float*>(aligned);
            };

            float resData[3 * 64];
            float* ftRes = alignVec(resData);
            float* resa = ftRes + 3 * 16;
            float* resb = ftRes + 6 * 16;
            _mm512_store_ps(ftRes, counter0);
            _mm512_store_ps(ftRes + 16, counter1);
            //_mm512_store_ps(ftRes + 32, counter2);
            if (info.smooth) {
                _mm512_store_ps(resa, resultsa0);
                _mm512_store_ps(resa + 16, resultsa1);
                //_mm512_store_ps(resa + 32, resultsa2);
                _mm512_store_ps(resb, resultsb0);
                _mm512_store_ps(resb + 16, resultsb1);
                //_mm512_store_ps(resb + 32, resultsb2);
            }
            for (int k = 0; k < 2 * 16 && i + k < info.bWidth; k++) {
                if (info.smooth) {
                    data[i + k + j * info.bWidth] = ftRes[k] < 0 ? info.maxIter :
                        ftRes[k] >= info.maxIter ? info.maxIter :
                        ((float)ftRes[k]) + 1 - ::log(::log(resa[k] * resa[k] + resb[k] * resb[k]) / 2) / ::log(2.0f);
                }
                else {
                    data[i + k + j * info.bWidth] = ftRes[k] < 0 ? info.maxIter : ftRes[k];
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

    T jX = mnd::convert<T>(info.juliaX);
    T jY = mnd::convert<T>(info.juliaY);
    __m512d juliaX = _mm512_set1_pd(jX);
    __m512d juliaY = _mm512_set1_pd(jY);

#if defined(_OPENMP)
    if constexpr(parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y + double(j) * view.height / info.bHeight);
        __m512d ys = { y, y, y, y, y, y, y, y };
        for (long i = 0; i < info.bWidth; i += 8) {
            __m512d pixc = { double(i), double(i + 1), double(i + 2), double(i + 3), double(i + 4), double(i + 5), double(i + 6), double(i + 7) };
            __m512d xs = _mm512_fmadd_pd(dpp, pixc, viewx);

            __m512d counter = { 0, 0, 0, 0, 0, 0, 0, 0 };
            __m512d adder = { 1, 1, 1, 1, 1, 1, 1, 1 };
            __m512d two = { 2, 2, 2, 2, 2, 2, 2, 2 };
            __m512d resultsa = { 0, 0, 0, 0, 0, 0, 0, 0 };
            __m512d resultsb = { 0, 0, 0, 0, 0, 0, 0, 0 };

            __m512d threshold = { 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f };

            __m512d cx = xs;
            __m512d cy = ys;
	    if (info.julia) {
		cx = juliaX;
		cy = juliaY;
	    }
            __m512d a = xs;
            __m512d b = ys;

            if (info.smooth) {
                __mmask8 cmp = 0xFF;
                for (int k = 0; k < info.maxIter; k++) {
                    __m512d aa = _mm512_mul_pd(a, a);
                    __m512d ab = _mm512_mul_pd(a, b);
                    a = _mm512_sub_pd(aa, _mm512_fmsub_pd(b, b, cx));
                    b = _mm512_fmadd_pd(two, ab, cy);
                    resultsa = _mm512_mask_blend_pd(cmp, resultsa, a);
                    resultsb = _mm512_mask_blend_pd(cmp, resultsb, b);
                    cmp = _mm512_cmp_pd_mask(_mm512_fmadd_pd(b, b, aa), threshold, _CMP_LE_OQ);
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
                    a = _mm512_sub_pd(aa, _mm512_fmsub_pd(b, b, cx));
                    b = _mm512_fmadd_pd(two, ab, cy);
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
                    data[i + k + j * info.bWidth] = ftRes[k] < 0 ? info.maxIter :
                        ftRes[k] >= info.maxIter ? info.maxIter :
                        ((float)ftRes[k]) + 1 - ::log(::log((float) (resa[k] * resa[k] + resb[k] * resb[k])) / 2) / ::log(2.0f);
                }
                else {
                    data[i + k + j * info.bWidth] = ftRes[k] < 0 ? info.maxIter : ftRes[k];
                }
            }
        }
    }
}


