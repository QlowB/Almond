#include "Benchmark.h"

#include "Generators.h"

#include <chrono>

namespace mnd
{

struct BenchmarkResult
{
    double iterations;
    double nanoseconds;
};


BenchmarkResult measure(MandelGenerator& generator, const MandelInfo& mi)
{
    auto floats = std::make_unique<float[]>(mi.bWidth * mi.bHeight);
    for (long i = 0; i < mi.bWidth * mi.bHeight; i++) {
        floats[i] = 1.0;
    }

    auto before = std::chrono::high_resolution_clock::now();
    generator.generate(mi, floats.get());
    auto after = std::chrono::high_resolution_clock::now();
    auto timeUsed = after - before;

    double sum = 0.0;
    for (long i = 0; i < mi.bWidth * mi.bHeight; i++) {
        sum += floats[i];
    }

    return BenchmarkResult {
        sum,
        double(std::chrono::duration_cast<std::chrono::nanoseconds>(timeUsed).count())
    };
}


MandelViewport getQuarter(const MandelViewport& v, int quarter)
{
    Real x = v.x;
    Real y = v.y;
    if (quarter & 1)
        x += v.width / 2;
    if (quarter & 2)
        y += v.height / 2;
    return MandelViewport{ x, y, v.width / 2, v.height / 2 };
}


std::pair<MandelViewport, double> findBestQuarter(MandelGenerator& generator, const MandelInfo& mi, int maxRecursion)
{
    BenchmarkResult full = measure(generator, mi);
    MandelViewport maxViewport = mi.view;
    double maxIterations = full.iterations;

    if (maxRecursion <= 0) {
        return std::make_pair(maxViewport, maxIterations);
    }

    for (int i = 0; i < 4; i++) {
        MandelViewport q = getQuarter(mi.view, i);
        MandelInfo testInfo = mi;
        testInfo.view = q;
        auto[bestV, bestIt] = findBestQuarter(generator, testInfo, 0);

        if (bestIt > maxIterations) {
            maxIterations = bestIt;
            maxViewport = q;
        }
    }

    MandelInfo testInfo = mi;
    testInfo.view = maxViewport;
    auto[fV, fI] = findBestQuarter(generator, testInfo, maxRecursion - 1);

    return std::make_pair(fV, fI);
}


double benchmark(MandelGenerator& generator)
{
    MandelViewport view = MandelViewport::centerView();

    MandelInfo mi;
    mi.bWidth = 32;
    mi.bHeight = 32;
    mi.julia = false;
    mi.maxIter = 32;
    mi.smooth = false;
    mi.view = view;

    auto upgrade = [&mi]() {
        if (mi.maxIter < 1024)
            mi.maxIter *= 2;
        else if (mi.bWidth < 1024)
            mi.bWidth *= 2;
        else if (mi.bHeight < 1024)
            mi.bHeight *= 2;
        else if (mi.maxIter < 4194304)
            mi.maxIter *= 2;
        else
            return true;
        return false;
    };

    bool foundBench = false;
    while (!foundBench) {
        BenchmarkResult br = measure(generator, mi);

        // select best-suited part of fractal
        if (br.iterations >= 1024 * 128) {
            mi.bWidth /= 4;
            mi.bHeight /= 4;
            auto[view, it] = findBestQuarter(generator, mi, 5);
            mi.bWidth *= 4;
            mi.bHeight *= 4;
            mi.view = view;
        }

        if (br.nanoseconds > 1000 * 1000 * 250) {
            foundBench = true;
        }
        else if (br.nanoseconds < 1000 * 1000 * 10) {
            foundBench = upgrade();
            foundBench = upgrade();
        }
        else {
            foundBench = upgrade();
        }
    }

    upgrade();
    upgrade();

    BenchmarkResult br = measure(generator, mi);
    return br.iterations / br.nanoseconds;
}

}

