#include "FloatLog.h"

#include <immintrin.h>
#include <omp.h>

///
/// \brief unique namespace just to be a little more sure we do not
///        accidentally compile a function used somewhere else and use
///        avx instructions in there.
///
namespace avxfma_private
{
#include "LightDoubleDouble.h"
#include "QuadDouble.h"
#include "HexDouble.h"
}



void generateFloatAvxFma(long width, long height, float* data, bool parallel,
    float vx, float vy, float vw, float vh, int maxIter, bool smooth,
    bool julia, float jX, float jY)
{
    using T = float;
    const float dppf = float(vw / width);
    const float viewxf = vx; 
    __m256 viewx = { viewxf, viewxf, viewxf, viewxf, viewxf, viewxf, viewxf, viewxf };
    __m256 dpp = { dppf, dppf, dppf, dppf, dppf, dppf, dppf, dppf };

    __m256 juliaX = { jX, jX, jX, jX, jX, jX, jX, jX };
    __m256 juliaY = { jY, jY, jY, jY, jY, jY, jY, jY };

#if defined(_OPENMP)
    if (parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < height; j++) {
        T y = vy + T(j) * vw / height;
        __m256 ys = {y, y, y, y, y, y, y, y};
        for (long i = 0; i < width; i += 24) {
            __m256 pixc = { float(i), float(i + 1), float(i + 2), float(i + 3), float(i + 4), float(i + 5), float(i + 6), float(i + 7) };
            __m256 pixc2 = { float(i + 8), float(i + 9), float(i + 10), float(i + 11), float(i + 12), float(i + 13), float(i + 14), float(i + 15) };
            __m256 pixc3 = { float(i + 16), float(i + 17), float(i + 18), float(i + 19), float(i + 20), float(i + 21), float(i + 22), float(i + 23) };

            __m256 xs = _mm256_add_ps(_mm256_mul_ps(dpp, pixc), viewx);
            __m256 xs2 = _mm256_add_ps(_mm256_mul_ps(dpp, pixc2), viewx);
            __m256 xs3 = _mm256_add_ps(_mm256_mul_ps(dpp, pixc3), viewx);

            __m256 counter = { 0, 0, 0, 0, 0, 0, 0, 0 };
            __m256 adder = { 1, 1, 1, 1, 1, 1, 1, 1 };
            __m256 resultsa = { 0, 0, 0, 0, 0, 0, 0, 0 };
            __m256 resultsb = { 0, 0, 0, 0, 0, 0, 0, 0 };

            __m256 counter2 = { 0, 0, 0, 0, 0, 0, 0, 0 };
            __m256 adder2 = { 1, 1, 1, 1, 1, 1, 1, 1 };
            __m256 resultsa2 = { 0, 0, 0, 0, 0, 0, 0, 0 };
            __m256 resultsb2 = { 0, 0, 0, 0, 0, 0, 0, 0 };

            __m256 counter3 = { 0, 0, 0, 0, 0, 0, 0, 0 };
            __m256 adder3 = { 1, 1, 1, 1, 1, 1, 1, 1 };
            __m256 resultsa3 = { 0, 0, 0, 0, 0, 0, 0, 0 };
            __m256 resultsb3 = { 0, 0, 0, 0, 0, 0, 0, 0 };

            __m256 threshold = { 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f };
            __m256 two = { 2, 2, 2, 2, 2, 2, 2, 2 };

            __m256 a = xs;
            __m256 a2 = xs2;
            __m256 a3 = xs3;
            __m256 b = ys;
            __m256 b2 = ys;
            __m256 b3 = ys;

            __m256 cx = julia ? juliaX : xs;
            __m256 cx2 = julia ? juliaX : xs2;
            __m256 cx3 = julia ? juliaX : xs3;
            __m256 cy = julia ? juliaY : ys;

            if (smooth) {
                __m256 cmp = _mm256_cmp_ps(threshold, threshold, _CMP_LE_OQ);
                __m256 cmp2 = _mm256_cmp_ps(threshold, threshold, _CMP_LE_OQ);
                __m256 cmp3 = _mm256_cmp_ps(threshold, threshold, _CMP_LE_OQ);
                for (int k = 0; k < maxIter; k++) {
                    __m256 bb = _mm256_mul_ps(b, b);
                    __m256 bb2 = _mm256_mul_ps(b2, b2);
                    __m256 bb3 = _mm256_mul_ps(b3, b3);
                    __m256 ab = _mm256_mul_ps(a, b);
                    __m256 ab2 = _mm256_mul_ps(a2, b2);
                    __m256 ab3 = _mm256_mul_ps(a3, b3);
                    __m256 olda = a;
                    __m256 olda2 = a2;
                    __m256 olda3 = a3;
                    a = _mm256_add_ps(_mm256_fmsub_ps(a, a, bb), cx);
                    a2 = _mm256_add_ps(_mm256_fmsub_ps(a2, a2, bb2), cx2);
                    a3 = _mm256_add_ps(_mm256_fmsub_ps(a3, a3, bb3), cx3);
                    b = _mm256_fmadd_ps(two, ab, cy);
                    b2 = _mm256_fmadd_ps(two, ab2, cy);
                    b3 = _mm256_fmadd_ps(two, ab3, cy);
                    /*resultsa = _mm256_or_ps(_mm256_andnot_ps(cmp, resultsa), _mm256_and_ps(cmp, a));
                    resultsb = _mm256_or_ps(_mm256_andnot_ps(cmp, resultsb), _mm256_and_ps(cmp, b));
                    resultsa2 = _mm256_or_ps(_mm256_andnot_ps(cmp2, resultsa2), _mm256_and_ps(cmp2, a2));
                    resultsb2 = _mm256_or_ps(_mm256_andnot_ps(cmp2, resultsb2), _mm256_and_ps(cmp2, b2));
                    resultsa3 = _mm256_or_ps(_mm256_andnot_ps(cmp3, resultsa3), _mm256_and_ps(cmp3, a3));
                    resultsb3 = _mm256_or_ps(_mm256_andnot_ps(cmp3, resultsb3), _mm256_and_ps(cmp3, b3));*/
                    resultsa = _mm256_blendv_ps(resultsa, a, cmp);
                    resultsb = _mm256_blendv_ps(resultsb, b, cmp);
                    resultsa2 = _mm256_blendv_ps(resultsa2, a2, cmp2);
                    resultsb2 = _mm256_blendv_ps(resultsb2, b2, cmp2);
                    resultsa3 = _mm256_blendv_ps(resultsa3, a3, cmp3);
                    resultsb3 = _mm256_blendv_ps(resultsb3, b3, cmp3);
                    cmp = _mm256_cmp_ps(_mm256_fmadd_ps(olda, olda, bb), threshold, _CMP_LE_OQ);
                    cmp2 = _mm256_cmp_ps(_mm256_fmadd_ps(olda2, olda2, bb2), threshold, _CMP_LE_OQ);
                    cmp3 = _mm256_cmp_ps(_mm256_fmadd_ps(olda3, olda3, bb3), threshold, _CMP_LE_OQ);
                    adder = _mm256_and_ps(adder, cmp);
                    counter = _mm256_add_ps(counter, adder);
                    adder2 = _mm256_and_ps(adder2, cmp2);
                    counter2 = _mm256_add_ps(counter2, adder2);
                    adder3 = _mm256_and_ps(adder3, cmp3);
                    counter3 = _mm256_add_ps(counter3, adder3);
                    if ((k & 0x7) == 0 && _mm256_testz_ps(cmp, cmp) != 0 && _mm256_testz_ps(cmp2, cmp2) != 0 && _mm256_testz_ps(cmp3, cmp3) != 0) {
                        break;
                    }
                }
            }
            else {
                for (int k = 0; k < maxIter; k++) {
                    __m256 bb = _mm256_mul_ps(b, b);
                    __m256 bb2 = _mm256_mul_ps(b2, b2);
                    __m256 bb3 = _mm256_mul_ps(b3, b3);
                    __m256 ab = _mm256_mul_ps(a, b);
                    __m256 ab2 = _mm256_mul_ps(a2, b2);
                    __m256 ab3 = _mm256_mul_ps(a3, b3);
                    __m256 cmp = _mm256_cmp_ps(_mm256_fmadd_ps(a, a, bb), threshold, _CMP_LE_OQ);
                    __m256 cmp2 = _mm256_cmp_ps(_mm256_fmadd_ps(a2, a2, bb2), threshold, _CMP_LE_OQ);
                    __m256 cmp3 = _mm256_cmp_ps(_mm256_fmadd_ps(a3, a3, bb3), threshold, _CMP_LE_OQ);
                    a = _mm256_add_ps(_mm256_fmsub_ps(a, a, bb), cx);
                    a2 = _mm256_add_ps(_mm256_fmsub_ps(a2, a2, bb2), cx2);
                    a3 = _mm256_add_ps(_mm256_fmsub_ps(a3, a3, bb3), cx3);
                    b = _mm256_fmadd_ps(two, ab, cy);
                    b2 = _mm256_fmadd_ps(two, ab2, cy);
                    b3 = _mm256_fmadd_ps(two, ab3, cy);
                    adder = _mm256_and_ps(adder, cmp);
                    counter = _mm256_add_ps(counter, adder);
                    adder2 = _mm256_and_ps(adder2, cmp2);
                    counter2 = _mm256_add_ps(counter2, adder2);
                    adder3 = _mm256_and_ps(adder3, cmp3);
                    counter3 = _mm256_add_ps(counter3, adder3);
                    if ((k & 0x7) == 0 && _mm256_testz_ps(cmp, cmp) != 0 && _mm256_testz_ps(cmp2, cmp2) != 0 && _mm256_testz_ps(cmp3, cmp3) != 0) {
                        break;
                    }
                }
            }

            float resData[96];
            float* ftRes = resData;
            float* resa = ftRes + 24;
            float* resb = resa + 24;

            _mm256_storeu_ps(ftRes, counter);
            _mm256_storeu_ps(ftRes + 8, counter2);
            _mm256_storeu_ps(ftRes + 16, counter3);
            _mm256_storeu_ps(resa, resultsa);
            _mm256_storeu_ps(resa + 8, resultsa2);
            _mm256_storeu_ps(resa + 16, resultsa3);
            _mm256_storeu_ps(resb, resultsb);
            _mm256_storeu_ps(resb + 8, resultsb2);
            _mm256_storeu_ps(resb + 16, resultsb3);
            for (int k = 0; k < 24 && i + k < width; k++) {
                if (smooth) {
                    data[i + k + j * width] = ftRes[k] < 0 ? maxIter :
                        ftRes[k] >= maxIter ? maxIter :
                        ((float)ftRes[k]) + 1 - floatLog2(floatLog(resa[k] * resa[k] + resb[k] * resb[k]) / 2);
                }
                else {
                    data[i + k + j * width] = ftRes[k] < 0 ? maxIter : ftRes[k];
                }
            }
        }
    }
}


void generateDoubleAvxFma(long width, long height, float* data, bool parallel,
    double vx, double vy, double vw, double vh, int maxIter, bool smooth,
    bool julia, double jX, double jY)
{
    using T = double;

    const double dppf = double(vw / width);
    const double viewxf = double(vx);
    __m256d viewx = { viewxf, viewxf, viewxf, viewxf };
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
            __m256d xs = _mm256_fmadd_pd(dpp, pixc, viewx);
            __m256d xs2 = _mm256_fmadd_pd(dpp, pixc2, viewx);

            int itRes[4] = { 0, 0, 0, 0 };

            __m256d threshold = { 16.0, 16.0, 16.0, 16.0 };
            __m256d counter = { 0, 0, 0, 0 };
            __m256d adder = { 1, 1, 1, 1 };
            __m256d counter2 = { 0, 0, 0, 0 };
            __m256d adder2 = { 1, 1, 1, 1 };
            __m256d two = { 2, 2, 2, 2 };

            __m256d resultsa = { 0, 0, 0, 0 };
            __m256d resultsb = { 0, 0, 0, 0 };
            __m256d resultsa2 = { 0, 0, 0, 0 };
            __m256d resultsb2 = { 0, 0, 0, 0 };

            __m256d a = xs;
            __m256d b = ys;
            __m256d a2 = xs2;
            __m256d b2 = ys;

            __m256d cx = julia ? juliaX : xs;
            __m256d cy = julia ? juliaY : ys;
            __m256d cx2 = julia ? juliaX : xs2;
            //__m256d cy2 = info.julia ? juliaY : ys;

            __m256d cmp = _mm256_cmp_pd(threshold, threshold, _CMP_LE_OQ);
            __m256d cmp2 = _mm256_cmp_pd(threshold, threshold, _CMP_LE_OQ);
            for (int k = 0; k < maxIter; k++) {
                __m256d aa = _mm256_mul_pd(a, a);
                __m256d ab = _mm256_mul_pd(a, b);
                __m256d bb = _mm256_mul_pd(b, b);
                __m256d aa2 = _mm256_mul_pd(a2, a2);
                __m256d ab2 = _mm256_mul_pd(a2, b2);
                __m256d bb2 = _mm256_mul_pd(b2, b2);
                a = _mm256_fmsub_pd(a, a, bb);
                a = _mm256_add_pd(a, cx);
                a2 = _mm256_fmsub_pd(a2, a2, bb2);
                a2 = _mm256_add_pd(a2, cx2);
                b = _mm256_fmadd_pd(two, ab, cy);
                b2 = _mm256_fmadd_pd(two, ab2, cy);
                if (smooth) {
                    resultsa = _mm256_blendv_pd(resultsa, a, cmp);
                    resultsb = _mm256_blendv_pd(resultsb, b, cmp);
                    resultsa2 = _mm256_blendv_pd(resultsa2, a2, cmp2);
                    resultsb2 = _mm256_blendv_pd(resultsb2, b2, cmp2);
                }
                cmp = _mm256_cmp_pd(_mm256_add_pd(aa, bb), threshold, _CMP_LE_OQ);
                cmp2 = _mm256_cmp_pd(_mm256_add_pd(aa2, bb2), threshold, _CMP_LE_OQ);
                adder = _mm256_and_pd(adder, cmp);
                adder2 = _mm256_and_pd(adder2, cmp2);
                counter = _mm256_add_pd(counter, adder);
                counter2 = _mm256_add_pd(counter2, adder2);
                if ((k & 0x7) == 0 && _mm256_testz_si256(_mm256_castpd_si256(cmp), _mm256_castpd_si256(cmp)) != 0 &&
                        _mm256_testz_si256(_mm256_castpd_si256(cmp2), _mm256_castpd_si256(cmp2)) != 0) {
                    break;
                }
            }

            double resData[24];
            double* ftRes = resData;
            double* resa = ftRes + 8;
            double* resb = ftRes + 16;
            _mm256_storeu_pd(ftRes, counter);
            _mm256_storeu_pd(ftRes + 4, counter2);
            _mm256_storeu_pd(resa, resultsa);
            _mm256_storeu_pd(resa + 4, resultsa2);
            _mm256_storeu_pd(resb, resultsb);
            _mm256_storeu_pd(resb + 4, resultsb2);
            for (int k = 0; k < 8 && i + k < width; k++) {
                if (smooth)
                    data[i + k + j * width] = ftRes[k] < 0 ? maxIter :
                        ftRes[k] >= maxIter ? maxIter :
                        ((float)ftRes[k]) + 1 - floatLog2(floatLog(resa[k] * resa[k] + resb[k] * resb[k]) / 2);
                else
                    data[i + k + j * width] = ftRes[k] < 0 ? maxIter : float(ftRes[k]);
            }
        }
    }
}


namespace avxfma_private
{


struct VecPair
{
    __m256d a;
    __m256d b;
};

struct VecTriple
{
    __m256d a;
    __m256d b;
    __m256d c;
};

struct VecQuadruple
{
    __m256d a;
    __m256d b;
    __m256d c;
    __m256d d;
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


static inline VecTriple threeSum(__m256d a, __m256d b, __m256d c)
{
    auto [s, e] = twoSum(a, b);
    auto [r0, e2] = twoSum(s, c);
    auto [r1, r2] = twoSum(e, e2);

    return { r0, r1, r2 };
}


static inline VecPair threeTwoSum(__m256d a, __m256d b, __m256d c)
{
    auto[t, e1] = twoSum(a, b);
    auto[s, e2] = twoSum(t, c);
    return { s, _mm256_add_pd(e1, e2) };
}


static inline __m256d threeOneSum(__m256d a, __m256d b, __m256d c)
{
    return _mm256_add_pd(a, _mm256_add_pd(b, c));
}


static inline VecQuadruple fourSum(__m256d a, __m256d b, __m256d c, __m256d d)
{
    auto[t1, t2] = twoSum(a, b);
    auto[t3, t4] = twoSum(t1, c);
    auto[r0, t5] = twoSum(t3, d);
    auto[r1, r2, r3] = threeSum(t2, t4, t5);
    return { r0, r1, r2, r3 };
}

static inline VecPair fiveTwoSum(__m256d a, __m256d b, __m256d c, __m256d d, __m256d e)
{
    auto[t1, t2] = twoSum(a, b);
    auto[t3, t4] = twoSum(t1, c);
    auto[t5, t6] = twoSum(t3, d);
    auto[r0, t7] = twoSum(t5, e);
    return { r0, _mm256_add_pd(_mm256_add_pd(_mm256_add_pd(t2, t4), t6), t7) };
}


static inline VecTriple sixThreeSum(__m256d a, __m256d b, __m256d c,
                                    __m256d d, __m256d e, __m256d f)
{
    auto[x0, x1, x2] = threeSum(a, b, c);
    auto[y0, y1, y2] = threeSum(d, e, f);

    auto[r0, t0] = twoSum(x0, y0);
    auto[t1, t2] = twoSum(x1, y1);
    auto[r1, t3] = twoSum(t0, t1);
    auto t4 = _mm256_add_pd(x2, y2);
    auto r2 = threeOneSum(t2, t3, t4);

    return { r0, r1, r2 };
}

static inline VecPair sixTwoSum(__m256d a, __m256d b, __m256d c,
                                    __m256d d, __m256d e, __m256d f)
{
    auto[x0, x1, x2] = threeSum(a, b, c);
    auto[y0, y1, y2] = threeSum(d, e, f);

    auto[r0, t0] = twoSum(x0, y0);
    auto[t1, t2] = twoSum(x1, y1);
    auto[r1, t3] = twoSum(t0, t1);
    r1 = _mm256_add_pd(r1, _mm256_add_pd(x2, y2));
    r1 = _mm256_add_pd(r1, t3);

    return { r0, r1 };
}

static inline VecPair addDD(const VecPair& a, const VecPair& b)
{
    auto[s, e] = twoSum(a.a, b.a);
    e = _mm256_add_pd(e, _mm256_add_pd(a.b, b.b));
    auto[r1, r2] = quickTwoSum(s, e);
    return { r1, r2 };
}

static inline VecPair nineTwoSum(__m256d a, __m256d b, __m256d c,
                                    __m256d d, __m256d e, __m256d f,
                                    __m256d g, __m256d h, __m256d i)
{
    auto[x1, x2] = twoSum(a, d);
    auto[y1, y2] = twoSum(b, c);
    auto[z1, z2] = twoSum(e, h);
    auto[u1, u2] = twoSum(f, g);

    auto[t1, t2] = addDD({ x1, x2 }, { y1, y2 });
    auto[t3, t4] = addDD({ z1, z2 }, { u1, u2 });

    auto[t5, t6] = addDD({ t1, t2 }, { t3, t4 });

    return threeTwoSum(t5, t6, i);
}


inline VecTriple nineThreeSum(__m256d a, __m256d b, __m256d c,
                              __m256d d, __m256d e, __m256d f,
                              __m256d g, __m256d h, __m256d i)
{
    auto[a1, a2, a3] = threeSum(a, b, c);
    auto[b1, b2, b3] = threeSum(d, e, f);
    auto[c1, c2, c3] = threeSum(g, h, i);

    auto[r1, t1, t2] = threeSum(a1, b1, c1);
    auto[r2, t3, t4, t5] = fourSum(a2, b2, c2, t1);
    auto r3 = _mm256_add_pd(_mm256_add_pd(
            _mm256_add_pd(_mm256_add_pd(a3, b3), _mm256_add_pd(c3, t2)),
            _mm256_add_pd(t3, t4)), t5);
    return { r1, r2, r3 };
}

static inline VecQuadruple renormalize(__m256d x0, __m256d x1, __m256d x2, __m256d x3, __m256d x4)
{
    auto [st1, t4] = quickTwoSum(x3, x4);
    auto [st2, t3] = quickTwoSum(x2, st1);
    auto [st3, t2] = quickTwoSum(x1, st2);
    auto [t0, t1] = quickTwoSum(x0, st3);

    __m256d s = t0;
    __m256d e;

    __m256d t[] = { t1, t2, t3, t4 };
    __m256d b[4] = { 0, 0, 0, 0 };

    int k = 0;
    for (int i = 0; i < 4; i++) {
        auto[st, et] = quickTwoSum(s, t[i]);
        s = st; e = et;
        b[k] = s;
        //if (e != 0) {
            b[k] = s;
            s = e;
            k = k + 1;
        //}
    }

    return { b[0], b[1], b[2], b[3] };
}


static inline VecQuadruple renorm1(__m256d x0, __m256d x1, __m256d x2, __m256d x3, __m256d x4)
{
    auto [r0, t0] = quickTwoSum(x0, x1);
    auto [r1, t1] = quickTwoSum(t0, x2);
    auto [r2, t2] = quickTwoSum(t1, x3);
    auto r3 = _mm256_add_pd(t2, x4);

    return { r0, r1, r2, r3 };
}


static inline VecQuadruple renorm2(__m256d x0, __m256d x1, __m256d x2, __m256d x3, __m256d x4)
{
    auto [st1, t4] = quickTwoSum(x3, x4);
    auto [st2, t3] = quickTwoSum(x2, st1);
    auto [st3, t2] = quickTwoSum(x1, st2);
    auto [t0, t1] = quickTwoSum(x0, st3);

    __m256d e = t0;

    auto [r0, e1] = quickTwoSum(e, t1);
    auto [r1, e2] = quickTwoSum(e1, t2);
    auto [r2, e3] = quickTwoSum(e2, t3);
    auto r3 = _mm256_add_pd(e3, t4);
    return { r0, r1, r2, r3 };
}

static inline VecPair twoProd(__m256d a, __m256d b)
{
    __m256d p = _mm256_mul_pd(a, b);
    __m256d e = _mm256_fmsub_pd(a, b, p);
    return { p, e };
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

    inline AvxDoubleDouble sq(void) const
    {
        auto[p1, p2] = twoProd(x[0], x[0]);
        __m256d x01 = _mm256_mul_pd(x[1], x[0]);
        p2 = _mm256_add_pd(p2, _mm256_add_pd(x01, x01));
        auto[r1, r2] = quickTwoSum(p1, p2);
        return AvxDoubleDouble{ r1, r2 };
    }

    inline AvxDoubleDouble mul_pow2(double v) const
    {
        __m256d vv = _mm256_set1_pd(v);
        return { _mm256_mul_pd(vv, x[0]), _mm256_mul_pd(vv, x[1]) };
    }
};


struct AvxQuadDouble
{
    __m256d x[4];

    inline AvxQuadDouble(__m256d a, __m256d b, __m256d c, __m256d d) :
        x{ a, b, c, d}
    {}

    inline AvxQuadDouble(double a, double b, double c, double d) :
        x{ _mm256_set1_pd(a), _mm256_set1_pd(b), _mm256_set1_pd(c), _mm256_set1_pd(d) }
    {}


    inline AvxQuadDouble operator + (const AvxQuadDouble& sm) const
    {
        auto[s0, e0] = twoSum(x[0], sm.x[0]);
        auto[s1, e1] = twoSum(x[1], sm.x[1]);
        auto[s2, e2] = twoSum(x[2], sm.x[2]);
        auto[s3, e3] = twoSum(x[3], sm.x[3]);
        __m256d r0 = s0;

        auto [r1, t0] = twoSum(s1, e0);
        auto [r2, t1, t2] = threeSum(s2, e1, t0);
        auto [r3, t3, _t4] = threeSum(s3, e2, t1);
        auto [r4, _t5, _t6] = threeSum(e3, t3, t2);

        auto [re0, re1, re2, re3] = renorm1(r0, r1, r2, r3, r4);
        return { re0, re1, re2, re3 };
    }

    inline AvxQuadDouble operator - (const AvxQuadDouble& sm) const
    {
        auto[s0, e0] = twoDiff(x[0], sm.x[0]);
        auto[s1, e1] = twoDiff(x[1], sm.x[1]);
        auto[s2, e2] = twoDiff(x[2], sm.x[2]);
        auto[s3, e3] = twoDiff(x[3], sm.x[3]);
        __m256d r0 = s0;

        auto [r1, t0] = twoSum(s1, e0);
        auto [r2, t1, t2] = threeSum(s2, e1, t0);
        auto [r3, t3, _t4] = threeSum(s3, e2, t1);
        auto [r4, _t5, _t6] = threeSum(e3, t3, t2);

        auto [re0, re1, re2, re3] = renorm1(r0, r1, r2, r3, r4);
        return { re0, re1, re2, re3 };
    }

    inline AvxQuadDouble operator * (const AvxQuadDouble& sm) const
    {
        auto[a0, b0] = twoProd(x[0], sm.x[0]);
        auto[b1, c0] = twoProd(x[0], sm.x[1]);
        auto[b2, c1] = twoProd(x[1], sm.x[0]);
        auto[c2, d0] = twoProd(x[0], sm.x[2]);
        auto[c3, d1] = twoProd(x[1], sm.x[1]);
        auto[c4, d2] = twoProd(x[2], sm.x[0]);
        auto d5 = _mm256_mul_pd(x[3], sm.x[0]);
        auto d6 = _mm256_mul_pd(x[2], sm.x[1]);
        auto d7 = _mm256_mul_pd(x[1], sm.x[2]);
        auto d8 = _mm256_mul_pd(x[0], sm.x[3]);

        auto r0 = a0;
        auto[r1, c5, d3] = threeSum(b0, b1, b2);
        auto[r2, d4, e0] = sixThreeSum(c0, c1, c2, c3, c4, c5);
        auto[r3, e1] = nineTwoSum(d0, d1, d2, d3, d4, d5, d6, d7, d8);
        auto r4 = _mm256_add_pd(e0, e1);

        auto [n0, n1, n2, n3] = renorm2(r0, r1, r2, r3, r4);

        return { n0, n1, n2, n3 };
    }

    inline AvxQuadDouble mul_pow2(double v) const
    {
        __m256d vv = _mm256_set1_pd(v);
        return { _mm256_mul_pd(vv, x[0]), _mm256_mul_pd(vv, x[1]),
                 _mm256_mul_pd(vv, x[2]), _mm256_mul_pd(vv, x[3]) };
    }

    inline AvxQuadDouble sq(void) const
    {
        auto[a0, b0] = twoProd(x[0], x[0]);
        auto[b1, c0] = twoProd(x[0], x[1]);
        //auto[b2, c1] = twoProd(x[0], x[1]); //
        auto[c2, d0] = twoProd(x[0], x[2]);
        auto[c3, d1] = twoProd(x[1], x[1]);
        //auto[c4, d2] = twoProd(x[0], x[2]); //
        auto d5 = _mm256_mul_pd(x[3], x[0]);
        auto d6 = _mm256_mul_pd(x[2], x[1]);
        //auto d7 = _mm256_mul_pd(x[1], x[2]); //
        //auto d8 = _mm256_mul_pd(x[0], x[3]); //

        auto r0 = a0;
        auto[r1, c5] = twoSum(b0, _mm256_add_pd(b1, b1)); // d3
        auto[r2, d4, e0] = sixThreeSum(_mm256_add_pd(c0, c0), /*c0*/ _mm256_set1_pd(0.0), c2, c3, c2, c5);
        auto[r3, e1] = sixTwoSum(d0, d1, d0, d4, _mm256_add_pd(d5, d5), _mm256_add_pd(d6, d6));
        auto r4 = _mm256_add_pd(e0, e1);

        auto [n0, n1, n2, n3] = renorm2(r0, r1, r2, r3, r4);

        return { n0, n1, n2, n3 };
    }
};


struct AvxHexDouble
{
    __m256d x[6];

    inline AvxHexDouble(__m256d a, __m256d b, __m256d c, __m256d d, __m256d e, __m256d f) :
        x{ a, b, c, d, e, f }
    {}

    inline AvxHexDouble(double a, double b, double c, double d, double e, double f) :
        x{ _mm256_set1_pd(a), _mm256_set1_pd(b), _mm256_set1_pd(c),
           _mm256_set1_pd(d), _mm256_set1_pd(e), _mm256_set1_pd(f) }
    {}


    inline AvxHexDouble operator + (const AvxHexDouble& sm) const
    {
        auto[a0, a1] = twoSum(x[0], sm.x[0]);
        auto[b0, b1] = twoSum(x[1], sm.x[1]);
        auto[c0, c1] = twoSum(x[2], sm.x[2]);
        auto[d0, d1] = twoSum(x[3], sm.x[3]);
        auto[e0, e1] = twoSum(x[4], sm.x[4]);

        auto t0 = a0;
        auto [t1, p1] = twoSum(a1, b0);
        auto [t2, p2, p3] = threeSum(b1, c0, p1);
        auto [t3, p4, p5, p6] = fourSum(c1, d0, p2, p3);
        auto [t4, p7] = fiveTwoSum(d1, e0, p4, p5, p6);
        auto t5 = _mm256_add_pd(_mm256_add_pd(x[5], sm.x[5]), _mm256_add_pd(e1, p7));

        auto[re0, er1] = quickTwoSum(t0, t1);
        auto[re1, e2] = quickTwoSum(er1, t2);
        auto[re2, e3] = quickTwoSum(e2, t3);
        auto[re3, e4] = quickTwoSum(e3, t4);
        auto[re4, re5] = quickTwoSum(e4, t5);

        return { re0, re1, re2, re3, re4, re5 };
    }

    inline AvxHexDouble operator - (const AvxHexDouble& sm) const
    {
        auto[a0, a1] = twoDiff(x[0], sm.x[0]);
        auto[b0, b1] = twoDiff(x[1], sm.x[1]);
        auto[c0, c1] = twoDiff(x[2], sm.x[2]);
        auto[d0, d1] = twoDiff(x[3], sm.x[3]);
        auto[e0, e1] = twoDiff(x[4], sm.x[4]);

        auto t0 = a0;
        auto [t1, p1] = twoSum(a1, b0);
        auto [t2, p2, p3] = threeSum(b1, c0, p1);
        auto [t3, p4, p5, p6] = fourSum(c1, d0, p2, p3);
        auto [t4, p7] = fiveTwoSum(d1, e0, p4, p5, p6);
        auto t5 = _mm256_add_pd(_mm256_add_pd(x[5], sm.x[5]), _mm256_add_pd(e1, p7));

        auto[re0, er1] = quickTwoSum(t0, t1);
        auto[re1, e2] = quickTwoSum(er1, t2);
        auto[re2, e3] = quickTwoSum(e2, t3);
        auto[re3, e4] = quickTwoSum(e3, t4);
        auto[re4, re5] = quickTwoSum(e4, t5);

        return { re0, re1, re2, re3, re4, re5 };
    }

    inline AvxHexDouble operator * (const AvxHexDouble& sm) const
    {
        auto[p1_0, p2_0] = twoProd(x[0], sm.x[0]);

        auto[p2_1, p3_0] = twoProd(x[0], sm.x[1]);
        auto[p2_2, p3_1] = twoProd(x[1], sm.x[0]);

        auto[p3_2, p4_0] = twoProd(x[2], sm.x[0]);
        auto[p3_3, p4_1] = twoProd(x[1], sm.x[1]);
        auto[p3_4, p4_2] = twoProd(x[0], sm.x[2]);

        auto[p4_3, p5_0] = twoProd(x[3], sm.x[0]);
        auto[p4_4, p5_1] = twoProd(x[2], sm.x[1]);
        auto[p4_5, p5_2] = twoProd(x[1], sm.x[2]);
        auto[p4_6, p5_3] = twoProd(x[0], sm.x[3]);

        auto[p5_4, p6_0] = twoProd(x[4], sm.x[0]);
        auto[p5_5, p6_1] = twoProd(x[3], sm.x[1]);
        auto[p5_6, p6_2] = twoProd(x[2], sm.x[2]);
        auto[p5_7, p6_3] = twoProd(x[1], sm.x[3]);
        auto[p5_8, p6_4] = twoProd(x[0], sm.x[4]);

        auto t1 = p1_0;
        auto[t2, tl3, tl4] = threeSum(p2_0, p2_1, p2_2);
        auto[t3, tl4_2, tl5] = sixThreeSum(p3_0, p3_1, p3_2, p3_3, p3_4, tl3);
        auto[t4, tl5_2, tl6] = nineThreeSum(p4_0, p4_1, p4_2, p4_3, p4_4, p4_5, p4_6, tl4, tl4_2);
        auto[x1, x2, x3] = nineThreeSum(p5_0, p5_1, p5_2, p5_3, p5_4, p5_5, p5_6, p5_7, p5_8);
        auto[t5, tl6_1, tl7] = sixThreeSum(x1, x2, x3, tl5, tl5_2, _mm256_set1_pd(0.0));

        auto t6 =
            _mm256_add_pd(
                _mm256_add_pd(
                    _mm256_add_pd(
                        _mm256_add_pd(tl6, tl6_1),
                        _mm256_add_pd(tl7, p6_0)
                    ),
                    _mm256_add_pd(
                        _mm256_add_pd(p6_1, p6_2),
                        _mm256_add_pd(p6_3, p6_4)
                    )
                ),
                _mm256_add_pd(
                    _mm256_fmadd_pd(x[5], sm.x[0], _mm256_fmadd_pd(x[4], sm.x[1], _mm256_mul_pd(x[3], sm.x[2]))),
                    _mm256_fmadd_pd(x[2], sm.x[3], _mm256_fmadd_pd(x[1], sm.x[4], _mm256_mul_pd(x[0], sm.x[5])))
                )
            );

        auto[re0, e1] = quickTwoSum(t1, t2);
        auto[re1, e2] = quickTwoSum(e1, t3);
        auto[re2, e3] = quickTwoSum(e2, t4);
        auto[re3, e4] = quickTwoSum(e3, t5);
        auto[re4, re5] = quickTwoSum(e4, t6);

        return { re0, re1, re2, re3, re4, re5 };
    }

    inline AvxHexDouble mul_pow2(double v) const
    {
        __m256d vv = _mm256_set1_pd(v);
        return { _mm256_mul_pd(vv, x[0]), _mm256_mul_pd(vv, x[1]),
                 _mm256_mul_pd(vv, x[2]), _mm256_mul_pd(vv, x[3]),
                 _mm256_mul_pd(vv, x[4]), _mm256_mul_pd(vv, x[5]) };
    }

    inline AvxHexDouble sq(void) const
    {
        return operator*(*this);
    }
};

} // namespace avxfma_private

void generateDoubleDoubleAvxFma(long width, long height, float* data, bool parallel,
    double vx1, double vx2, double vy1, double vy2, double vw1, double vw2, double vh1, double vh2, int maxIter, bool smooth,
    bool julia, double jX1, double jX2, double jY1, double jY2)
{
    using namespace avxfma_private;
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
        __m256d y0s = { y.x[0], y.x[0], y.x[0], y.x[0] };
        __m256d y1s = { y.x[1], y.x[1], y.x[1], y.x[1] };
        AvxDoubleDouble ys{ y0s, y1s };
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

            __m256d resultsa;
            __m256d resultsb;

            __m256d cmp = _mm256_cmp_pd(threshold, threshold, _CMP_LE_OQ);
            for (int k = 0; k < maxIter; k++) {
                AvxDoubleDouble aa = a.sq();
                AvxDoubleDouble bb = b.sq();
                AvxDoubleDouble abab = a * b.mul_pow2(2.0);
                a = aa - bb + cx;
                b = abab + cy;
                if (smooth) {
                    resultsa = _mm256_blendv_pd(resultsa, a.x[0], cmp);
                    resultsb = _mm256_blendv_pd(resultsb, b.x[0], cmp);
                }
                cmp = _mm256_cmp_pd(_mm256_add_pd(aa.x[0], bb.x[0]), threshold, _CMP_LE_OQ);
                adder = _mm256_and_pd(adder, cmp);
                counter = _mm256_add_pd(counter, adder);
                if ((k & 0x7) && _mm256_testz_si256(_mm256_castpd_si256(cmp), _mm256_castpd_si256(cmp)) != 0) {
                    break;
                }
            }

            double resData[12];
            double* ftRes = resData;
            double* resa = ftRes + 4;
            double* resb = ftRes + 8;
            _mm256_storeu_pd(ftRes, counter);
            _mm256_storeu_pd(resa, resultsa);
            _mm256_storeu_pd(resb, resultsb);

            for (int k = 0; k < 4 && i + k < width; k++) {
                if (smooth)
                    data[i + k + j * width] = ftRes[k] < 0 ? maxIter :
                        ftRes[k] >= maxIter ? maxIter :
                        ((float)ftRes[k]) + 1 - floatLog2(floatLog(resa[k] * resa[k] + resb[k] * resb[k]) / 2);
                else
                    data[i + k + j * width] = ftRes[k] >= 0 ? float(ftRes[k]) : maxIter;
            }
        }
    }
}


void generateQuadDoubleAvxFma(long width, long height, float* data, bool parallel,
    const double* vx, const double* vy,
    const double* vw, const double* vh,
    int maxIter, bool smooth, bool julia,
    const double* jXp, const double* jYp)
{
    using namespace avxfma_private;
    using T = mnd::QuadDouble;

    T viewx{ vx[0], vx[1], vx[2], vx[3] };
    T viewy{ vy[0], vy[1], vy[2], vy[3] };
    T wpp = T{ vw[0], vw[1], vw[2], vw[3] } * T(1.0 / width);
    T hpp = T{ vh[0], vh[1], vh[2], vh[3] } * T(1.0 / height);


    T jX{ jXp[0], jXp[1], jXp[2], jXp[3] };
    T jY{ jYp[0], jYp[1], jYp[2], jYp[3] };


    auto toAvxQuadDouble4 = [] (const T& a, const T& b,
            const T& c, const T& d) -> AvxQuadDouble {
        __m256d xs = { a[0], b[0], c[0], d[0] };
        __m256d ys = { a[1], b[1], c[1], d[1] };
        __m256d zs = { a[2], b[2], c[2], d[2] };
        __m256d us = { a[3], b[3], c[3], d[3] };

        return AvxQuadDouble{ xs, ys, zs, us };
    };

    AvxQuadDouble juliaX{ jX[0], jX[1], jX[2], jX[3] };
    AvxQuadDouble juliaY{ jY[0], jY[1], jY[2], jY[3] };

#if defined(_OPENMP)
    if (parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < height; j++) {
        T y = viewy + T(double(j)) * hpp;
        AvxQuadDouble ys{ y[0], y[1], y[2], y[3] };
        for (long i = 0; i < width; i += 4) {
            T x1 = viewx + T(double(i)) * wpp;
            T x2 = x1 + wpp;
            T x3 = x2 + wpp;
            T x4 = x3 + wpp;

            AvxQuadDouble xs = toAvxQuadDouble4(x1, x2, x3, x4);

            AvxQuadDouble cx = julia ? juliaX : xs;
            AvxQuadDouble cy = julia ? juliaY : ys;

            __m256d threshold = { 16.0, 16.0, 16.0, 16.0 };
            __m256d counter = { 0, 0, 0, 0 };
            __m256d adder = { 1, 1, 1, 1 };

            AvxQuadDouble a = xs;
            AvxQuadDouble b = ys;

            __m256d resultsa;
            __m256d resultsb;

            __m256d cmp = _mm256_cmp_pd(threshold, threshold, _CMP_LE_OQ);
            for (int k = 0; k < maxIter; k++) {
                AvxQuadDouble aa = a.sq();
                AvxQuadDouble bb = b.sq();
                AvxQuadDouble abab = a * b.mul_pow2(2.0);
                a = aa - bb + cx;
                b = abab + cy;
                if (smooth) {
                    resultsa = _mm256_blendv_pd(resultsa, a.x[0], cmp);
                    resultsb = _mm256_blendv_pd(resultsb, b.x[0], cmp);
                }
                cmp = _mm256_cmp_pd(_mm256_add_pd(aa.x[0], bb.x[0]), threshold, _CMP_LE_OQ);
                adder = _mm256_and_pd(adder, cmp);
                counter = _mm256_add_pd(counter, adder);
                if (_mm256_testz_si256(_mm256_castpd_si256(cmp), _mm256_castpd_si256(cmp)) != 0) {
                    break;
                }
            }

            double resData[12];
            double* ftRes = resData;
            double* resa = resData + 4;
            double* resb = resData + 8;
            _mm256_storeu_pd(ftRes, counter);
            _mm256_storeu_pd(resa, resultsa);
            _mm256_storeu_pd(resb, resultsb);

            for (int k = 0; k < 4 && i + k < width; k++) {
                if (smooth)
                    data[i + k + j * width] = ftRes[k] < 0 ? maxIter :
                        ftRes[k] >= maxIter ? maxIter :
                        ((float)ftRes[k]) + 1 - floatLog2(floatLog(resa[k] * resa[k] + resb[k] * resb[k]) / 2);
                else
                    data[i + k + j * width] = ftRes[k] >= 0 ? float(ftRes[k]) : maxIter;
            }
        }
    }
}


void generateHexDoubleAvxFma(long width, long height, float* data, bool parallel,
    const double* vx, const double* vy,
    const double* vw, const double* vh,
    int maxIter, bool smooth, bool julia,
    const double* jX, const double* jY)
{
    using namespace avxfma_private;
    using T = mnd::HexDouble;

    T viewx{ vx[0], vx[1], vx[2], vx[3], vx[4], vx[5] };
    T viewy{ vy[0], vy[1], vy[2], vy[3] , vy[4], vy[5] };
    T wpp = T{ vw[0], vw[1], vw[2], vw[3], vw[4], vw[5] } * T(1.0 / width);
    T hpp = T{ vh[0], vh[1], vh[2], vh[3], vh[4], vh[5] } * T(1.0 / height);

    auto toAvxHexDouble4 = [] (const T& a, const T& b,
            const T& c, const T& d) -> AvxHexDouble {
        __m256d xs = { a[0], b[0], c[0], d[0] };
        __m256d ys = { a[1], b[1], c[1], d[1] };
        __m256d zs = { a[2], b[2], c[2], d[2] };
        __m256d us = { a[3], b[3], c[3], d[3] };
        __m256d vs = { a[4], b[4], c[4], d[4] };
        __m256d ws = { a[5], b[5], c[5], d[5] };

        return AvxHexDouble{ xs, ys, zs, us, vs, ws };
    };

    AvxHexDouble juliaX{ jX[0], jX[1], jX[2], jX[3], jX[4], jX[5] };
    AvxHexDouble juliaY{ jY[0], jY[1], jY[2], jY[3], jY[4], jY[5] };

#if defined(_OPENMP)
    if (parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < height; j++) {
        T y = viewy + T(double(j)) * hpp;
        AvxHexDouble ys{ y[0], y[1], y[2], y[3], y[4], y[5] };
        for (long i = 0; i < width; i += 4) {
            T x1 = viewx + T(double(i)) * wpp;
            T x2 = x1 + wpp;
            T x3 = x2 + wpp;
            T x4 = x3 + wpp;

            AvxHexDouble xs = toAvxHexDouble4(x1, x2, x3, x4);

            AvxHexDouble cx = julia ? juliaX : xs;
            AvxHexDouble cy = julia ? juliaY : ys;

            __m256d threshold = { 16.0, 16.0, 16.0, 16.0 };
            __m256d counter = { 0, 0, 0, 0 };
            __m256d adder = { 1, 1, 1, 1 };

            AvxHexDouble a = xs;
            AvxHexDouble b = ys;

            __m256d resultsa;
            __m256d resultsb;

            __m256d cmp = _mm256_cmp_pd(threshold, threshold, _CMP_LE_OQ);
            for (int k = 0; k < maxIter; k++) {
                AvxHexDouble aa = a.sq();
                AvxHexDouble bb = b.sq();
                AvxHexDouble abab = a * b.mul_pow2(2.0);
                a = aa - bb + cx;
                b = abab + cy;
                if (smooth) {
                    resultsa = _mm256_blendv_pd(resultsa, a.x[0], cmp);
                    resultsb = _mm256_blendv_pd(resultsb, b.x[0], cmp);
                }
                cmp = _mm256_cmp_pd(_mm256_add_pd(aa.x[0], bb.x[0]), threshold, _CMP_LE_OQ);
                adder = _mm256_and_pd(adder, cmp);
                counter = _mm256_add_pd(counter, adder);
                if (_mm256_testz_si256(_mm256_castpd_si256(cmp), _mm256_castpd_si256(cmp)) != 0) {
                    break;
                }
            }

            double resData[12];
            double* ftRes = resData;
            double* resa = resData + 4;
            double* resb = resData + 8;
            _mm256_storeu_pd(ftRes, counter);
            _mm256_storeu_pd(resa, resultsa);
            _mm256_storeu_pd(resb, resultsb);

            for (int k = 0; k < 4 && i + k < width; k++) {
                if (smooth)
                    data[i + k + j * width] = ftRes[k] < 0 ? maxIter :
                        ftRes[k] >= maxIter ? maxIter :
                        ((float)ftRes[k]) + 1 - floatLog2(floatLog(resa[k] * resa[k] + resb[k] * resb[k]) / 2);
                else
                    data[i + k + j * width] = ftRes[k] >= 0 ? float(ftRes[k]) : maxIter;
            }
        }
    }
}

