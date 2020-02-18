#include "CpuGenerators.h"

#include <immintrin.h>
#include <omp.h>
#include <cmath>

#include <utility>
#include <memory>

using mnd::CpuGenerator;

namespace mnd
{
    template class CpuGenerator<float, mnd::X86_AVX, false>;
    template class CpuGenerator<float, mnd::X86_AVX, true>;

    template class CpuGenerator<double, mnd::X86_AVX, false>;
    template class CpuGenerator<double, mnd::X86_AVX, true>;

    template class CpuGenerator<DoubleDouble, mnd::X86_AVX_FMA, false>;
    template class CpuGenerator<DoubleDouble, mnd::X86_AVX_FMA, true>;
}

template<bool parallel>
void CpuGenerator<float, mnd::X86_AVX, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = float;
    const MandelViewport& view = info.view;

    if constexpr(parallel)
    omp_set_num_threads(2 * omp_get_num_procs());
#pragma omp parallel for schedule(static, 1) if (parallel)
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y) + T(j) * T(view.height / info.bHeight);
        __m256 ys = {y, y, y, y, y, y, y, y};
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
            __m256 resultsa = {0, 0, 0, 0, 0, 0, 0, 0};
            __m256 resultsb = {0, 0, 0, 0, 0, 0, 0, 0};

            __m256 threshold = {16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f};

            __m256 a = xs;
            __m256 b = ys;

            for (int k = 0; k < info.maxIter; k++) {
                __m256 aa = _mm256_mul_ps(a, a);
                __m256 bb = _mm256_mul_ps(b, b);
                __m256 abab = _mm256_mul_ps(a, b); abab = _mm256_add_ps(abab, abab);
                a = _mm256_add_ps(_mm256_sub_ps(aa, bb), xs);
                b = _mm256_add_ps(abab, ys);
                __m256 cmp = _mm256_cmp_ps(_mm256_add_ps(aa, bb), threshold, _CMP_LE_OQ);
                if (info.smooth) {
                    resultsa = _mm256_or_ps(_mm256_andnot_ps(cmp, resultsa), _mm256_and_ps(cmp, a));
                    resultsb = _mm256_or_ps(_mm256_andnot_ps(cmp, resultsb), _mm256_and_ps(cmp, b));
                }
                adder = _mm256_and_ps(adder, cmp);
                counter = _mm256_add_ps(counter, adder);
                if ((k & 0x7) == 0 && _mm256_testz_ps(cmp, cmp) != 0) {
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


template<bool parallel>
void CpuGenerator<double, mnd::X86_AVX, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = double;
    const MandelViewport& view = info.view;

    if constexpr(parallel)
        omp_set_num_threads(2 * omp_get_num_procs());
#pragma omp parallel for schedule(static, 1) if (parallel)
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y + T(j) * view.height / info.bHeight);
        __m256d ys = { y, y, y, y };
        long i = 0;
        for (i; i < info.bWidth; i += 4) {
            __m256d xs = {
                double(view.x + double(i) * view.width / info.bWidth),
                double(view.x + double(i + 1) * view.width / info.bWidth),
                double(view.x + double(i + 2) * view.width / info.bWidth),
                double(view.x + double(i + 3) * view.width / info.bWidth)
            };

            int itRes[4] = { 0, 0, 0, 0 };

            __m256d threshold = { 16.0, 16.0, 16.0, 16.0 };
            __m256d counter = { 0, 0, 0, 0 };
            __m256d adder = { 1, 1, 1, 1 };

            __m256d a = xs;
            __m256d b = ys;

            for (int k = 0; k < info.maxIter; k++) {
                __m256d aa = _mm256_mul_pd(a, a);
                __m256d bb = _mm256_mul_pd(b, b);
                __m256d abab = _mm256_mul_pd(a, b); abab = _mm256_add_pd(abab, abab);
                a = _mm256_add_pd(_mm256_sub_pd(aa, bb), xs);
                b = _mm256_add_pd(abab, ys);
                __m256i cmp = _mm256_castpd_si256(_mm256_cmp_pd(_mm256_add_pd(aa, bb), threshold, _CMP_LE_OQ));
                /*if (info.smooth) {
                    resultsa = _mm256_or_pd(_mm256_andnot_ps(cmp, resultsa), _mm256_and_ps(cmp, a));
                    resultsb = _mm256_or_ps(_mm256_andnot_ps(cmp, resultsb), _mm256_and_ps(cmp, b));
                }*/
                adder = _mm256_and_pd(adder, _mm256_castsi256_pd(cmp));
                counter = _mm256_add_pd(counter, adder);
                if ((k & 0x7) == 0 && _mm256_testz_si256(cmp, cmp) != 0) {
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

static inline std::pair<__m256d, __m256d> quickTwoSum(__m256d a, __m256d b)
{
    __m256d s = _mm256_add_pd(a, b);
    __m256d e = _mm256_sub_pd(b, _mm256_sub_pd(s, a));
    return { s, e };
}

static inline std::pair<__m256d, __m256d> quickTwoDiff(__m256d a, __m256d b)
{
    __m256d s = _mm256_sub_pd(a, b);
    __m256d e = _mm256_sub_pd(_mm256_sub_pd(a, s), b);
    return { s, e };
}

static inline std::pair<__m256d, __m256d> twoSum(__m256d a, __m256d b)
{
    __m256d s = _mm256_add_pd(a, b);
    __m256d bb = _mm256_sub_pd(s, a);
    __m256d e = _mm256_add_pd(_mm256_sub_pd(a, _mm256_sub_pd(s, bb)), _mm256_sub_pd(b, bb));
    return { s, e };
}

static inline std::pair<__m256d, __m256d> twoDiff(__m256d a, __m256d b)
{
    __m256d s = _mm256_sub_pd(a, b);
    __m256d bb = _mm256_sub_pd(s, a);
    __m256d e = _mm256_sub_pd(_mm256_sub_pd(a, _mm256_sub_pd(s, bb)), _mm256_add_pd(b, bb));
    return { s, e };
}

/*
static inline std::pair<__m256d, __m256d> split(__m256d a)
{
    static const __m256d SPLIT_THRESH = { 6.69692879491417e+299, 6.69692879491417e+299, 6.69692879491417e+299, 6.69692879491417e+299 };
    static const __m256d MINUS_SPLIT_THRESH = { -6.69692879491417e+299, -6.69692879491417e+299, -6.69692879491417e+299, -6.69692879491417e+299 };
    static const __m256d SPLITTER = { 134217729.0, 134217729.0, 134217729.0, 134217729.0};
    __m256d temp;
    __m256i cmp1 = _mm256_castpd_si256(_mm256_cmp_pd(a, SPLIT_THRESH, _CMP_GT_OQ));
    __m256i cmp2 = _mm256_castpd_si256(_mm256_cmp_pd(a, MINUS_SPLIT_THRESH, _CMP_LT_OQ));
    __m256i cmp = _mm256_or_si256
}*/

static inline std::pair<__m256d, __m256d> twoProd(__m256d a, __m256d b)
{
//#ifdef CPUID_FMA
    __m256d p = _mm256_mul_pd(a, b);
    __m256d e = _mm256_fmadd_pd(a, b, p);
    return { p, e };
//#else
/*    double a_hi, a_lo, b_hi, b_lo;
    __m256d p = _mm256_mul_ps(a, b);
    split(a, a_hi, a_lo);
    split(b, b_hi, b_lo);
    err = ((a_hi * b_hi - p) + a_hi * b_lo + a_lo * b_hi) + a_lo * b_lo;
    return p;*/
//#endif
}

struct AvxDoubleDouble
{
    __m256d x[2];

    inline AvxDoubleDouble(__m256d a, __m256d b) :
        x{ a, b }
    {}


    inline AvxDoubleDouble operator + (const AvxDoubleDouble& sm) const
    {
        auto[s, e] = twoSum(x[0], sm.x[0]);
        e = _mm256_add_pd(e, _mm256_add_pd(x[1], sm.x[1]));
        auto[r1, r2] = quickTwoSum(s, e);
        return AvxDoubleDouble{ r1, r2 };
    }

    inline AvxDoubleDouble operator - (const AvxDoubleDouble& sm) const
    {
        auto[s, e] = twoDiff(x[0], sm.x[0]);
        e = _mm256_add_pd(e, x[1]);
        e = _mm256_sub_pd(e, sm.x[1]);
        auto[r1, r2] = quickTwoSum(s, e);
        return AvxDoubleDouble{ r1, r2 };
    }

    inline AvxDoubleDouble operator * (const AvxDoubleDouble& sm) const
    {
        auto[p1, p2] = twoProd(this->x[0], sm.x[0]);
        p2 = _mm256_add_pd(p2,
            _mm256_add_pd(_mm256_mul_pd(x[0], sm.x[1]), _mm256_mul_pd(x[1], sm.x[0])) );
        auto[r1, r2] = quickTwoSum(p1, p2);
        return AvxDoubleDouble{ r1, r2 };
    }
};

template<bool parallel>
void CpuGenerator<mnd::DoubleDouble, mnd::X86_AVX_FMA, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    const MandelViewport& view = info.view;

    using T = DoubleDouble;

    T viewx = mnd::convert<T>(view.x);
    T viewy = mnd::convert<T>(view.y);
    T wpp = mnd::convert<T>(view.width / info.bWidth);
    T hpp = mnd::convert<T>(view.height / info.bHeight);

    if constexpr(parallel)
        omp_set_num_threads(2 * omp_get_num_procs());
#pragma omp parallel for schedule(static, 1) if (parallel)
    for (long j = 0; j < info.bHeight; j++) {
        T y = viewy + T(double(j)) * hpp;
        __m256d y0s = { y.x[0], y.x[0], y.x[0], y.x[0] };
        __m256d y1s = { y.x[1], y.x[1], y.x[1], y.x[1] };
        AvxDoubleDouble ys{ y0s, y1s };
        long i = 0;
        for (i; i < info.bWidth; i += 4) {
            T x1 = viewx + T(double(i)) * wpp;
            T x2 = viewx + T(double(i + 1)) * wpp;
            T x3 = viewx + T(double(i + 2)) * wpp;
            T x4 = viewx + T(double(i + 3)) * wpp;

            __m256d x0s = {
                x1.x[0], x2.x[0], x3.x[0], x4.x[0],
            };

            __m256d x1s = {
                x1.x[1], x2.x[1], x3.x[1], x4.x[1],
            };

            AvxDoubleDouble xs{ x0s, x1s };

            int itRes[4] = { 0, 0, 0, 0 };

            __m256d threshold = { 16.0, 16.0, 16.0, 16.0 };
            __m256d counter = { 0, 0, 0, 0 };
            __m256d adder = { 1, 1, 1, 1 };

            AvxDoubleDouble a = xs;
            AvxDoubleDouble b = ys;

            for (int k = 0; k < info.maxIter; k++) {
                AvxDoubleDouble aa = a * a;
                AvxDoubleDouble bb = b * b;
                AvxDoubleDouble abab = a * b; abab = abab + abab;
                a = aa - bb + xs;
                b = abab + ys;
                __m256i cmp = _mm256_castpd_si256(_mm256_cmp_pd(_mm256_add_pd(aa.x[0], bb.x[0]), threshold, _CMP_LE_OQ));
                /*if (info.smooth) {
                    resultsa = _mm256_or_pd(_mm256_andnot_ps(cmp, resultsa), _mm256_and_ps(cmp, a));
                    resultsb = _mm256_or_ps(_mm256_andnot_ps(cmp, resultsb), _mm256_and_ps(cmp, b));
                }*/
                adder = _mm256_and_pd(adder, _mm256_castsi256_pd(cmp));
                counter = _mm256_add_pd(counter, adder);
                if ((k & 0x7) == 0 && _mm256_testz_si256(cmp, cmp) != 0) {
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

