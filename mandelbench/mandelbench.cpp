#include <iostream>
#include <iomanip>
#include <chrono>
#include <functional>
#include <Mandel.h>
#include <cmath>

#include "Fixed.h"

constexpr mnd::MandelViewport benchViewport(void)
{
    return mnd::MandelViewport{ -1.250000598933854152929, 0.0001879894057291665530, 0.0000003839916666666565, 0.0000003839916666666565 };
}

const std::vector<mnd::MandelInfo> benches {
    mnd::MandelInfo{ benchViewport(), 100, 100, 1000 },
    mnd::MandelInfo{ benchViewport(), 100, 200, 1000 },
    mnd::MandelInfo{ benchViewport(), 200, 200, 1000 },
    mnd::MandelInfo{ benchViewport(), 200, 200, 2000 },
    mnd::MandelInfo{ benchViewport(), 200, 400, 2000 },
    mnd::MandelInfo{ benchViewport(), 400, 400, 2000 },
    mnd::MandelInfo{ benchViewport(), 400, 400, 4000 },
    mnd::MandelInfo{ benchViewport(), 400, 800, 4000 },
    mnd::MandelInfo{ benchViewport(), 800, 800, 4000 },
    mnd::MandelInfo{ benchViewport(), 800, 800, 8000 },
    mnd::MandelInfo{ benchViewport(), 800, 800, 16000 },
    mnd::MandelInfo{ benchViewport(), 800, 1600, 16000 },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 16000 },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 32000 },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 64000 },
    mnd::MandelInfo{ benchViewport(), 1600, 3200, 64000 },
    mnd::MandelInfo{ benchViewport(), 3200, 3200, 64000 },
    mnd::MandelInfo{ benchViewport(), 3200, 3200, 128000 },
    mnd::MandelInfo{ benchViewport(), 3200, 3200, 256000 },
    mnd::MandelInfo{ benchViewport(), 3200, 3200, 512000 },
    mnd::MandelInfo{ benchViewport(), 3200, 3200, 1024000 },
    mnd::MandelInfo{ benchViewport(), 3200, 3200, 2048000 },
    mnd::MandelInfo{ benchViewport(), 3200, 6400, 2048000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 2048000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 4096000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 8192000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 16384000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 32768000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 65536000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 131072000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 262144000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 524288000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 1048576000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 2097152000 },
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


double benchmark(mnd::Generator& generator)
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
        if (time > std::chrono::milliseconds(1000)) {
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
do { std::cout << std::setw(30) << name << std::setw(10) << std::right << std::showbase << std::fixed << std::setprecision(2) << performance << std::endl; } while(0)

int main()
{
    Fixed128 a = 5.2;
    Fixed128 b = 2.0;

    Fixed128 c = a / b;

    std::cout << "val: " << double(c) << std::endl;

    mnd::MandelContext mc = mnd::initializeContext();

    std::cout << "Benchmarking CPU [" << mc.getCpuInfo().getBrand() << "]" << std::endl;

    REPORT_PERFORMANCE("float [MIps]: ", benchmark(mc.getCpuGeneratorFloat()));
    REPORT_PERFORMANCE("double [MIps]: ", benchmark(mc.getCpuGeneratorDouble()));
    REPORT_PERFORMANCE("fixed-point 128 bit [MIps]: ", benchmark(mc.getCpuGenerator128()));
    

    for (auto& device : mc.getDevices()) {
        std::cout << "Benchmarking Device [" << device.getName() << "]" << std::endl;
        if (mnd::Generator* gpuf; gpuf = device.getGeneratorFloat()) {
            REPORT_PERFORMANCE("float [MIps]: ", benchmark(*gpuf));
        }
        if (mnd::Generator* gpud; gpud = device.getGeneratorDouble()) {
            REPORT_PERFORMANCE("double [MIps]: ", benchmark(*gpud));
        }
        if (mnd::Generator* gpu128; gpu128 = device.getGenerator128()) {
            REPORT_PERFORMANCE("fixed-point 128 bit [MIps]: ", benchmark(*gpu128));
        }
    }


    /*
    std::cout << std::setw(30) << "float [MIps]: " << std::setw(10) << std::right << std::showbase << std::fixed << std::setprecision(2) << benchmark(mc.getCpuGeneratorFloat()) << std::endl;
    std::cout << std::setw(30) << "double [MIps]: " << std::setw(10) << std::right << std::showbase << std::fixed << std::setprecision(2) << benchmark(mc.getCpuGeneratorDouble()) << std::endl;
    std::cout << std::setw(30) << "fixed-point 128 bit [MIps]: " << std::setw(10) << std::right << std::showbase << std::fixed << std::setprecision(2) << benchmark(mc.getCpuGenerator128()) << std::endl;
    */
}

