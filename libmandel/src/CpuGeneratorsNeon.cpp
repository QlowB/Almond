#include "CpuGenerators.h"
#include "LightDoubleDouble.h"

#include <omp.h>
#include <arm_neon.h>
#include <memory>

using mnd::CpuGenerator;

namespace mnd
{
    template class CpuGenerator<float, mnd::ARM_NEON, false>;
    template class CpuGenerator<float, mnd::ARM_NEON, true>;

    template class CpuGenerator<double, mnd::ARM_NEON, false>;
    template class CpuGenerator<double, mnd::ARM_NEON, true>;

    template class CpuGenerator<mnd::DoubleDouble, mnd::ARM_NEON, false>;
    template class CpuGenerator<mnd::DoubleDouble, mnd::ARM_NEON, true>;
}


template<bool parallel>
void CpuGenerator<float, mnd::ARM_NEON, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = float;
    const MandelViewport& view = info.view;

    float32x4_t juliaX = vmovq_n_f32(double(info.juliaX));
    float32x4_t juliaY = vmovq_n_f32(double(info.juliaY));
#if defined(_OPENMP)
    if constexpr(parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y) + T(j) * T(view.height / info.bHeight);
        for (long i = 0; i < info.bWidth; i += 4) {
            float xsvals[] = {
                float(view.x + float(i) * view.width / info.bWidth),
                float(view.x + float(i + 1) * view.width / info.bWidth),
                float(view.x + float(i + 2) * view.width / info.bWidth),
                float(view.x + float(i + 3) * view.width / info.bWidth)
            };

            float32x4_t xs = vld1q_f32(xsvals);


            uint32x4_t counter = vmovq_n_u32(0);
            uint32x4_t adder = vmovq_n_u32(1);
            //uint32x4_t ones = vmovq_n_u32(1);

            float32x4_t threshold = vmovq_n_f32(16);

            float32x4_t ys = vmovq_n_f32(y);
            float32x4_t a = xs;
            float32x4_t b = ys;

            float32x4_t cx = info.julia ? juliaX : xs;
            float32x4_t cy = info.julia ? juliaY : ys;

            for (int k = 0; k < info.maxIter; k++) {
                float32x4_t aa = vmulq_f32(a, a);
                float32x4_t bb = vmulq_f32(b, b);
                float32x4_t abab = vmulq_f32(a, b); abab = vaddq_f32(abab, abab);
                uint32x4_t cmp = vcleq_f32(vaddq_f32(aa, bb), threshold);
                if (info.smooth) {
                    float32x4_t tempa = vaddq_f32(vsubq_f32(aa, bb), cx);
                    float32x4_t tempb = vaddq_f32(abab, cy);
                    a = vreinterpretq_f32_u32(vorrq_u32(vandq_u32(cmp, vreinterpretq_u32_f32(tempa)), vandq_u32(vmvnq_u32(cmp), vreinterpretq_u32_f32(a))));
                    b = vreinterpretq_f32_u32(vorrq_u32(vandq_u32(cmp, vreinterpretq_u32_f32(tempb)), vandq_u32(vmvnq_u32(cmp), vreinterpretq_u32_f32(b))));
                }
                else {
                    a = vaddq_f32(vsubq_f32(aa, bb), cx);
                    b = vaddq_f32(abab, cy);
                }
                adder = vandq_u32(adder, cmp);
                counter = vaddq_u32(counter, adder);
                // checking for break criterion is possibly expensive, only do it every 8 iterations
                if ((k & 0x7) == 0) {
                    /* // ARM-v7 method
                    uint32x2_t allZero = vorr_u32(vget_low_u32(cmp), vget_high_u32(cmp));
                    if (vget_lane_u32(vpmax_u32(allZero, allZero), 0) == 0) {
                        break;
                    }
                    */
                    uint32_t allZero = vaddvq_u32(cmp);
                    if (allZero == 0) {
                        break;
                    }
                }
            }

            uint32_t resData[4];
            float resa[4];
            float resb[4];
            vst1q_u32(resData, counter);
            vst1q_f32(resa, a);
            vst1q_f32(resb, b);

            for (int k = 0; k < 4 && i + k < info.bWidth; k++) {
                if (info.smooth)
                    data[i + k + j * info.bWidth] = resData[k] <= 0 ? info.maxIter :
                    resData[k] >= info.maxIter ? info.maxIter :
                    ((float) resData[k]) + 1 - ::logf(::logf(resa[k] * resa[k] + resb[k] * resb[k]) / 2) / ::logf(2.0f);
                else
                    data[i + k + j * info.bWidth] = resData[k] > 0 ? float(resData[k]) : info.maxIter;
            }
        }
    }
}


template<bool parallel>
void CpuGenerator<double, mnd::ARM_NEON, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = double;
    const MandelViewport& view = info.view;

    float64x2_t juliaX = vmovq_n_f64(double(info.juliaX));
    float64x2_t juliaY = vmovq_n_f64(double(info.juliaY));

#if defined(_OPENMP)
    if constexpr(parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y) + T(j) * T(view.height / info.bHeight);
        for (long i = 0; i < info.bWidth; i += 2) {
            double xsvals[] = {
                double(view.x + double(i) * view.width / info.bWidth),
                double(view.x + double(i + 1) * view.width / info.bWidth),
            };

            float64x2_t xs = vld1q_f64(xsvals);


            uint64x2_t counter = vmovq_n_u64(0);
            uint64x2_t adder = vmovq_n_u64(1);
            //uint32x4_t ones = vmovq_n_u32(1);

            float64x2_t threshold = vmovq_n_f64(16);

            float64x2_t ys = vmovq_n_f64(y);
            float64x2_t a = xs;
            float64x2_t b = ys;
            float64x2_t cx = info.julia ? juliaX : xs;
            float64x2_t cy = info.julia ? juliaY : ys;

            for (int k = 0; k < info.maxIter; k++) {
                float64x2_t aa = vmulq_f64(a, a);
                float64x2_t bb = vmulq_f64(b, b);
                float64x2_t abab = vmulq_f64(a, b); abab = vaddq_f64(abab, abab);
                //a = vaddq_f64(vsubq_f64(aa, bb), xs);
                //b = vaddq_f64(abab, ys);
                uint64x2_t cmp = vcleq_f64(vaddq_f64(aa, bb), threshold);
                if (info.smooth) {
                    float64x2_t tempa = vaddq_f64(vsubq_f64(aa, bb), cx);
                    float64x2_t tempb = vaddq_f64(abab, cy);
                    a = vreinterpretq_f64_u64(vorrq_u64(vandq_u64(cmp, vreinterpretq_u64_f64(tempa)), vandq_u64(vreinterpretq_u64_u32(vmvnq_u32(vreinterpretq_u32_u64(cmp))), vreinterpretq_u64_f64(a))));
                    b = vreinterpretq_f64_u64(vorrq_u64(vandq_u64(cmp, vreinterpretq_u64_f64(tempb)), vandq_u64(vreinterpretq_u64_u32(vmvnq_u32(vreinterpretq_u32_u64(cmp))), vreinterpretq_u64_f64(b))));
                }
                else {
                    a = vaddq_f64(vsubq_f64(aa, bb), cx);
                    b = vaddq_f64(abab, cy);
                }
                adder = vandq_u64(adder, cmp);
                counter = vaddq_u64(counter, adder);
                // checking for break criterion is possibly expensive, only do it every 8 iterations
                if ((k & 0x7) == 0) {
                    /* // ARM-v7 method
                    uint32x2_t allZero = vorr_u32(vget_low_u32(cmp), vget_high_u32(cmp));
                    if (vget_lane_u32(vpmax_u32(allZero, allZero), 0) == 0) {
                        break;
                    }
                    */
                    uint64_t allZero = vaddvq_u64(cmp);
                    if (allZero == 0) {
                        break;
                    }
                }
            }

            uint64_t resData[2];
            double resa[2];
            double resb[2];
            vst1q_u64(resData, counter);
            vst1q_f64(resa, a);
            vst1q_f64(resb, b);

            for (int k = 0; k < 2 && i + k < info.bWidth; k++) {
                if (info.smooth)
                    data[i + k + j * info.bWidth] = resData[k] <= 0 ? info.maxIter :
                    resData[k] >= info.maxIter ? info.maxIter :
                    ((float) resData[k]) + 1 - ::logf(::logf(resa[k] * resa[k] + resb[k] * resb[k]) / 2) / ::logf(2.0f);
                else
                    data[i + k + j * info.bWidth] = resData[k] > 0 ? float(resData[k]) : info.maxIter;
            }
        }
    }
}



struct VecPair
{
    float64x2_t a;
    float64x2_t b;
};


static inline VecPair quickTwoSum(float64x2_t a, float64x2_t b)
{
    float64x2_t s = vaddq_f64(a, b);
    float64x2_t e = vsubq_f64(b, vsubq_f64(s, a));
    return { s, e };
}

static inline VecPair quickTwoDiff(float64x2_t a, float64x2_t b)
{
    float64x2_t s = vsubq_f64(a, b);
    float64x2_t e = vsubq_f64(vsubq_f64(a, s), b);
    return { s, e };
}

static inline VecPair twoSum(float64x2_t a, float64x2_t b)
{
    float64x2_t s = vaddq_f64(a, b);
    float64x2_t bb = vsubq_f64(s, a);
    float64x2_t e = vaddq_f64(vsubq_f64(a, vsubq_f64(s, bb)), vsubq_f64(b, bb));
    return { s, e };
}

static inline VecPair twoDiff(float64x2_t a, float64x2_t b)
{
    float64x2_t s = vsubq_f64(a, b);
    float64x2_t bb = vsubq_f64(s, a);
    float64x2_t e = vsubq_f64(vsubq_f64(a, vsubq_f64(s, bb)), vaddq_f64(b, bb));
    return { s, e };
}


static inline VecPair split(float64x2_t a)
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

    static const float64x2_t SPLITTER = vmovq_n_f64(134217729.0);
    float64x2_t temp = vmulq_f64(SPLITTER, a);
    float64x2_t hi = vsubq_f64(temp, vsubq_f64(temp, a));
    float64x2_t lo = vsubq_f64(a, hi);
    return { hi, lo };
}

static inline VecPair twoProd(float64x2_t a, float64x2_t b)
{
    float64x2_t p = vmulq_f64(a, b);
    auto[a_hi, a_lo] = split(a);
    auto[b_hi, b_lo] = split(b);
    float64x2_t err = vaddq_f64(vaddq_f64(vsubq_f64(vmulq_f64(a_hi, b_hi), p), vaddq_f64(vmulq_f64(a_hi, b_lo), vmulq_f64(a_lo, b_hi))), vmulq_f64(a_lo, b_lo));
    return { p, err };
}

struct NeonDoubleDouble
{
    float64x2_t x[2];

    inline NeonDoubleDouble(const float64x2_t& a, const float64x2_t& b) :
        x{ a, b }
    {}

    inline NeonDoubleDouble(double a, double b) :
        x{ vmovq_n_f64(a), vmovq_n_f64(b) }
    {}


    inline NeonDoubleDouble operator + (const NeonDoubleDouble& sm) const
    {
        auto[s, e] = twoSum(x[0], sm.x[0]);
        e = vaddq_f64(e, vaddq_f64(x[1], sm.x[1]));
        auto[r1, r2] = quickTwoSum(s, e);
        return NeonDoubleDouble{ r1, r2 };
    }

    inline NeonDoubleDouble operator - (const NeonDoubleDouble& sm) const
    {
        auto[s, e] = twoDiff(x[0], sm.x[0]);
        e = vaddq_f64(e, x[1]);
        e = vsubq_f64(e, sm.x[1]);
        auto[r1, r2] = quickTwoSum(s, e);
        return NeonDoubleDouble{ r1, r2 };
    }

    inline NeonDoubleDouble operator * (const NeonDoubleDouble& sm) const
    {
        auto[p1, p2] = twoProd(this->x[0], sm.x[0]);
        p2 = vaddq_f64(p2,
            vaddq_f64(vmulq_f64(sm.x[1], x[0]), vmulq_f64(sm.x[0], x[1])) );
        auto[r1, r2] = quickTwoSum(p1, p2);
        return NeonDoubleDouble{ r1, r2 };
    }
};


template<bool parallel>
void CpuGenerator<mnd::DoubleDouble, mnd::ARM_NEON, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    const MandelViewport& view = info.view;

    using T = LightDoubleDouble;

    T viewx = mnd::convert<T>(view.x);
    T viewy = mnd::convert<T>(view.y);
    T wpp = mnd::convert<T>(view.width / info.bWidth);
    T hpp = mnd::convert<T>(view.height / info.bHeight);

    T jX = mnd::convert<T>(info.juliaX);
    T jY = mnd::convert<T>(info.juliaY);
    NeonDoubleDouble juliaX = { jX[0], jX[1] };
    NeonDoubleDouble juliaY = { jY[0], jY[1] };

#if defined(_OPENMP)
    if constexpr(parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < info.bHeight; j++) {
        T y = viewy + T(double(j)) * hpp;
        NeonDoubleDouble ys{ y[0], y[1] };
        for (long i = 0; i < info.bWidth; i += 2) {
            T x1 = viewx + T(double(i)) * wpp;
            T x2 = x1 + wpp;
            double xarr1[] = { x1[0], x2[0] };
            double xarr2[] = { x1[1], x2[1] };
            float64x2_t x1s = vld1q_f64(xarr1);
            float64x2_t x2s = vld1q_f64(xarr2);

            NeonDoubleDouble xs{ x1s, x2s };

            NeonDoubleDouble cx = info.julia ? juliaX : xs;
            NeonDoubleDouble cy = info.julia ? juliaY : ys;

            float64x2_t threshold = vmovq_n_f64(16.0);
            uint64x2_t counter = vmovq_n_u64(0);
            uint64x2_t adder = vmovq_n_u64(1);

            NeonDoubleDouble a = xs;
            NeonDoubleDouble b = ys;
            float64x2_t resultA = a.x[0];
            float64x2_t resultB = b.x[0];

            float64x2_t resultsa = vmovq_n_f64(0);
            float64x2_t resultsb = vmovq_n_f64(0);

            for (int k = 0; k < info.maxIter; k++) {
                NeonDoubleDouble aa = a * a;
                NeonDoubleDouble bb = b * b;
                NeonDoubleDouble abab = a * b; abab = abab + abab;
                a = aa - bb + cx;
                b = abab + cy;


                uint64x2_t cmp = vcleq_f64(vaddq_f64(aa.x[0], bb.x[0]), threshold);
                if (info.smooth) {
                    resultA = vreinterpretq_f64_u64(vorrq_u64(vandq_u64(cmp, vreinterpretq_u64_f64(a.x[0])), vandq_u64(vreinterpretq_u64_u32(vmvnq_u32(vreinterpretq_u32_u64(cmp))), vreinterpretq_u64_f64(resultA))));
                    resultB = vreinterpretq_f64_u64(vorrq_u64(vandq_u64(cmp, vreinterpretq_u64_f64(b.x[0])), vandq_u64(vreinterpretq_u64_u32(vmvnq_u32(vreinterpretq_u32_u64(cmp))), vreinterpretq_u64_f64(resultB))));
                }
                a = aa - bb + cx;
                b = abab + cy;
                adder = vandq_u64(adder, cmp);
                counter = vaddq_u64(counter, adder);
                // checking for break criterion is possibly expensive, only do it every 8 iterations
                if ((k & 0x7) == 0) {
                    /* // ARM-v7 method
                    uint32x2_t allZero = vorr_u32(vget_low_u32(cmp), vget_high_u32(cmp));
                    if (vget_lane_u32(vpmax_u32(allZero, allZero), 0) == 0) {
                        break;
                    }
                    */
                    uint64_t allZero = vaddvq_u64(cmp);
                    if (allZero == 0) {
                        break;
                    }
                }
            }

            uint64_t resData[2];
            double resa[2];
            double resb[2];
            vst1q_u64(resData, counter);
            vst1q_f64(resa, resultA);
            vst1q_f64(resb, resultB);

            for (int k = 0; k < 2 && i + k < info.bWidth; k++) {
                if (info.smooth)
                    data[i + k + j * info.bWidth] = resData[k] <= 0 ? info.maxIter :
                    resData[k] >= info.maxIter ? info.maxIter :
                    ((float) resData[k]) + 1 - ::logf(::logf(resa[k] * resa[k] + resb[k] * resb[k]) / 2) / ::logf(2.0f);
                else
                    data[i + k + j * info.bWidth] = resData[k] > 0 ? float(resData[k]) : info.maxIter;
            }
        }
    }
}
