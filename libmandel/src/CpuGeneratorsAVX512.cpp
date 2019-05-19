#include "CpuGenerators.h"

#include <immintrin.h>
#include <omp.h>

#include <memory>

using mnd::CpuGeneratorAvx512Float;
using mnd::CpuGeneratorAvx512Double;

void CpuGeneratorAvx512Float::generate(const mnd::MandelInfo& info, float* data)
{
    using T = float;
    const MandelViewport& view = info.view;
    omp_set_num_threads(2 * omp_get_num_procs());
#pragma omp parallel for
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y) + T(j) * T(view.height / info.bHeight);
        long i = 0;
        for (i; i < info.bWidth; i += 16) {
            __m512 xs = {
                float(view.x + double(i) * view.width / info.bWidth),
                float(view.x + double(i + 1) * view.width / info.bWidth),
                float(view.x + double(i + 2) * view.width / info.bWidth),
                float(view.x + double(i + 3) * view.width / info.bWidth),
                float(view.x + double(i + 4) * view.width / info.bWidth),
                float(view.x + double(i + 5) * view.width / info.bWidth),
                float(view.x + double(i + 6) * view.width / info.bWidth),
                float(view.x + double(i + 7) * view.width / info.bWidth),
                float(view.x + double(i + 8) * view.width / info.bWidth),
                float(view.x + double(i + 9) * view.width / info.bWidth),
                float(view.x + double(i + 10) * view.width / info.bWidth),
                float(view.x + double(i + 11) * view.width / info.bWidth),
                float(view.x + double(i + 12) * view.width / info.bWidth),
                float(view.x + double(i + 13) * view.width / info.bWidth),
                float(view.x + double(i + 14) * view.width / info.bWidth),
                float(view.x + double(i + 15) * view.width / info.bWidth)
            };

            __m512 counter = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            __m512 adder = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

            __m512 threshold = {16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f};

            __m512 ys = {y, y, y, y, y, y, y, y, y, y, y, y, y, y, y};
            __m512 a = xs;
            __m512 b = ys;

            for (int k = 0; k < info.maxIter; k++) {
                __m512 aa = _mm512_mul_ps(a, a);
                __m512 bb = _mm512_mul_ps(b, b);
                __m512 abab = _mm512_mul_ps(a, b); abab = _mm512_add_ps(abab, abab);
                a = _mm512_add_ps(_mm512_sub_ps(aa, bb), xs);
                b = _mm512_add_ps(abab, ys);
                __mmask16 cmp = _mm512_cmp_ps_mask(_mm512_add_ps(aa, bb), threshold, _CMP_LE_OQ);
                counter = _mm512_mask_add_ps(counter, cmp, counter, adder);
                if (cmp == 0) {
                    break;
                }
            }

            auto alignVec = [](float* data) -> float* {
                void* aligned = data;
                ::size_t length = 64;
                std::align(32, 16 * sizeof(float), aligned, length);
                return static_cast<float*>(aligned);
            };

            float resData[32];
            float* ftRes = alignVec(resData);

            _mm512_store_ps(ftRes, counter);
            for (int k = 0; k < 8 && i + k < info.bWidth; k++)
                data[i + k + j * info.bWidth] = ftRes[k] > 0 ? ftRes[k] : info.maxIter;
        }
    }
}


void CpuGeneratorAvx512Double::generate(const mnd::MandelInfo& info, float* data)
{
}