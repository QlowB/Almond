#include "FloatLog.h"

#include <immintrin.h>
#include <omp.h>

///
/// \brief unique namespace just to be a little more sure we do not
///        accidentally compile a function used somewhere else and use
///        avx instructions in there.
///
namespace avx_private
{
#include "LightDoubleDouble.h"
#include "TripleDouble.h"
}


void generateFloatAvx(long width, long height, float* data, bool parallel,
    float vx, float vy, float vw, float vh, int maxIter, bool smooth,
    bool julia, float jX, float jY)
{
    using T = float;
    const float dppf = float(vw / width);
    __m256 viewx = _mm256_set1_ps(vx);
    __m256 dpp = _mm256_set1_ps(dppf);

    __m256 juliaX = { jX, jX, jX, jX, jX, jX, jX, jX };
    __m256 juliaY = { jY, jY, jY, jY, jY, jY, jY, jY };

#if defined(_OPENMP)
    if (parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < height; j++) {
        T y = vy + T(j) * vw / height;
        __m256 ys = _mm256_set1_ps(y);
        for (long i = 0; i < width; i += 16) {
            __m256 pixc = { float(i), float(i + 1), float(i + 2), float(i + 3), float(i + 4), float(i + 5), float(i + 6), float(i + 7) };
            __m256 pixc2 = { float(i + 8), float(i + 9), float(i + 10), float(i + 11), float(i + 12), float(i + 13), float(i + 14), float(i + 15) };

            __m256 xs = _mm256_add_ps(_mm256_mul_ps(dpp, pixc), viewx);
            __m256 xs2 = _mm256_add_ps(_mm256_mul_ps(dpp, pixc2), viewx);

            __m256 counter = _mm256_setzero_ps();
            __m256 adder = _mm256_set1_ps(1);

            __m256 counter2 = _mm256_setzero_ps();
            __m256 adder2 = _mm256_set1_ps(1);

            __m256 threshold = _mm256_set1_ps(16);

            __m256 a = xs;
            __m256 a2 = xs2;
            __m256 b = ys;
            __m256 b2 = ys;

            __m256 cx = julia ? juliaX : xs;
            __m256 cx2 = julia ? juliaX : xs2;
            __m256 cy = julia ? juliaY : ys;

            __m256 resultsa = a;
            __m256 resultsb = b;
            __m256 resultsa2 = a2;
            __m256 resultsb2 = b2;

            if (smooth) {
                __m256 cmp = _mm256_cmp_ps(a, a, _CMP_LE_OQ);
                __m256 cmp2 = _mm256_cmp_ps(a, a, _CMP_LE_OQ);
                for (int k = 0; k < maxIter; k++) {
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
                    resultsa = _mm256_or_ps(_mm256_andnot_ps(cmp, resultsa), _mm256_and_ps(cmp, a));
                    resultsb = _mm256_or_ps(_mm256_andnot_ps(cmp, resultsb), _mm256_and_ps(cmp, b));
                    resultsa2 = _mm256_or_ps(_mm256_andnot_ps(cmp2, resultsa2), _mm256_and_ps(cmp2, a2));
                    resultsb2 = _mm256_or_ps(_mm256_andnot_ps(cmp2, resultsb2), _mm256_and_ps(cmp2, b2));
                    cmp = _mm256_cmp_ps(_mm256_add_ps(aa, bb), threshold, _CMP_LE_OQ);
                    cmp2 = _mm256_cmp_ps(_mm256_add_ps(aa2, bb2), threshold, _CMP_LE_OQ);
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
                for (int k = 0; k < maxIter; k++) {
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

            float resData[64];
            float* ftRes = resData;
            float* resa = ftRes + 16;
            float* resb = resa + 16;

            _mm256_storeu_ps(ftRes, counter);
            _mm256_storeu_ps(ftRes + 8, counter2);
            _mm256_storeu_ps(resa, resultsa);
            _mm256_storeu_ps(resa + 8, resultsa2);
            _mm256_storeu_ps(resb, resultsb);
            _mm256_storeu_ps(resb + 8, resultsb2);
            for (int k = 0; k < 16 && i + k < width; k++) {
                if (smooth) {
                    data[i + k + j * width] = ftRes[k] < 0 ? maxIter :
                        ftRes[k] >= maxIter ? maxIter :
                        ((float)ftRes[k]) + 1 - floatLog2(floatLog(resa[k] * resa[k] + resb[k] * resb[k]) * 0.5f);
                }
                else {
                    data[i + k + j * width] = ftRes[k] < 0 ? maxIter : ftRes[k];
                }
            }
        }
    }
}


void generateDoubleAvx(long width, long height, float* data, bool parallel,
    double vx, double vy, double vw, double vh, int maxIter, bool smooth,
    bool julia, double jX, double jY)
{
    using T = double;

    const double dppf = double(vw / width);
    __m256d viewx = { vx, vx, vx, vx };
    __m256d dpp = { dppf, dppf, dppf, dppf };

    __m256d juliaX = { jX, jX, jX, jX };
    __m256d juliaY = { jY, jY, jY, jY };

#if defined(_OPENMP)
    if (parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < height; j++) {
        T y = vy + T(j) * vh / height;
        __m256d ys = { y, y, y, y };
        for (long i = 0; i < width; i += 8) {
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

            __m256d cx = julia ? juliaX : xs;
            __m256d cx2 = julia ? juliaX : xs2;
            __m256d cy = julia ? juliaY : ys;

            if (smooth) {
                __m256d cmp = _mm256_cmp_pd(a, a, _CMP_LE_OQ);
                __m256d cmp2 = _mm256_cmp_pd(a, a, _CMP_LE_OQ);
                for (int k = 0; k < maxIter; k++) {
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
                    resultsa = _mm256_or_pd(_mm256_andnot_pd(cmp, resultsa), _mm256_and_pd(cmp, a));
                    resultsb = _mm256_or_pd(_mm256_andnot_pd(cmp, resultsb), _mm256_and_pd(cmp, b));
                    resultsa2 = _mm256_or_pd(_mm256_andnot_pd(cmp2, resultsa2), _mm256_and_pd(cmp2, a2));
                    resultsb2 = _mm256_or_pd(_mm256_andnot_pd(cmp2, resultsb2), _mm256_and_pd(cmp2, b2));
                    cmp = _mm256_cmp_pd(_mm256_add_pd(aa, bb), threshold, _CMP_LE_OQ);
                    cmp2 = _mm256_cmp_pd(_mm256_add_pd(aa2, bb2), threshold, _CMP_LE_OQ);
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
                for (int k = 0; k < maxIter; k++) {
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

            double resData[8];
            double* ftRes = resData;
            double* resa = (double*) &resultsa;
            double* resb = (double*) &resultsb;
            _mm256_storeu_pd(ftRes, counter);
            for (int k = 0; k < 4 && i + k < width; k++) {
                if (smooth)
                    data[i + k + j * width] = ftRes[k] < 0 ? float(maxIter) :
                        ftRes[k] >= maxIter ? float(maxIter) :
                        float(((float)ftRes[k]) + 1 - floatLog2(floatLog(float(resa[k] * resa[k] + resb[k] * resb[k])) / 2));
                else
                    data[i + k + j * width] = ftRes[k] >= 0 ? float(ftRes[k]) : maxIter;
            }

            resa = (double*) &resultsa2;
            resb = (double*) &resultsb2;
            _mm256_storeu_pd(ftRes, counter2);
            i += 4;
            for (int k = 0; k < 4 && i + k < width; k++) {
                if (smooth)
                    data[i + k + j * width] = ftRes[k] < 0 ? float(maxIter) :
                        ftRes[k] >= maxIter ? float(maxIter) :
                        float(((float)ftRes[k]) + 1 - floatLog2(floatLog(float(resa[k] * resa[k] + resb[k] * resb[k])) / 2));
                else
                    data[i + k + j * width] = ftRes[k] >= 0 ? float(ftRes[k]) : maxIter;
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


static inline VecPair threeTwoSum(__m256d a, __m256d b, __m256d c)
{
    auto[t1, t2] = twoSum(a, b);
    auto[r0, t3] = twoSum(t1, c);
    return { r0, _mm256_add_pd(t2, t3) };
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

    inline AvxDoubleDouble(double a, double b) :
        x{ _mm256_set1_pd(a), _mm256_set1_pd(b) }
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


void generateDoubleDoubleAvx(long width, long height, float* data, bool parallel,
    double vx1, double vx2, double vy1, double vy2, double vw1, double vw2, double vh1, double vh2, int maxIter, bool smooth,
    bool julia, double jX1, double jX2, double jY1, double jY2)
{
    using namespace avx_private;
    using T = mnd::LightDoubleDouble;

    T viewx{ vx1, vx2 };
    T viewy{ vy1, vy2 };
    T wpp = T{ vw1, vw2 } * T(1.0 / width);
    T hpp = T{ vh1, vh2 } * T(1.0 / height);

    T jX{ jX1, jX2 };
    T jY{ jY1, jY2 };
    AvxDoubleDouble juliaX = { jX[0], jX[1] };
    AvxDoubleDouble juliaY = { jY[0], jY[1] };

#if defined(_OPENMP)
    if (parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < height; j++) {
        T y = viewy + T(double(j)) * hpp;
        AvxDoubleDouble ys{ y[0], y[1] };
        for (long i = 0; i < width; i += 4) {
            T x1 = viewx + T(double(i)) * wpp;
            T x2 = x1 + wpp;
            T x3 = x2 + wpp;
            T x4 = x3 + wpp;

            __m256d x0s = {
                x1[0], x2[0], x3[0], x4[0],
            };

            __m256d x1s = {
                x1[1], x2[1], x3[1], x4[1],
            };

            AvxDoubleDouble xs{ x0s, x1s };

            AvxDoubleDouble cx = julia ? juliaX : xs;
            AvxDoubleDouble cy = julia ? juliaY : ys;

            int itRes[4] = { 0, 0, 0, 0 };

            __m256d threshold = { 16.0, 16.0, 16.0, 16.0 };
            __m256d counter = { 0, 0, 0, 0 };
            __m256d adder = { 1, 1, 1, 1 };

            AvxDoubleDouble a = xs;
            AvxDoubleDouble b = ys;

            __m256d resultsa = _mm256_set1_pd(0);
            __m256d resultsb = _mm256_set1_pd(0);

            __m256d cmp = _mm256_cmp_pd(threshold, threshold, _CMP_LE_OQ);
            for (int k = 0; k < maxIter; k++) {
                AvxDoubleDouble aa = a * a;
                AvxDoubleDouble bb = b * b;
                AvxDoubleDouble abab = a * b; abab = abab + abab;
                a = aa - bb + cx;
                b = abab + cy;
                if (smooth) {
                    resultsa = _mm256_or_pd(_mm256_andnot_pd(cmp, resultsa), _mm256_and_pd(cmp, a.x[0]));
                    resultsb = _mm256_or_pd(_mm256_andnot_pd(cmp, resultsb), _mm256_and_pd(cmp, b.x[0]));
                }
                cmp = _mm256_cmp_pd(_mm256_add_pd(aa.x[0], bb.x[0]), threshold, _CMP_LE_OQ);
                adder = _mm256_and_pd(adder, cmp);
                counter = _mm256_add_pd(counter, adder);
                if (_mm256_testz_si256(_mm256_castpd_si256(cmp), _mm256_castpd_si256(cmp)) != 0) {
                    break;
                }
            }

            double resData[8];
            double* ftRes = resData;
            double* resa = (double*) &resultsa;
            double* resb = (double*) &resultsb;
            _mm256_storeu_pd(ftRes, counter);

            for (int k = 0; k < 4 && i + k < width; k++) {
                if (smooth)
                    data[i + k + j * width] = float(ftRes[k] < 0 ? maxIter :
                        ftRes[k] >= maxIter ? maxIter :
                        ((float)ftRes[k]) + 1 - floatLog2(::floatLog(float(resa[k] * resa[k] + resb[k] * resb[k])) / 2));
                else
                    data[i + k + j * width] = ftRes[k] >= 0 ? float(ftRes[k]) : maxIter;
            }
        }
    }
}

struct AvxTripleDouble
{
    __m256d x[3];

    inline AvxTripleDouble(__m256d a, __m256d b, __m256d c) :
        x{ a, b, c }
    {}

    inline AvxTripleDouble(double a, double b, double c) :
        x{ _mm256_set1_pd(a), _mm256_set1_pd(b), _mm256_set1_pd(c) }
    {}

    inline AvxTripleDouble operator + (const AvxTripleDouble& b) const
    {
        const auto& a = *this;
        auto[r0, t0] = twoSum(a.x[0], b.x[0]);
        auto[t1, t2] = twoSum(a.x[1], b.x[1]);
        auto[r1, t3] = twoSum(t0, t1);
        auto r2 = _mm256_add_pd(_mm256_add_pd(t2, _mm256_add_pd(a.x[2], b.x[2])), t3);

        auto[re1, t4] = quickTwoSum(r0, r1);
        auto[re2, re3] = quickTwoSum(t4, r2);
        return { re1, re2, re3 };
    }

    inline AvxTripleDouble operator - (const AvxTripleDouble& b) const
    {
        const auto& a = *this;
        auto[r0, t0] = twoDiff(a.x[0], b.x[0]);
        auto[t1, t2] = twoDiff(a.x[1], b.x[1]);
        auto[r1, t3] = twoSum(t0, t1);
        auto r2 = _mm256_add_pd(_mm256_add_pd(t2, _mm256_sub_pd(a.x[2], b.x[2])), t3);

        auto[re1, t4] = quickTwoSum(r0, r1);
        auto[re2, re3] = quickTwoSum(t4, r2);
        return { re1, re2, re3 };
    }

    inline AvxTripleDouble operator * (const AvxTripleDouble& b) const
    {
        const auto& a = *this;
        auto[p1_0, p2_0] = twoProd(a.x[0], b.x[0]);
        auto[p2_1, p3_0] = twoProd(a.x[0], b.x[1]);
        auto[p2_2, p3_1] = twoProd(a.x[1], b.x[0]);

        auto[t2, tl3] = threeTwoSum(p2_0, p2_1, p2_2);
        auto t3 = _mm256_add_pd(tl3,
            _mm256_add_pd(
                _mm256_add_pd(p3_0, p3_1),
                _mm256_add_pd(
                    _mm256_mul_pd(a.x[1], b.x[1]),
                    _mm256_add_pd(
                        _mm256_mul_pd(a.x[2], b.x[0]),
                        _mm256_mul_pd(a.x[0], b.x[2])
                    )
                )
            )
            );
        auto[re0, q2] = quickTwoSum(p1_0, t2);
        auto[re1, re2] = quickTwoSum(q2, t3);
        return { re0, re1, re2 };
    }
};


void generateTripleDoubleAvx(long width, long height, float* data, bool parallel,
    double vx1, double vx2, double vx3, double vy1, double vy2, double vy3,
    double vw1, double vw2, double vw3, double vh1, double vh2, double vh3,
    int maxIter, bool smooth, bool julia, double jX1,
    double jX2, double jX3, double jY1, double jY2, double jY3)
{
    using namespace avx_private;
    using T = mnd::TripleDouble;

    T viewx{ vx1, vx2, vx3 };
    T viewy{ vy1, vy2, vy3 };
    T wpp = T{ vw1, vw2, vw3 } * T(1.0 / width);
    T hpp = T{ vh1, vh2, vh3 } * T(1.0 / height);

    T jX{ jX1, jX2, jX3 };
    T jY{ jY1, jY2, jY3 };
    AvxTripleDouble juliaX = { jX[0], jX[1], jX[2] };
    AvxTripleDouble juliaY = { jY[0], jY[1], jY[2] };

#if defined(_OPENMP)
    if (parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < height; j++) {
        T y = viewy + T(double(j)) * hpp;
        AvxTripleDouble ys{ y[0], y[1], y[2] };
        for (long i = 0; i < width; i += 4) {
            T x1 = viewx + T(double(i)) * wpp;
            T x2 = x1 + wpp;
            T x3 = x2 + wpp;
            T x4 = x3 + wpp;

            __m256d x0s = {
                x1[0], x2[0], x3[0], x4[0],
            };
            __m256d x1s = {
                x1[1], x2[1], x3[1], x4[1],
            };
            __m256d x2s = {
                x1[2], x2[2], x3[2], x4[2],
            };

            AvxTripleDouble xs{ x0s, x1s, x2s };

            AvxTripleDouble cx = julia ? juliaX : xs;
            AvxTripleDouble cy = julia ? juliaY : ys;

            int itRes[4] = { 0, 0, 0, 0 };

            __m256d threshold = { 16.0, 16.0, 16.0, 16.0 };
            __m256d counter = { 0, 0, 0, 0 };
            __m256d adder = { 1, 1, 1, 1 };

            AvxTripleDouble a = xs;
            AvxTripleDouble b = ys;

            __m256d resultsa = _mm256_set1_pd(0);
            __m256d resultsb = _mm256_set1_pd(0);

            __m256d cmp = _mm256_cmp_pd(threshold, threshold, _CMP_LE_OQ);
            for (int k = 0; k < maxIter; k++) {
                AvxTripleDouble aa = a * a;
                AvxTripleDouble bb = b * b;
                AvxTripleDouble abab = a * b; abab = abab + abab;
                a = aa - bb + cx;
                b = abab + cy;
                if (smooth) {
                    resultsa = _mm256_or_pd(_mm256_andnot_pd(cmp, resultsa), _mm256_and_pd(cmp, a.x[0]));
                    resultsb = _mm256_or_pd(_mm256_andnot_pd(cmp, resultsb), _mm256_and_pd(cmp, b.x[0]));
                }
                cmp = _mm256_cmp_pd(_mm256_add_pd(aa.x[0], bb.x[0]), threshold, _CMP_LE_OQ);
                adder = _mm256_and_pd(adder, cmp);
                counter = _mm256_add_pd(counter, adder);
                if (_mm256_testz_si256(_mm256_castpd_si256(cmp), _mm256_castpd_si256(cmp)) != 0) {
                    break;
                }
            }

            double resData[8];
            double* ftRes = resData;
            double* resa = (double*) &resultsa;
            double* resb = (double*) &resultsb;
            _mm256_storeu_pd(ftRes, counter);

            for (int k = 0; k < 4 && i + k < width; k++) {
                if (smooth)
                    data[i + k + j * width] = float(ftRes[k] < 0 ? maxIter :
                        ftRes[k] >= maxIter ? maxIter :
                        ((float)ftRes[k]) + 1 - floatLog2(::floatLog(float(resa[k] * resa[k] + resb[k] * resb[k])) / 2));
                else
                    data[i + k + j * width] = ftRes[k] >= 0 ? float(ftRes[k]) : maxIter;
            }
        }
    }
}


