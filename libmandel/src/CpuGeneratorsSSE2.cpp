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

    const float dppf = float(view.width / info.bWidth);
    const float viewxf = float(view.x);
    __m128 viewx = { viewxf, viewxf, viewxf, viewxf };
    __m128 dpp = { dppf, dppf, dppf, dppf };

    T jX = mnd::convert<T>(info.juliaX);
    T jY = mnd::convert<T>(info.juliaY);
    __m128 juliaX = { jX, jX, jX, jX };
    __m128 juliaY = { jY, jY, jY, jY };

#if defined(_OPENMP)
    if constexpr(parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y) + T(j) * T(view.height / info.bHeight);
        __m128 ys = {y, y, y, y};
        for (long i = 0; i < info.bWidth; i += 8) {
            __m128 pixc = { float(i), float(i + 1), float(i + 2), float(i + 3) };
            __m128 pixc2 = { float(i + 4), float(i + 5), float(i + 6), float(i + 7) };
            __m128 xs = _mm_add_ps(_mm_mul_ps(dpp, pixc), viewx);
            __m128 xs2 = _mm_add_ps(_mm_mul_ps(dpp, pixc2), viewx);

            __m128 counter = { 0, 0, 0, 0 };
            __m128 adder = { 1, 1, 1, 1 };
            __m128 counter2 = { 0, 0, 0, 0 };
            __m128 adder2 = { 1, 1, 1, 1 };

            __m128 threshold = { 16.0f, 16.0f, 16.0f, 16.0f };

            __m128 a = xs;
            __m128 b = ys;
            __m128 a2 = xs2;
            __m128 b2 = ys;

            __m128 cx = xs;
            __m128 cy = ys;
            __m128 cx2 = xs2;
	    if (info.julia) {
		cx = juliaX;
		cx2 = juliaX;
		cy = juliaY;
	    }

            __m128 resulta = { 0, 0, 0, 0 };
            __m128 resultb = { 0, 0, 0, 0 };
            __m128 resulta2 = { 0, 0, 0, 0 };
            __m128 resultb2 = { 0, 0, 0, 0 };

            __m128 cmp = _mm_cmple_ps(threshold, threshold);
            __m128 cmp2 = _mm_cmple_ps(threshold, threshold);
            for (int k = 0; k < info.maxIter; k++) {
                __m128 aa = _mm_mul_ps(a, a);
                __m128 aa2 = _mm_mul_ps(a2, a2);
                __m128 bb = _mm_mul_ps(b, b);
                __m128 bb2 = _mm_mul_ps(b2, b2);
                __m128 abab = _mm_mul_ps(a, b); abab = _mm_add_ps(abab, abab);
                __m128 abab2 = _mm_mul_ps(a2, b2); abab2 = _mm_add_ps(abab2, abab2);
                a = _mm_add_ps(_mm_sub_ps(aa, bb), cx);
                b = _mm_add_ps(abab, cy);
                a2 = _mm_add_ps(_mm_sub_ps(aa2, bb2), cx2);
                b2 = _mm_add_ps(abab2, cy);
                if (info.smooth) {
                    resulta = _mm_or_ps(_mm_andnot_ps(cmp, resulta), _mm_and_ps(cmp, a));
                    resultb = _mm_or_ps(_mm_andnot_ps(cmp, resultb), _mm_and_ps(cmp, b));
                    resulta2 = _mm_or_ps(_mm_andnot_ps(cmp2, resulta2), _mm_and_ps(cmp2, a2));
                    resultb2 = _mm_or_ps(_mm_andnot_ps(cmp2, resultb2), _mm_and_ps(cmp2, b2));
                }
                cmp = _mm_cmple_ps(_mm_add_ps(aa, bb), threshold);
                cmp2 = _mm_cmple_ps(_mm_add_ps(aa2, bb2), threshold);
                adder = _mm_and_ps(adder, cmp);
                counter = _mm_add_ps(counter, adder);
                adder2 = _mm_and_ps(adder2, cmp2);
                counter2 = _mm_add_ps(counter2, adder2);
                if ((k & 0x7) == 0 && _mm_movemask_epi8(_mm_castps_si128(cmp)) == 0 &&
                    _mm_movemask_epi8(_mm_castps_si128(cmp2)) == 0) {
                    break;
                }
            }

            auto alignVec = [](float* data) -> float* {
                void* aligned = data;
                ::size_t length = 64;
                std::align(32, 8 * sizeof(float), aligned, length);
                return static_cast<float*>(aligned);
            };

            float resData[64];
            float* ftRes = alignVec(resData);
            float* resa = ftRes + 8;
            float* resb = ftRes + 16;

            _mm_store_ps(ftRes, counter);
            _mm_store_ps(ftRes + 4, counter2);
            _mm_store_ps(resa, resulta);
            _mm_store_ps(resa + 4, resulta2);
            _mm_store_ps(resb, resultb);
            _mm_store_ps(resb + 4, resultb2);
            for (int k = 0; k < 8 && i + k < info.bWidth; k++) {
                if (info.smooth)
                    data[i + k + j * info.bWidth] = ftRes[k] < 0 ? info.maxIter :
                    ftRes[k] >= info.maxIter ? info.maxIter :
                    ((float)ftRes[k]) + 1 - ::logf(::logf(resa[k] * resa[k] + resb[k] * resb[k]) / 2) / ::logf(2.0f);
                else
                    data[i + k + j * info.bWidth] = ftRes[k] >= 0 ? float(ftRes[k]) : info.maxIter;
            }
        }
    }
}


template<bool parallel>
void CpuGenerator<double, mnd::X86_SSE2, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = double;
    const MandelViewport& view = info.view;

    const double dppf = double(view.width / info.bWidth);
    const double viewxf = double(view.x);
    __m128d viewx = { viewxf, viewxf };
    __m128d dpp = { dppf, dppf };

    T jX = mnd::convert<T>(info.juliaX);
    T jY = mnd::convert<T>(info.juliaY);
    __m128d juliaX = { jX, jX };
    __m128d juliaY = { jY, jY };

#if defined(_OPENMP)
    if constexpr(parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y) + T(j) * T(view.height / info.bHeight);
        __m128d ys = { y, y };
        for (long i = 0; i < info.bWidth; i += 4) {
            __m128d pixc = { double(i), double(i + 1) };
            __m128d pixc2 = { double(i + 2), double(i + 3) };
            __m128d xs = _mm_add_pd(_mm_mul_pd(dpp, pixc), viewx);
            __m128d xs2 = _mm_add_pd(_mm_mul_pd(dpp, pixc2), viewx);

            __m128d counter = { 0, 0 };
            __m128d adder = { 1, 1 };
            __m128d counter2 = { 0, 0 };
            __m128d adder2 = { 1, 1 };

            __m128d threshold = { 16.0f, 16.0f };

            __m128d a = xs;
            __m128d b = ys;
            __m128d a2 = xs2;
            __m128d b2 = ys;

            __m128d cx = xs;
            __m128d cy = ys;
            __m128d cx2 = xs2;
	        if (info.julia) {
		        cx = juliaX;
		        cx2 = juliaX;
		        cy = juliaY;
	        }

            __m128d resulta = { 0, 0 };
            __m128d resultb = { 0, 0 };
            __m128d resulta2 = { 0, 0 };
            __m128d resultb2 = { 0, 0 };

            __m128d cmp = _mm_cmple_pd(threshold, threshold);
            __m128d cmp2 = _mm_cmple_pd(threshold, threshold);
            for (int k = 0; k < info.maxIter; k++) {
                __m128d aa = _mm_mul_pd(a, a);
                __m128d aa2 = _mm_mul_pd(a2, a2);
                __m128d bb = _mm_mul_pd(b, b);
                __m128d bb2 = _mm_mul_pd(b2, b2);
                __m128d abab = _mm_mul_pd(a, b); abab = _mm_add_pd(abab, abab);
                __m128d abab2 = _mm_mul_pd(a2, b2); abab2 = _mm_add_pd(abab2, abab2);
                a = _mm_add_pd(_mm_sub_pd(aa, bb), cx);
                b = _mm_add_pd(abab, cy);
                a2 = _mm_add_pd(_mm_sub_pd(aa2, bb2), cx2);
                b2 = _mm_add_pd(abab2, cy);
                if (info.smooth) {
                    resulta = _mm_or_pd(_mm_andnot_pd(cmp, resulta), _mm_and_pd(cmp, a));
                    resultb = _mm_or_pd(_mm_andnot_pd(cmp, resultb), _mm_and_pd(cmp, b));
                    resulta2 = _mm_or_pd(_mm_andnot_pd(cmp2, resulta2), _mm_and_pd(cmp2, a2));
                    resultb2 = _mm_or_pd(_mm_andnot_pd(cmp2, resultb2), _mm_and_pd(cmp2, b2));
                }
                cmp = _mm_cmple_pd(_mm_add_pd(aa, bb), threshold);
                cmp2 = _mm_cmple_pd(_mm_add_pd(aa2, bb2), threshold);
                adder = _mm_and_pd(adder, cmp);
                counter = _mm_add_pd(counter, adder);
                adder2 = _mm_and_pd(adder2, cmp2);
                counter2 = _mm_add_pd(counter2, adder2);
                if (((k & 0x7) == 0) && _mm_movemask_epi8(_mm_castpd_si128(cmp)) == 0 &&
                    _mm_movemask_epi8(_mm_castpd_si128(cmp2)) == 0) {
                    break;
                }
            }

            double ftRes[24];
            double* resa = ftRes + 4;
            double* resb = ftRes + 8;

            _mm_storeu_pd(ftRes, counter);
            _mm_storeu_pd(ftRes + 2, counter2);
            _mm_storeu_pd(resa, resulta);
            _mm_storeu_pd(resa + 2, resulta2);
            _mm_storeu_pd(resb, resultb);
            _mm_storeu_pd(resb + 2, resultb2);
            for (int k = 0; k < 4 && i + k < info.bWidth; k++) {
                if (info.smooth)
                    data[i + k + j * info.bWidth] = ftRes[k] < 0 ? info.maxIter :
                    ftRes[k] >= info.maxIter ? info.maxIter :
                    ((float)ftRes[k]) + 1 - ::logf(::logf(resa[k] * resa[k] + resb[k] * resb[k]) / 2) / ::logf(2.0f);
                else
                    data[i + k + j * info.bWidth] = ftRes[k] >= 0 ? float(ftRes[k]) : info.maxIter;
            }
        }
    }
}
