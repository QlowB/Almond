#include <iostream>
#include <iomanip>
#include <chrono>
#include <functional>
#include <Mandel.h>
#include <cmath>

#include "Fixed.h"

mnd::MandelViewport benchViewport(void)
{
    return mnd::MandelViewport{ -1.250000598933854152929, 0.0001879894057291665530, 0.0000003839916666666565, 0.0000003839916666666565 };
}

const std::vector<mnd::MandelInfo> benches = {
    mnd::MandelInfo{ benchViewport(), 32, 32, 15, false },
    mnd::MandelInfo{ benchViewport(), 32, 32, 25, false },
    mnd::MandelInfo{ benchViewport(), 32, 32, 75, false },
    mnd::MandelInfo{ benchViewport(), 32, 32, 125, false },
    mnd::MandelInfo{ benchViewport(), 32, 32, 250, false },
    mnd::MandelInfo{ benchViewport(), 64, 32, 250, false },
    mnd::MandelInfo{ benchViewport(), 64, 64, 250, false },
    mnd::MandelInfo{ benchViewport(), 64, 64, 500, false },
    mnd::MandelInfo{ benchViewport(), 64, 128, 500, false },
    mnd::MandelInfo{ benchViewport(), 128, 128, 500, false },
    mnd::MandelInfo{ benchViewport(), 128, 128, 1000, false },
    mnd::MandelInfo{ benchViewport(), 128, 256, 1000, false },
    mnd::MandelInfo{ benchViewport(), 256, 256, 1000, false },
    mnd::MandelInfo{ benchViewport(), 256, 256, 2000, false },
    mnd::MandelInfo{ benchViewport(), 256, 512, 2000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 2000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 4000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 8000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 16000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 32000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 64000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 128000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 256000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 512000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 1024000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 2048000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 4096000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 8192000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 16384000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 32768000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 65536000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 131072000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 262144000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 524288000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 1048576000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 2097152000, false },
};

std::pair<long long, std::chrono::nanoseconds> measureMips(const std::function<std::pair<float*, long>()>& bench)
{
    using namespace std::chrono;
    auto before = high_resolution_clock::now();
    auto [bitmap, length] = bench();
    auto after = high_resolution_clock::now();

    long long sum = 0;
    for (int i = 0; i < length; i++) {
        sum += std::floor(bitmap[size_t(i)]);
    }

    return std::make_pair(sum, duration_cast<nanoseconds>(after - before));

    /*
    double iterPerNanos = double(sum) / duration_cast<nanoseconds>(after - before).count();
    //printf("test took %lld millis\n", duration_cast<milliseconds>(after - before).count());
    //printf("test did %lld iters\n", sum);
    double megaItersPerSecond = iterPerNanos * 1000.0;
    return megaItersPerSecond;*/
}


double benchmark(mnd::MandelGenerator& generator)
{
    /*mnd::MandelInfo mi;
    mi.bWidth = 250;
    mi.bHeight = 250;
    mi.maxIter = 4000;
    mi.view = benchViewport();*/

    int testIndex = 0;

    for (int i = 0; i < benches.size(); i++) {
        const mnd::MandelInfo& mi = benches[i];
        auto data = std::make_unique<float[]>(mi.bWidth * mi.bHeight);
        auto [iters, time] = measureMips([&generator, &mi, &data]() { generator.generate(mi, data.get()); return std::make_pair(data.get(), mi.bWidth * mi.bHeight);  });
        //printf("benchmark lvl %d, time %d ms\n", i, time.count() / 1000 / 1000);
        //fflush(stdout);
        if (time > std::chrono::milliseconds(200)) {
            testIndex = i + 2;
            break;
        }
    }


    const mnd::MandelInfo& mi = benches[(testIndex >= benches.size()) ? (benches.size() - 1) : testIndex];
    auto data = std::make_unique<float[]>(mi.bWidth * mi.bHeight);

    auto [iters, time] = measureMips([&generator, &mi, &data]() { generator.generate(mi, data.get()); return std::make_pair(data.get(), mi.bWidth * mi.bHeight);  });
    //printf("bench time %d ms\n", time.count() / 1000 / 1000);
    //fflush(stdout);

    return double(iters) / time.count() * 1000;
}

#define REPORT_PERFORMANCE(name, performance) \
do { std::cout << std::setw(30) << (name) << std::setw(10) << std::right << std::showbase << std::fixed << std::setprecision(2) << performance << std::endl; } while(0)

int main()
{
    mnd::MandelContext mc = mnd::initializeContext();

    std::cout << "Benchmarking CPU [" << mc.getCpuInfo().getBrand() << "]" << std::endl;

    auto types = mc.getSupportedTypes();
    for (auto type : types) {
        REPORT_PERFORMANCE(mnd::getGeneratorName(type) + " [MIps]: ", benchmark(*mc.getCpuGenerator(type)));
    }
    
    std::cout << std::endl;
    for (auto& device : mc.getDevices()) {
        std::cout << "Benchmarking Device [" << device->getVendor() << " " << device->getName() << "]" << std::endl;

        auto types = device->getSupportedTypes();
        for (auto type : types) {
            REPORT_PERFORMANCE(mnd::getGeneratorName(type) + " [MIps]: ", benchmark(*device->getGenerator(type)));
        }
        std::cout << std::endl;
    }
}

