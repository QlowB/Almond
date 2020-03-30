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
    
    template class CpuGenerator<DoubleDouble, mnd::X86_AVX, false>;
    template class CpuGenerator<DoubleDouble, mnd::X86_AVX, true>;
}

template<bool parallel>
void CpuGenerator<float, mnd::X86_AVX, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = float;
    const MandelViewport& view = info.view;
    const float dppf = float(view.width / info.bWidth);
    const float viewxf = float(view.x);
    __m256 viewx = { viewxf, viewxf, viewxf, viewxf, viewxf, viewxf, viewxf, viewxf };
    __m256 dpp = { dppf, dppf, dppf, dppf, dppf, dppf, dppf, dppf };

    T jX = mnd::convert<T>(info.juliaX);
    T jY = mnd::convert<T>(info.juliaY);
    __m256 juliaX = { jX, jX, jX, jX, jX, jX, jX, jX };
    __m256 juliaY = { jY, jY, jY, jY, jY, jY, jY, jY };

    if constexpr(parallel)
        omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for schedule(static, 1) if (parallel)
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y) + T(j) * T(view.height / info.bHeight);
        __m256 ys = {y, y, y, y, y, y, y, y};
        for (long i = 0; i < info.bWidth; i += 16) {
            __m256 pixc = { float(i), float(i + 1), float(i + 2), float(i + 3), float(i + 4), float(i + 5), float(i + 6), float(i + 7) };
            __m256 pixc2 = { float(i + 8), float(i + 9), float(i + 10), float(i + 11), float(i + 12), float(i + 13), float(i + 14), float(i + 15) };

            __m256 xs = _mm256_add_ps(_mm256_mul_ps(dpp, pixc), viewx);
            __m256 xs2 = _mm256_add_ps(_mm256_mul_ps(dpp, pixc2), viewx);

            __m256 counter = _mm256_setzero_ps();
            __m256 adder = _mm256_set1_ps(1);
            __m256 resultsa = _mm256_setzero_ps();
            __m256 resultsb = _mm256_setzero_ps();

            __m256 counter2 = _mm256_setzero_ps();
            __m256 adder2 = _mm256_set1_ps(1);
            __m256 resultsa2 = _mm256_setzero_ps();
            __m256 resultsb2 = _mm256_setzero_ps();

            __m256 threshold = _mm256_set1_ps(16);

            __m256 a = xs;
            __m256 a2 = xs2;
            __m256 b = ys;
            __m256 b2 = ys;

            __m256 cx = info.julia ? juliaX : xs;
            __m256 cx2 = info.julia ? juliaX : xs2;
            __m256 cy = info.julia ? juliaY : ys;

            if (info.smooth) {
                for (int k = 0; k < info.maxIter; k++) {
                    __m256 aa = _mm256_mul_ps(a, a);
                    __m256 aa2 = _mm256_mul_ps(a2, a2);
                    __m256 bb = _mm256_mul_ps(b, b);
                    __m256 bb2 = _mm256_mul_ps(b2, b2);
                    __m256 abab = _mm256_mul_ps(a, b); abab = _mm256_add_ps(abab, abab);
                    __m256 abab2 = _mm256_mul_ps(a2, b2); abab2 = _mm256_add_ps(abab2, abab2);
                    a = _mm256_add_ps(_mm256_sub_ps(aa, bb), cx);
                    a2 = _mm256_add_ps(_mm256_sub_ps(aa2, bb2), cx2);
                    b = _mm256_add_ps(abab, cy);
                    b2 = _mm256_add_ps(abab2, cy);
                    __m256 cmp = _mm256_cmp_ps(_mm256_add_ps(aa, bb), threshold, _CMP_LE_OQ);
                    __m256 cmp2 = _mm256_cmp_ps(_mm256_add_ps(aa2, bb2), threshold, _CMP_LE_OQ);
                    resultsa = _mm256_or_ps(_mm256_andnot_ps(cmp, resultsa), _mm256_and_ps(cmp, a));
                    resultsb = _mm256_or_ps(_mm256_andnot_ps(cmp, resultsb), _mm256_and_ps(cmp, b));
                    resultsa2 = _mm256_or_ps(_mm256_andnot_ps(cmp2, resultsa2), _mm256_and_ps(cmp2, a2));
                    resultsb2 = _mm256_or_ps(_mm256_andnot_ps(cmp2, resultsb2), _mm256_and_ps(cmp2, b2));
                    adder = _mm256_and_ps(adder, cmp);
                    counter = _mm256_add_ps(counter, adder);
                    adder2 = _mm256_and_ps(adder2, cmp2);
                    counter2 = _mm256_add_ps(counter2, adder2);
                    if ((k & 0x7) == 0 && _mm256_testz_ps(cmp, cmp) != 0 && _mm256_testz_ps(cmp2, cmp2) != 0) {
                        break;
                    }
                }
            }
            else {
                for (int k = 0; k < info.maxIter; k++) {
                    __m256 aa = _mm256_mul_ps(a, a);
                    __m256 aa2 = _mm256_mul_ps(a2, a2);
                    __m256 bb = _mm256_mul_ps(b, b);
                    __m256 bb2 = _mm256_mul_ps(b2, b2);
                    __m256 abab = _mm256_mul_ps(a, b); abab = _mm256_add_ps(abab, abab);
                    __m256 abab2 = _mm256_mul_ps(a2, b2); abab2 = _mm256_add_ps(abab2, abab2);
                    a = _mm256_add_ps(_mm256_sub_ps(aa, bb), cx);
                    a2 = _mm256_add_ps(_mm256_sub_ps(aa2, bb2), cx2);
                    b = _mm256_add_ps(abab, cy);
                    b2 = _mm256_add_ps(abab2, cy);
                    __m256 cmp = _mm256_cmp_ps(_mm256_add_ps(aa, bb), threshold, _CMP_LE_OQ);
                    __m256 cmp2 = _mm256_cmp_ps(_mm256_add_ps(aa2, bb2), threshold, _CMP_LE_OQ);
                    adder = _mm256_and_ps(adder, cmp);
                    counter = _mm256_add_ps(counter, adder);
                    adder2 = _mm256_and_ps(adder2, cmp2);
                    counter2 = _mm256_add_ps(counter2, adder2);
                    if ((k & 0x7) == 0 && _mm256_testz_ps(cmp, cmp) != 0 && _mm256_testz_ps(cmp2, cmp2) != 0) {
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

            float resData[64];
            float* ftRes = alignVec(resData);
            float* resa = ftRes + 16;
            float* resb = resa + 16;

            _mm256_store_ps(ftRes, counter);
            _mm256_store_ps(ftRes + 8, counter2);
            _mm256_store_ps(resa, resultsa);
            _mm256_store_ps(resa + 8, resultsa2);
            _mm256_store_ps(resb, resultsb);
            _mm256_store_ps(resb + 8, resultsb2);
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
void CpuGenerator<double, mnd::X86_AVX, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = double;
    const MandelViewport& view = info.view;

    const double dppf = double(view.width / info.bWidth);
    const double viewxf = double(view.x);
    __m256d viewx = { viewxf, viewxf, viewxf, viewxf };
    __m256d dpp = { dppf, dppf, dppf, dppf };

    T jX = mnd::convert<T>(info.juliaX);
    T jY = mnd::convert<T>(info.juliaY);
    __m256d juliaX = { jX, jX, jX, jX };
    __m256d juliaY = { jY, jY, jY, jY };

    if constexpr(parallel)
        omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for schedule(static, 1) if (parallel)
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y + T(j) * view.height / info.bHeight);
        __m256d ys = { y, y, y, y };
        for (long i = 0; i < info.bWidth; i += 8) {
            __m256d pixc = { double(i), double(i + 1), double(i + 2), double(i + 3) };
            __m256d pixc2 = { double(i + 4), double(i + 5), double(i + 6), double(i + 7) };
            __m256d xs = _mm256_add_pd(_mm256_mul_pd(dpp, pixc), viewx);
            __m256d xs2 = _mm256_add_pd(_mm256_mul_pd(dpp, pixc2), viewx);

            int itRes[4] = { 0, 0, 0, 0 };

            __m256d threshold = { 16.0, 16.0, 16.0, 16.0 };
            __m256d counter = { 0, 0, 0, 0 };
            __m256d adder = { 1, 1, 1, 1 };
            __m256d counter2 = { 0, 0, 0, 0 };
            __m256d adder2 = { 1, 1, 1, 1 };

            __m256d resultsa = { 0, 0, 0, 0 };
            __m256d resultsb = { 0, 0, 0, 0 };
            __m256d resultsa2 = { 0, 0, 0, 0 };
            __m256d resultsb2 = { 0, 0, 0, 0 };

            __m256d a = xs;
            __m256d b = ys;
            __m256d a2 = xs2;
            __m256d b2 = ys;

            __m256d cx = info.julia ? juliaX : xs;
            __m256d cx2 = info.julia ? juliaX : xs2;
            __m256d cy = info.julia ? juliaY : ys;

            if (info.smooth) {
                for (int k = 0; k < info.maxIter; k++) {
                    __m256d aa = _mm256_mul_pd(a, a);
                    __m256d aa2 = _mm256_mul_pd(a2, a2);
                    __m256d bb = _mm256_mul_pd(b, b);
                    __m256d bb2 = _mm256_mul_pd(b2, b2);
                    __m256d abab = _mm256_mul_pd(a, b); abab = _mm256_add_pd(abab, abab);
                    __m256d abab2 = _mm256_mul_pd(a2, b2); abab2 = _mm256_add_pd(abab2, abab2);
                    a = _mm256_add_pd(_mm256_sub_pd(aa, bb), cx);
                    a2 = _mm256_add_pd(_mm256_sub_pd(aa2, bb2), cx2);
                    b = _mm256_add_pd(abab, cy);
                    b2 = _mm256_add_pd(abab2, cy);
                    __m256d cmp = _mm256_cmp_pd(_mm256_add_pd(aa, bb), threshold, _CMP_LE_OQ);
                    __m256d cmp2 = _mm256_cmp_pd(_mm256_add_pd(aa2, bb2), threshold, _CMP_LE_OQ);
                    resultsa = _mm256_or_pd(_mm256_andnot_pd(cmp, resultsa), _mm256_and_pd(cmp, a));
                    resultsb = _mm256_or_pd(_mm256_andnot_pd(cmp, resultsb), _mm256_and_pd(cmp, b));
                    resultsa2 = _mm256_or_pd(_mm256_andnot_pd(cmp2, resultsa2), _mm256_and_pd(cmp2, a2));
                    resultsb2 = _mm256_or_pd(_mm256_andnot_pd(cmp2, resultsb2), _mm256_and_pd(cmp2, b2));
                    adder = _mm256_and_pd(adder, cmp);
                    counter = _mm256_add_pd(counter, adder);
                    adder2 = _mm256_and_pd(adder2, cmp2);
                    counter2 = _mm256_add_pd(counter2, adder2);
                    if ((k & 0x7) == 0 && _mm256_testz_si256(_mm256_castpd_si256(cmp), _mm256_castpd_si256(cmp)) != 0 &&
                        _mm256_testz_si256(_mm256_castpd_si256(cmp2), _mm256_castpd_si256(cmp2)) != 0) {
                        break;
                    }
                }
            }
            else {
                for (int k = 0; k < info.maxIter; k++) {
                    __m256d aa = _mm256_mul_pd(a, a);
                    __m256d aa2 = _mm256_mul_pd(a2, a2);
                    __m256d bb = _mm256_mul_pd(b, b);
                    __m256d bb2 = _mm256_mul_pd(b2, b2);
                    __m256d abab = _mm256_mul_pd(a, b); abab = _mm256_add_pd(abab, abab);
                    __m256d abab2 = _mm256_mul_pd(a2, b2); abab2 = _mm256_add_pd(abab2, abab2);
                    a = _mm256_add_pd(_mm256_sub_pd(aa, bb), cx);
                    a2 = _mm256_add_pd(_mm256_sub_pd(aa2, bb2), cx2);
                    b = _mm256_add_pd(abab, cy);
                    b2 = _mm256_add_pd(abab2, cy);
                    __m256d cmp = _mm256_cmp_pd(_mm256_add_pd(aa, bb), threshold, _CMP_LE_OQ);
                    __m256d cmp2 = _mm256_cmp_pd(_mm256_add_pd(aa2, bb2), threshold, _CMP_LE_OQ);
                    adder = _mm256_and_pd(adder, cmp);
                    counter = _mm256_add_pd(counter, adder);
                    adder2 = _mm256_and_pd(adder2, cmp2);
                    counter2 = _mm256_add_pd(counter2, adder2);
                    if ((k & 0x7) == 0 && _mm256_testz_si256(_mm256_castpd_si256(cmp), _mm256_castpd_si256(cmp)) != 0 &&
                        _mm256_testz_si256(_mm256_castpd_si256(cmp2), _mm256_castpd_si256(cmp2)) != 0) {
                        break;
                    }
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
            double* resa = (double*) &resultsa;
            double* resb = (double*) &resultsb;
            _mm256_store_pd(ftRes, counter);
            for (int k = 0; k < 4 && i + k < info.bWidth; k++) {
                if (info.smooth)
                    data[i + k + j * info.bWidth] = ftRes[k] <= 0 ? info.maxIter :
                        ftRes[k] >= info.maxIter ? info.maxIter :
                        ((float)ftRes[k]) + 1 - ::log(::log(resa[k] * resa[k] + resb[k] * resb[k]) / 2) / ::log(2.0f);
                else
                    data[i + k + j * info.bWidth] = ftRes[k] > 0 ? float(ftRes[k]) : info.maxIter;
            }

            resa = (double*) &resultsa2;
            resb = (double*) &resultsb2;
            _mm256_store_pd(ftRes, counter2);
            i += 4;
            for (int k = 0; k < 4 && i + k < info.bWidth; k++) {
                if (info.smooth)
                    data[i + k + j * info.bWidth] = ftRes[k] <= 0 ? info.maxIter :
                        ftRes[k] >= info.maxIter ? info.maxIter :
                        ((float)ftRes[k]) + 1 - ::log(::log(resa[k] * resa[k] + resb[k] * resb[k]) / 2) / ::log(2.0f);
                else
                    data[i + k + j * info.bWidth] = ftRes[k] > 0 ? float(ftRes[k]) : info.maxIter;
            }
            i -= 4;
        }
    }
}


struct VecPair
{
    __m256d a;
    __m256d b;
};


static inline VecPair quickTwoSum(__m256d a, __m256d b)
{
    __m256d s = _mm256_add_pd(a, b);
    __m256d e = _mm256_sub_pd(b, _mm256_sub_pd(s, a));
    return { s, e };
}

static inline VecPair quickTwoDiff(__m256d a, __m256d b)
{
    __m256d s = _mm256_sub_pd(a, b);
    __m256d e = _mm256_sub_pd(_mm256_sub_pd(a, s), b);
    return { s, e };
}

static inline VecPair twoSum(__m256d a, __m256d b)
{
    __m256d s = _mm256_add_pd(a, b);
    __m256d bb = _mm256_sub_pd(s, a);
    __m256d e = _mm256_add_pd(_mm256_sub_pd(a, _mm256_sub_pd(s, bb)), _mm256_sub_pd(b, bb));
    return { s, e };
}

static inline VecPair twoDiff(__m256d a, __m256d b)
{
    __m256d s = _mm256_sub_pd(a, b);
    __m256d bb = _mm256_sub_pd(s, a);
    __m256d e = _mm256_sub_pd(_mm256_sub_pd(a, _mm256_sub_pd(s, bb)), _mm256_add_pd(b, bb));
    return { s, e };
}


static inline VecPair split(__m256d a)
{
    /*
    // -- this should never happen when doing mandelbrot calculations,
    //    so we omit this check.
    if (a > _QD_SPLIT_THRESH || a < -_QD_SPLIT_THRESH) {
        a *= 3.7252902984619140625e-09;  // 2^-28
        temp = _QD_SPLITTER * a;
        hi = temp - (temp - a);
        lo = a - hi;
        hi *= 268435456.0;          // 2^28
        lo *= 268435456.0;          // 2^28
    } else {
        temp = _QD_SPLITTER * a;
        hi = temp - (temp - a);
        lo = a - hi;
    }
    */

    static const __m256d SPLITTER = { 134217729.0, 134217729.0, 134217729.0, 134217729.0 };
    __m256d temp = _mm256_mul_pd(SPLITTER, a);
    __m256d hi = _mm256_sub_pd(temp, _mm256_sub_pd(temp, a));
    __m256d lo = _mm256_sub_pd(a, hi);
    return { hi, lo };
}

static inline VecPair twoProd(__m256d a, __m256d b)
{
    __m256d p = _mm256_mul_pd(a, b);
    auto[a_hi, a_lo] = split(a);
    auto[b_hi, b_lo] = split(b);
    __m256d err = _mm256_add_pd(_mm256_add_pd(_mm256_sub_pd(_mm256_mul_pd(a_hi, b_hi), p), _mm256_add_pd(_mm256_mul_pd(a_hi, b_lo), _mm256_mul_pd(a_lo, b_hi))), _mm256_mul_pd(a_lo, b_lo));
    return { p, err };
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
            _mm256_add_pd(_mm256_mul_pd(sm.x[1], x[0]), _mm256_mul_pd(sm.x[0], x[1])) );
        auto[r1, r2] = quickTwoSum(p1, p2);
        return AvxDoubleDouble{ r1, r2 };
    }
};

template<bool parallel>
void CpuGenerator<mnd::DoubleDouble, mnd::X86_AVX, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    const MandelViewport& view = info.view;

    using T = DoubleDouble;

    T viewx = mnd::convert<T>(view.x);
    T viewy = mnd::convert<T>(view.y);
    T wpp = mnd::convert<T>(view.width / info.bWidth);
    T hpp = mnd::convert<T>(view.height / info.bHeight);


    T jX = mnd::convert<T>(info.juliaX);
    T jY = mnd::convert<T>(info.juliaY);
    AvxDoubleDouble juliaX = { __m256d{ jX.x[0], jX.x[0], jX.x[0], jX.x[0] }, __m256d{ jX.x[1], jX.x[1], jX.x[1], jX.x[1] } };
    AvxDoubleDouble juliaY = { __m256d{ jY.x[0], jY.x[0], jY.x[0], jY.x[0] }, __m256d{ jY.x[1], jY.x[1], jY.x[1], jY.x[1] } };

    if constexpr(parallel)
        omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for schedule(static, 1) if (parallel)
    for (long j = 0; j < info.bHeight; j++) {
        T y = viewy + T(double(j)) * hpp;
        __m256d y0s = { y.x[0], y.x[0], y.x[0], y.x[0] };
        __m256d y1s = { y.x[1], y.x[1], y.x[1], y.x[1] };
        AvxDoubleDouble ys{ y0s, y1s };
        for (long i = 0; i < info.bWidth; i += 4) {
            T x1 = viewx + T(double(i)) * wpp;
            T x2 = x1 + wpp;
            T x3 = x2 + wpp;
            T x4 = x3 + wpp;

            __m256d x0s = {
                x1.x[0], x2.x[0], x3.x[0], x4.x[0],
            };

            __m256d x1s = {
                x1.x[1], x2.x[1], x3.x[1], x4.x[1],
            };

            AvxDoubleDouble xs{ x0s, x1s };

            AvxDoubleDouble cx = info.julia ? juliaX : xs;
            AvxDoubleDouble cy = info.julia ? juliaY : ys;

            int itRes[4] = { 0, 0, 0, 0 };

            __m256d threshold = { 16.0, 16.0, 16.0, 16.0 };
            __m256d counter = { 0, 0, 0, 0 };
            __m256d adder = { 1, 1, 1, 1 };

            AvxDoubleDouble a = xs;
            AvxDoubleDouble b = ys;

            __m256d resultsa;
            __m256d resultsb;

            for (int k = 0; k < info.maxIter; k++) {
                AvxDoubleDouble aa = a * a;
                AvxDoubleDouble bb = b * b;
                AvxDoubleDouble abab = a * b; abab = abab + abab;
                a = aa - bb + cx;
                b = abab + cy;
                __m256d cmp = _mm256_cmp_pd(_mm256_add_pd(aa.x[0], bb.x[0]), threshold, _CMP_LE_OQ);
                if (info.smooth) {
                    resultsa = _mm256_or_pd(_mm256_andnot_pd(cmp, resultsa), _mm256_and_pd(cmp, a.x[0]));
                    resultsb = _mm256_or_pd(_mm256_andnot_pd(cmp, resultsb), _mm256_and_pd(cmp, b.x[0]));
                }
                adder = _mm256_and_pd(adder, cmp);
                counter = _mm256_add_pd(counter, adder);
                if (_mm256_testz_si256(_mm256_castpd_si256(cmp), _mm256_castpd_si256(cmp)) != 0) {
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
            double* resa = (double*) &resultsa;
            double* resb = (double*) &resultsb;
            _mm256_store_pd(ftRes, counter);

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



