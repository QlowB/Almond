#include "CpuGenerators.h"

#include <immintrin.h>
#include <omp.h>
#include <cmath>

#include <utility>
#include <memory>

using mnd::CpuGenerator;

namespace mnd
{
    template class CpuGenerator<float, mnd::X86_AVX_FMA, false>;
    template class CpuGenerator<float, mnd::X86_AVX_FMA, true>;

    template class CpuGenerator<double, mnd::X86_AVX_FMA, false>;
    template class CpuGenerator<double, mnd::X86_AVX_FMA, true>;

    template class CpuGenerator<DoubleDouble, mnd::X86_AVX_FMA, false>;
    template class CpuGenerator<DoubleDouble, mnd::X86_AVX_FMA, true>;

    template class CpuGenerator<QuadDouble, mnd::X86_AVX_FMA, false>;
    template class CpuGenerator<QuadDouble, mnd::X86_AVX_FMA, true>;
}


template<bool parallel>
void CpuGenerator<float, mnd::X86_AVX_FMA, parallel>::generate(const mnd::MandelInfo& info, float* data)
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

#if defined(_OPENMP)
    if constexpr(parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y) + T(j) * T(view.height / info.bHeight);
        __m256 ys = {y, y, y, y, y, y, y, y};
        for (long i = 0; i < info.bWidth; i += 24) {
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

            __m256 cx = info.julia ? juliaX : xs;
            __m256 cx2 = info.julia ? juliaX : xs2;
            __m256 cx3 = info.julia ? juliaX : xs3;
            __m256 cy = info.julia ? juliaY : ys;

            if (info.smooth) {
                __m256 cmp = _mm256_cmp_ps(threshold, threshold, _CMP_LE_OQ);
                __m256 cmp2 = _mm256_cmp_ps(threshold, threshold, _CMP_LE_OQ);
                __m256 cmp3 = _mm256_cmp_ps(threshold, threshold, _CMP_LE_OQ);
                for (int k = 0; k < info.maxIter; k++) {
                    __m256 bb = _mm256_mul_ps(b, b);
                    __m256 bb2 = _mm256_mul_ps(b2, b2);
                    __m256 bb3 = _mm256_mul_ps(b3, b3);
                    __m256 ab = _mm256_mul_ps(a, b);
                    __m256 ab2 = _mm256_mul_ps(a2, b2);
                    __m256 ab3 = _mm256_mul_ps(a3, b3);
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
                    cmp = _mm256_cmp_ps(_mm256_fmadd_ps(a, a, bb), threshold, _CMP_LE_OQ);
                    cmp2 = _mm256_cmp_ps(_mm256_fmadd_ps(a2, a2, bb2), threshold, _CMP_LE_OQ);
                    cmp3 = _mm256_cmp_ps(_mm256_fmadd_ps(a3, a3, bb3), threshold, _CMP_LE_OQ);
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
                for (int k = 0; k < info.maxIter; k++) {
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


            auto alignVec = [](float* data) -> float* {
                void* aligned = data;
                ::size_t length = 64;
                std::align(32, 8 * sizeof(float), aligned, length);
                return static_cast<float*>(aligned);
            };

            float resData[96];
            float* ftRes = alignVec(resData);
            float* resa = ftRes + 24;
            float* resb = resa + 24;

            _mm256_store_ps(ftRes, counter);
            _mm256_store_ps(ftRes + 8, counter2);
            _mm256_store_ps(ftRes + 16, counter3);
            _mm256_store_ps(resa, resultsa);
            _mm256_store_ps(resa + 8, resultsa2);
            _mm256_store_ps(resa + 16, resultsa3);
            _mm256_store_ps(resb, resultsb);
            _mm256_store_ps(resb + 8, resultsb2);
            _mm256_store_ps(resb + 16, resultsb3);
            for (int k = 0; k < 24 && i + k < info.bWidth; k++) {
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
void CpuGenerator<double, mnd::X86_AVX_FMA, parallel>::generate(const mnd::MandelInfo& info, float* data)
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


#if defined(_OPENMP)
    if constexpr(parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y + T(j) * view.height / info.bHeight);
        __m256d ys = { y, y, y, y };
        for (long i = 0; i < info.bWidth; i += 8) {
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

            __m256d cx = info.julia ? juliaX : xs;
            __m256d cy = info.julia ? juliaY : ys;
            __m256d cx2 = info.julia ? juliaX : xs2;
            //__m256d cy2 = info.julia ? juliaY : ys;

            __m256d cmp = _mm256_cmp_pd(threshold, threshold, _CMP_LE_OQ);
            __m256d cmp2 = _mm256_cmp_pd(threshold, threshold, _CMP_LE_OQ);
            for (int k = 0; k < info.maxIter; k++) {
                __m256d ab = _mm256_mul_pd(a, b);
                __m256d bb = _mm256_mul_pd(b, b);
                __m256d ab2 = _mm256_mul_pd(a2, b2);
                __m256d bb2 = _mm256_mul_pd(b2, b2);
                a = _mm256_fmsub_pd(a, a, bb);
                a = _mm256_add_pd(a, cx);
                a2 = _mm256_fmsub_pd(a2, a2, bb2);
                a2 = _mm256_add_pd(a2, cx2);
                b = _mm256_fmadd_pd(two, ab, cy);
                b2 = _mm256_fmadd_pd(two, ab2, cy);
                if (info.smooth) {
                    resultsa = _mm256_blendv_pd(resultsa, a, cmp);
                    resultsb = _mm256_blendv_pd(resultsb, b, cmp);
                    resultsa2 = _mm256_blendv_pd(resultsa2, a2, cmp2);
                    resultsb2 = _mm256_blendv_pd(resultsb2, b2, cmp2);
                }
                cmp = _mm256_cmp_pd(_mm256_fmadd_pd(a, a, bb), threshold, _CMP_LE_OQ);
                cmp2 = _mm256_cmp_pd(_mm256_fmadd_pd(a2, a2, bb2), threshold, _CMP_LE_OQ);
                adder = _mm256_and_pd(adder, cmp);
                adder2 = _mm256_and_pd(adder2, cmp2);
                counter = _mm256_add_pd(counter, adder);
                counter2 = _mm256_add_pd(counter2, adder2);
                if ((k & 0x7) == 0 && _mm256_testz_si256(_mm256_castpd_si256(cmp), _mm256_castpd_si256(cmp)) != 0 &&
                        _mm256_testz_si256(_mm256_castpd_si256(cmp2), _mm256_castpd_si256(cmp2)) != 0) {
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
                    data[i + k + j * info.bWidth] = ftRes[k] < 0 ? info.maxIter :
                        ftRes[k] >= info.maxIter ? info.maxIter :
                        ((float)ftRes[k]) + 1 - ::log(::log(resa[k] * resa[k] + resb[k] * resb[k]) / 2) / ::log(2.0f);
                else
                    data[i + k + j * info.bWidth] = ftRes[k] < 0 ? info.maxIter : float(ftRes[k]);
            }

            resa = (double*) &resultsa2;
            resb = (double*) &resultsb2;
            _mm256_store_pd(ftRes, counter2);
            i += 4;
            for (int k = 0; k < 4 && i + k < info.bWidth; k++) {
                if (info.smooth)
                    data[i + k + j * info.bWidth] = ftRes[k] < 0 ? info.maxIter :
                        ftRes[k] >= info.maxIter ? info.maxIter :
                        ((float)ftRes[k]) + 1 - ::log(::log(resa[k] * resa[k] + resb[k] * resb[k]) / 2) / ::log(2.0f);
                else
                    data[i + k + j * info.bWidth] = ftRes[k] < 0 ? info.maxIter : float(ftRes[k]);
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
};


template<bool parallel>
void CpuGenerator<mnd::DoubleDouble, mnd::X86_AVX_FMA, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    const MandelViewport& view = info.view;

    using T = LightDoubleDouble;

    T viewx = mnd::convert<T>(view.x);
    T viewy = mnd::convert<T>(view.y);
    T wpp = mnd::convert<T>(view.width / info.bWidth);
    T hpp = mnd::convert<T>(view.height / info.bHeight);


    T jX = mnd::convert<T>(info.juliaX);
    T jY = mnd::convert<T>(info.juliaY);

    AvxDoubleDouble juliaX = { jX[0], jX[1] };
    AvxDoubleDouble juliaY = { jY[0], jY[1] };

#if defined(_OPENMP)
    if constexpr(parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
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
                x1[0], x2[0], x3[0], x4[0],
            };

            __m256d x1s = {
                x1[1], x2[1], x3[1], x4[1],
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

            __m256d cmp = _mm256_cmp_pd(threshold, threshold, _CMP_LE_OQ);
            for (int k = 0; k < info.maxIter; k++) {
                AvxDoubleDouble aa = a * a;
                AvxDoubleDouble bb = b * b;
                AvxDoubleDouble abab = a * b; abab = abab + abab;
                a = aa - bb + cx;
                b = abab + cy;
                if (info.smooth) {
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
                    data[i + k + j * info.bWidth] = ftRes[k] < 0 ? info.maxIter :
                        ftRes[k] >= info.maxIter ? info.maxIter :
                        ((float)ftRes[k]) + 1 - ::log(::log(resa[k] * resa[k] + resb[k] * resb[k]) / 2) / ::log(2.0f);
                else
                    data[i + k + j * info.bWidth] = ftRes[k] >= 0 ? float(ftRes[k]) : info.maxIter;
            }
        }
    }
}

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

        auto [re0, re1, re2, re3] = renormalize(r0, r1, r2, r3, r4);
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

        auto [re0, re1, re2, re3] = renormalize(r0, r1, r2, r3, r4);
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

        auto [n0, n1, n2, n3] = renormalize(r0, r1, r2, r3, r4);

        return { n0, n1, n2, n3 };
    }
};


template<bool parallel>
void CpuGenerator<mnd::QuadDouble, mnd::X86_AVX_FMA, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    const MandelViewport& view = info.view;

    using T = mnd::Float256;

    T viewx = mnd::convert<T>(view.x);
    T viewy = mnd::convert<T>(view.y);
    T wpp = mnd::convert<T>(view.width / info.bWidth);
    T hpp = mnd::convert<T>(view.height / info.bHeight);


    T jX = mnd::convert<T>(info.juliaX);
    T jY = mnd::convert<T>(info.juliaY);


    auto toQd = [] (const mnd::Float256& x) -> std::tuple<double, double, double, double> {
        double a = double(x);
        mnd::Float256 rem = x - a;
        double b = double(rem);
        rem = rem - b;
        double c = double(rem);
        rem = rem - c;
        double d = double(rem);
        return { a, b, c, d };
    };

    auto toAvxQuadDouble = [&toQd] (const mnd::Float256& x) -> AvxQuadDouble {
        auto [a, b, c, d] = toQd(x);
        return AvxQuadDouble{ a, b, c, d };
    };

    auto toAvxQuadDouble4 = [&toQd] (const mnd::Float256& a, const mnd::Float256& b,
                            const mnd::Float256& c, const mnd::Float256& d) -> AvxQuadDouble {
        auto [x0, y0, z0, u0] = toQd(a);
        auto [x1, y1, z1, u1] = toQd(b);
        auto [x2, y2, z2, u2] = toQd(c);
        auto [x3, y3, z3, u3] = toQd(d);

        __m256d xs = { x0, x1, x2, x3 };
        __m256d ys = { y0, y1, y2, y3 };
        __m256d zs = { z0, z1, z2, z3 };
        __m256d us = { u0, u1, u2, u3 };

        return AvxQuadDouble{ xs, ys, zs, us };
    };

    AvxQuadDouble juliaX = toAvxQuadDouble(jX);
    AvxQuadDouble juliaY = toAvxQuadDouble(jY);

#if defined(_OPENMP)
    if constexpr(parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < info.bHeight; j++) {
        T y = viewy + T(double(j)) * hpp;
        AvxQuadDouble ys = toAvxQuadDouble(y);
        for (long i = 0; i < info.bWidth; i += 4) {
            T x1 = viewx + T(double(i)) * wpp;
            T x2 = x1 + wpp;
            T x3 = x2 + wpp;
            T x4 = x3 + wpp;

            AvxQuadDouble xs = toAvxQuadDouble4(x1, x2, x3, x4);

            AvxQuadDouble cx = info.julia ? juliaX : xs;
            AvxQuadDouble cy = info.julia ? juliaY : ys;

            int itRes[4] = { 0, 0, 0, 0 };

            __m256d threshold = { 16.0, 16.0, 16.0, 16.0 };
            __m256d counter = { 0, 0, 0, 0 };
            __m256d adder = { 1, 1, 1, 1 };

            AvxQuadDouble a = xs;
            AvxQuadDouble b = ys;

            __m256d resultsa;
            __m256d resultsb;

            __m256d cmp = _mm256_cmp_pd(threshold, threshold, _CMP_LE_OQ);
            for (int k = 0; k < info.maxIter; k++) {
                AvxQuadDouble aa = a * a;
                AvxQuadDouble bb = b * b;
                AvxQuadDouble abab = a * b; abab = abab + abab;
                a = aa - bb + cx;
                b = abab + cy;
                if (info.smooth) {
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
                    data[i + k + j * info.bWidth] = ftRes[k] < 0 ? info.maxIter :
                        ftRes[k] >= info.maxIter ? info.maxIter :
                        ((float)ftRes[k]) + 1 - ::log(::log(resa[k] * resa[k] + resb[k] * resb[k]) / 2) / ::log(2.0f);
                else
                    data[i + k + j * info.bWidth] = ftRes[k] >= 0 ? float(ftRes[k]) : info.maxIter;
            }
        }
    }
}
