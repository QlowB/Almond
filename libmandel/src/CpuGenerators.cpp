#include "CpuGenerators.h"

#include <omp.h>

#include <memory>

using mnd::CpuGeneratorFloat;
using mnd::CpuGeneratorDouble;


void CpuGeneratorFloat::generate(const mnd::MandelInfo& info, float* data)
{
    const MandelViewport& view = info.view;
    omp_set_num_threads(2 * omp_get_num_procs());
#pragma omp parallel for
    for (long j = 0; j < info.bHeight; j++) {
        float y = float(view.y) + float(j) * float(view.height / info.bHeight);
        long i = 0;
        for (i; i < info.bWidth; i++) {
            float x = float(view.x + double(i) * view.width / info.bWidth);

            float a = x;
            float b = y;

            int k = 0;
            for (k = 0; k < info.maxIter; k++) {
                float aa = a * a;
                float bb = b * b;
                float ab = a * b;
                a = aa - bb + x;
                b = ab + ab + y;
                if (aa + bb > 16) {
                    break;
                }
            }
            data[i + j * info.bWidth] = k;
        }
    }
}


void CpuGeneratorDouble::generate(const mnd::MandelInfo& info, float* data)
{
    const MandelViewport& view = info.view;
    omp_set_num_threads(2 * omp_get_num_procs());
#pragma omp parallel for
    for (long j = 0; j < info.bHeight; j++) {
        double y = double(view.y) + double(j) * double(view.height / info.bHeight);
        long i = 0;
        for (i; i < info.bWidth; i++) {
            double x = view.x + double(i) * view.width / info.bWidth;

            double a = x;
            double b = y;

            int k = 0;
            for (k = 0; k < info.maxIter; k++) {
                double aa = a * a;
                double bb = b * b;
                double ab = a * b;
                a = aa - bb + x;
                b = ab + ab + y;
                if (aa + bb > 16) {
                    break;
                }
            }
            data[i + j * info.bWidth] = k;
        }
    }
}

