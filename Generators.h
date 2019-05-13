#pragma once
#ifndef MANDELQUEUE_H_
#define MANDELQUEUE_H_

#include "QueueManager.h"
#include "GenericMandelbrot.h"
#include <omp.h>
#include <future>
#include <cstdlib>

#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

#include <immintrin.h>


class ClGenerator : public MandelGenerator
{
    cl::Device device;
    cl::Context context;
    cl::Program program;
    cl::CommandQueue queue;
public:
    ClGenerator(void);
    ~ClGenerator(void) = default;

    //Bitmap<RGBColor> generate(const MandelInfo& info);
    Bitmap<float> generateRaw(const MandelInfo& info);

    std::future<Bitmap<RGBColor>> enqueueMandelbrot(long width, long height, float x, float y, float fwidth);
};


std::future<Bitmap<RGBColor>> createHPM();


template<typename T>
class CpuGenerator : public MandelGenerator
{
public:

    Bitmap<float> generateRaw(const MandelInfo& info)
    {
        const MandelViewport& view = info.view;
        Bitmap<float> res{ info.bWidth, info.bHeight };

        omp_set_num_threads(2 * omp_get_num_procs());
#pragma omp parallel for
        for (int j = 0; j < res.height; j++) {
            T y = T(view.y) + T(j) * view.height / res.height;
            for (::size_t i = 0; i < res.width; i++) {
                T x = T(view.x) + T(i) * view.width / res.width;
                res.get(i, j) = iterate<T>(x, y, info.maxIter);
            }
        }
        return res;
    }
};


template<>
Bitmap<float> CpuGenerator<double>::generateRaw(const MandelInfo& info)
{
    using T = double;
    const MandelViewport& view = info.view;
    Bitmap<float> res{ info.bWidth, info.bHeight };

    omp_set_num_threads(2 * omp_get_num_procs());
#pragma omp parallel for
    for (long j = 0; j < res.height; j++) {
        T y = T(view.y) + T(j) * view.height / res.height;
        long i = 0;
        for (i; i < res.width; i += 4) {
            __m256d xs = {
                double(view.x) + double(i) * view.width / res.width,
                double(view.x) + double(i + 1) * view.width / res.width,
                double(view.x) + double(i + 2) * view.width / res.width,
                double(view.x) + double(i + 3) * view.width / res.width
            };

            int itRes[4] = {0, 0, 0, 0};

            __m256d threshold = {16.0, 16.0, 16.0, 16.0};
            __m256d counter = {0, 0, 0, 0};
            __m256d adder = {1, 1, 1, 1};

            __m256d ys = {y, y, y, y};
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
            double data[8];
            void* aligned = data;
            unsigned int length = sizeof data;
            std::align(32, 4 * sizeof(double), aligned, length);
            double* ftRes = static_cast<double*>(aligned);
            _mm256_store_pd(ftRes, counter);
            for (int k = 0; k < 4; k++)
                res.get(i + k, j) = ftRes[k] > 0 ? float(ftRes[k]) : info.maxIter;
        }
    }

    return res;
}


template<>
Bitmap<float> CpuGenerator<float>::generateRaw(const MandelInfo& info)
{
    using T = float;
    const MandelViewport& view = info.view;
    Bitmap<float> res{ info.bWidth, info.bHeight };

    omp_set_num_threads(2 * omp_get_num_procs());
#pragma omp parallel for
    for (long j = 0; j < res.height; j++) {
        T y = T(view.y) + T(j) * view.height / res.height;
        long i = 0;
        for (i; i < res.width; i += 8) {
            __m256 xs = {
                float(view.x + double(i) * view.width / res.width),
                float(view.x + double(i + 1) * view.width / res.width),
                float(view.x + double(i + 2) * view.width / res.width),
                float(view.x + double(i + 3) * view.width / res.width),
                float(view.x + double(i + 4) * view.width / res.width),
                float(view.x + double(i + 5) * view.width / res.width),
                float(view.x + double(i + 6) * view.width / res.width),
                float(view.x + double(i + 7) * view.width / res.width)
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
            float data[16];
            void* aligned = data;
            unsigned int length = sizeof data;
            std::align(32, 8 * sizeof(float), aligned, length);
            float* ftRes = static_cast<float*>(aligned);
            _mm256_store_ps(ftRes, counter);
            for (int k = 0; k < 8; k++)
                res.get(i + k, j) = ftRes[k] > 0 ? ftRes[k] : info.maxIter;
        }
    }

    return res;
}



#endif // MANDELQUEUE_H_
