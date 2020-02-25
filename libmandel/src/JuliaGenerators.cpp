#include "Generators.h"
#include "JuliaGenerators.h"

#include <omp.h>

mnd::JuliaGeneratorFloat::JuliaGeneratorFloat(const mnd::Real& precision) :
    JuliaGenerator{ precision },
    a{ a },
    b{ b }
{
}


mnd::JuliaGeneratorFloat::~JuliaGeneratorFloat(void)
{
}


void mnd::JuliaGeneratorFloat::generate(const MandelInfo& info, float * data)
{
    const MandelViewport& view = info.view;

    using T = double;


    const float dppf = float(view.width / info.bWidth);
    const float viewxf = float(view.x);
    __m256 viewx = { viewxf, viewxf, viewxf, viewxf, viewxf, viewxf, viewxf, viewxf };
    __m256 dpp = { dppf, dppf, dppf, dppf, dppf, dppf, dppf, dppf };


    T juliaa = mnd::convert<T>(info.juliaX);
    T juliab = mnd::convert<T>(info.juliaY);
    __m256 constA = { juliaa, juliaa, juliaa, juliaa, juliaa, juliaa, juliaa,juliaa };
    __m256 constB = { juliab, juliab, juliab, juliab, juliab, juliab, juliab, juliab};

    omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for schedule(static, 1)
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y) + T(j) * T(view.height / info.bHeight);
        __m256 ys = {y, y, y, y, y, y, y, y};
        long i = 0;
        for (i; i < info.bWidth; i += 8) {
            __m256 pixc = { float(i), float(i + 1), float(i + 2), float(i + 3), float(i + 4), float(i + 5), float(i + 6), float(i + 7) };

            __m256 xs = _mm256_add_ps(_mm256_mul_ps(dpp, pixc), viewx);

            __m256 counter = { 0, 0, 0, 0, 0, 0, 0, 0 };
            __m256 adder = { 1, 1, 1, 1, 1, 1, 1, 1 };
            __m256 resultsa = { 0, 0, 0, 0, 0, 0, 0, 0 };
            __m256 resultsb = { 0, 0, 0, 0, 0, 0, 0, 0 };

            __m256 threshold = { 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f };

            __m256 a = xs;
            __m256 b = ys;
            if (info.smooth) {
                for (int k = 0; k < info.maxIter; k++) {
                    __m256 aa = _mm256_mul_ps(a, a);
                    __m256 bb = _mm256_mul_ps(b, b);
                    __m256 abab = _mm256_mul_ps(a, b); abab = _mm256_add_ps(abab, abab);
                    a = _mm256_add_ps(_mm256_sub_ps(aa, bb), constA);
                    b = _mm256_add_ps(abab, constB);
                    __m256 cmp = _mm256_cmp_ps(_mm256_add_ps(aa, bb), threshold, _CMP_LE_OQ);
                    resultsa = _mm256_or_ps(_mm256_andnot_ps(cmp, resultsa), _mm256_and_ps(cmp, a));
                    resultsb = _mm256_or_ps(_mm256_andnot_ps(cmp, resultsb), _mm256_and_ps(cmp, b));
                    adder = _mm256_and_ps(adder, cmp);
                    counter = _mm256_add_ps(counter, adder);
                    if ((k & 0x7) == 0 && _mm256_testz_ps(cmp, cmp) != 0) {
                        break;
                    }
                }
            }
            else {
                for (int k = 0; k < info.maxIter; k++) {
                    __m256 aa = _mm256_mul_ps(a, a);
                    __m256 bb = _mm256_mul_ps(b, b);
                    __m256 abab = _mm256_mul_ps(a, b); abab = _mm256_add_ps(abab, abab);
                    a = _mm256_add_ps(_mm256_sub_ps(aa, bb), constA);
                    b = _mm256_add_ps(abab, constB);
                    __m256 cmp = _mm256_cmp_ps(_mm256_add_ps(aa, bb), threshold, _CMP_LE_OQ);
                    adder = _mm256_and_ps(adder, cmp);
                    counter = _mm256_add_ps(counter, adder);
                    if ((k & 0x7) == 0 && _mm256_testz_ps(cmp, cmp) != 0) {
                        break;
                    }
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
            float* resa = (float*) &resultsa;
            float* resb = (float*) &resultsb;

            _mm256_store_ps(ftRes, counter);
            for (int k = 0; k < 8 && i + k < info.bWidth; k++) {
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

