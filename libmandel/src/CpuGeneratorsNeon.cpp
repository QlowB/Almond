#include "CpuGenerators.h"

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
}


template<bool parallel>
void CpuGenerator<float, mnd::ARM_NEON, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = float;
    const MandelViewport& view = info.view;
    omp_set_num_threads(2 * omp_get_num_procs());
#pragma omp parallel for
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y) + T(j) * T(view.height / info.bHeight);
        long i = 0;
        for (i; i < info.bWidth; i += 4) {
            float xsvals[] = {
                float(view.x + double(i) * view.width / info.bWidth),
                float(view.x + double(i + 1) * view.width / info.bWidth),
                float(view.x + double(i + 2) * view.width / info.bWidth),
                float(view.x + double(i + 3) * view.width / info.bWidth)
            };

            float32x4_t xs = vld1q_f32(xsvals);


            uint32x4_t counter = vmovq_n_u32(0);
            uint32x4_t adder = vmovq_n_u32(1);
            //uint32x4_t ones = vmovq_n_u32(1);

            float32x4_t threshold = vmovq_n_f32(16);

            float32x4_t ys = vmovq_n_f32(y);
            float32x4_t a = xs;
            float32x4_t b = ys;

            for (int k = 0; k < info.maxIter; k++) {
                float32x4_t aa = vmulq_f32(a, a);
                float32x4_t bb = vmulq_f32(b, b);
                float32x4_t abab = vmulq_f32(a, b); abab = vaddq_f32(abab, abab);
                a = vaddq_f32(vsubq_f32(aa, bb), xs);
                b = vaddq_f32(abab, ys);
                uint32x4_t cmp = vcleq_f32(vaddq_f32(aa, bb), threshold);
                adder = vandq_u32(adder, cmp);
                counter = vaddq_u32(counter, adder);
                // checking for break criterion is possibly expensive, only do it every 8 iterations
                if ((k & 0xF) == 0) {
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
            vst1q_u32(resData, counter);
            for (int k = 0; k < 4 && i + k < info.bWidth; k++)
                data[i + k + j * info.bWidth] = resData[k] > 0 ? resData[k] : info.maxIter;
        }
    }
}


template<bool parallel>
void CpuGenerator<double, mnd::ARM_NEON, parallel>::generate(const mnd::MandelInfo& info, float* data)
{
    using T = double;
    const MandelViewport& view = info.view;
    omp_set_num_threads(2 * omp_get_num_procs());
#pragma omp parallel for
    for (long j = 0; j < info.bHeight; j++) {
        T y = T(view.y) + T(j) * T(view.height / info.bHeight);
        long i = 0;
        for (i; i < info.bWidth; i += 2) {
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

            for (int k = 0; k < info.maxIter; k++) {
                float64x2_t aa = vmulq_f64(a, a);
                float64x2_t bb = vmulq_f64(b, b);
                float64x2_t abab = vmulq_f64(a, b); abab = vaddq_f64(abab, abab);
                a = vaddq_f64(vsubq_f64(aa, bb), xs);
                b = vaddq_f64(abab, ys);
                uint64x2_t cmp = vcleq_f64(vaddq_f64(aa, bb), threshold);
                adder = vandq_u64(adder, cmp);
                counter = vaddq_u64(counter, adder);
                // checking for break criterion is possibly expensive, only do it every 8 iterations
                if ((k & 0xF) == 0) {
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
            vst1q_u64(resData, counter);
            for (int k = 0; k < 2 && i + k < info.bWidth; k++)
                data[i + k + j * info.bWidth] = resData[k] > 0 ? resData[k] : info.maxIter;
        }
    }
}
