#include "Generators.h"

#include <cstdio>

using mnd::Generator;
using mnd::AdaptiveGenerator;

Generator::~Generator(void)
{
}


AdaptiveGenerator::AdaptiveGenerator(Generator* floatGen, Generator* doubleGen)
{
    generators.insert({ 0.0000001, floatGen });
    generators.insert({ 0.0, doubleGen });
}

/*
AdaptiveGenerator::AdaptiveGenerator(Generator* floatGen, Generator* doubleGen, Generator* quadGen)
{
    generators.insert({ 0.0000001, floatGen });
    generators.insert({ 5.0e-16, doubleGen });
    generators.insert({ Real("1.0e-28"), quadGen });
}*/


void AdaptiveGenerator::addGenerator(const mnd::Real& precision, mnd::Generator& generator)
{
    generators.insert({ precision, &generator });
}


void AdaptiveGenerator::addGenerator(Precision p, Generator& generator)
{
    generators.insert({ getPrecision(p), &generator });
}


void AdaptiveGenerator::generate(const mnd::MandelInfo& info, float* data)
{
    Real pixelW = info.view.width / info.bWidth;
    Real pixelH = info.view.height / info.bHeight;
    Real neededPrecision = pixelW < pixelH ? pixelW : pixelH;

    //Generator* toUse = nullptr;
    auto firstSmaller = generators.lower_bound(neededPrecision);
    if (firstSmaller != generators.end()) {
        //printf("use generator with precision: %s\n", mnd::toString(firstSmaller->first).c_str());
        firstSmaller->second->generate(info, data);
    }
    else {
        for (long s = 0; s < info.bWidth * info.bHeight; s++) {
            data[s] = 0.0;
        }
    }
    return;

/*
    int i = 0;

    for (auto [thresh, gen] : generators) {
        ++i;
        if (neededPrecision > thresh) {
            toUse = gen;
        }
    }
    if (toUse != nullptr) {
        toUse->generate(info, data);
    }
    else {
        for (long s = 0; s < info.bWidth * info.bHeight; s++) {
            data[s] = 0.0;
        }
    }*/
}

namespace mnd
{
    Real getPrecision(Precision p)
    {
        static const std::map<Precision, Real> precs {
            { Precision::FLOAT, 1.0e-7 },
            { Precision::DOUBLE, 1.0e-15 },
            { Precision::DOUBLE_DOUBLE, Real("1.0e-29") },
            { Precision::QUAD_DOUBLE, Real("1.0e-56") },
            { Precision::FLOAT256, Real("1.0e-58") },
            { Precision::INFINITE, Real(0.0) },
        };

        return precs.at(p);
    }
}




