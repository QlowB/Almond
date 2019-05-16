#include "CpuGenerators.h"

#include <immintrin.h>
#include <omp.h>

#include <memory>

using mnd::CpuGeneratorAvxFloat;
using mnd::CpuGeneratorAvxDouble;


void CpuGeneratorAvxFloat::generate(const mnd::MandelInfo& info, float* data)
{
    using T = float;
    const MandelViewport& view = info.view;
    omp_set_num_threads(2 * omp_get_num_procs());
#pragma omp parallel for
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y) + T(j) * T(view.height / info.bHeight);
        long i = 0;
        for (i; i < info.bWidth; i += 8) {
            __m256 xs = {
                float(view.x + double(i) * view.width / info.bWidth),
                float(view.x + double(i + 1) * view.width / info.bWidth),
                float(view.x + double(i + 2) * view.width / info.bWidth),
                float(view.x + double(i + 3) * view.width / info.bWidth),
                float(view.x + double(i + 4) * view.width / info.bWidth),
                float(view.x + double(i + 5) * view.width / info.bWidth),
                float(view.x + double(i + 6) * view.width / info.bWidth),
                float(view.x + double(i + 7) * view.width / info.bWidth)
            };

            __m256 counter = {0, 0, 0, 0, 0, 0, 0, 0};
            __m256 adder = {1, 1, 1, 1, 1, 1, 1, 1};

            __m256 threshold = {16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f};

            __m256 ys = {y, y, y, y, y, y, y, y};
            __m256 a = xs;
            __m256 b = ys;

            for (int k = 0; k < info.maxIter; k++) {
                __m256 aa = _mm256_mul_ps(a, a);
                __m256 bb = _mm256_mul_ps(b, b);
                __m256 abab = _mm256_mul_ps(a, b); abab = _mm256_add_ps(abab, abab);
                a = _mm256_add_ps(_mm256_sub_ps(aa, bb), xs);
                b = _mm256_add_ps(abab, ys);
                __m256i cmp = _mm256_castps_si256(_mm256_cmp_ps(_mm256_add_ps(aa, bb), threshold, _CMP_LE_OQ));
                adder = _mm256_and_ps(adder, _mm256_castsi256_ps(cmp));
                counter = _mm256_add_ps(counter, adder);
                if (_mm256_testz_si256(cmp, cmp) != 0) {
                    break;
                }
            }

            auto alignVec = [](float* data) -> float* {
                void* aligned = data;
                ::size_t length = 64;
                std::align(32, 8 * sizeof(float), aligned, length);
                return static_cast<float*>(aligned);
            };

            float resData[16];
            float* ftRes = alignVec(resData);

            _mm256_store_ps(ftRes, counter);
            for (int k = 0; k < 8 && i + k < info.bWidth; k++)
                data[i + k + j * info.bWidth] = ftRes[k] > 0 ? ftRes[k] : info.maxIter;
        }
    }
}


void CpuGeneratorAvxDouble::generate(const mnd::MandelInfo& info, float* data)
{
    using T = double;
    const MandelViewport& view = info.view;

    omp_set_num_threads(2 * omp_get_num_procs());
#pragma omp parallel for
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y) + T(j) * view.height / info.bHeight;
        long i = 0;
        for (i; i < info.bWidth; i += 4) {
            __m256d xs = {
                double(view.x) + double(i) * view.width / info.bWidth,
                double(view.x) + double(i + 1) * view.width / info.bWidth,
                double(view.x) + double(i + 2) * view.width / info.bWidth,
                double(view.x) + double(i + 3) * view.width / info.bWidth
            };

            int itRes[4] = { 0, 0, 0, 0 };

            __m256d threshold = { 16.0, 16.0, 16.0, 16.0 };
            __m256d counter = { 0, 0, 0, 0 };
            __m256d adder = { 1, 1, 1, 1 };

            __m256d ys = { y, y, y, y };
            __m256d a = xs;
            __m256d b = ys;

            for (int k = 0; k < info.maxIter; k++) {
                __m256d aa = _mm256_mul_pd(a, a);
                __m256d bb = _mm256_mul_pd(b, b);
                __m256d abab = _mm256_mul_pd(a, b); abab = _mm256_add_pd(abab, abab);
                a = _mm256_add_pd(_mm256_sub_pd(aa, bb), xs);
                b = _mm256_add_pd(abab, ys);
                __m256i cmp = _mm256_castpd_si256(_mm256_cmp_pd(_mm256_add_pd(aa, bb), threshold, _CMP_LE_OQ));
                adder = _mm256_and_pd(adder, _mm256_castsi256_pd(cmp));
                counter = _mm256_add_pd(counter, adder);
                if (_mm256_testz_si256(cmp, cmp) != 0) {
                    break;
                }
            }

            auto alignVec = [](double* data) -> double* {
                void* aligned = data;
                ::size_t length = 64;
                std::align(32, 4 * sizeof(double), aligned, length);
                return static_cast<double*>(aligned);
            };

            double resData[8];
            double* ftRes = alignVec(resData);
            _mm256_store_pd(ftRes, counter);
            for (int k = 0; k < 4 && i + k < info.bWidth; k++)
                data[i + k + j * info.bWidth] = ftRes[k] > 0 ? float(ftRes[k]) : info.maxIter;
        }
    }
}

