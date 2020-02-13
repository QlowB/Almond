#include "Generators.h"

#include <cstdio>

using mnd::Generator;
using mnd::AdaptiveGenerator;

Generator::~Generator(void)
{
}


AdaptiveGenerator::AdaptiveGenerator(Generator* floatGen, Generator* doubleGen)
{
    generators.push_back({ 0.0000001, floatGen });
    generators.push_back({ 0.0, doubleGen });
}


AdaptiveGenerator::AdaptiveGenerator(Generator* floatGen, Generator* doubleGen, Generator* quadGen)
{
    //generators.push_back({ 0.0000001, floatGen });
    //generators.push_back({ 5.0e-16, doubleGen });
    generators.push_back({ 0.0, quadGen });
}


void AdaptiveGenerator::generate(const mnd::MandelInfo& info, float* data)
{
    Real pixelW = info.view.width / info.bWidth;
    Real pixelH = info.view.height / info.bHeight;
    Real minimum = pixelW < pixelH ? pixelW : pixelH;

    Generator* toUse = nullptr;
    int i = 0;

    for (auto [thresh, gen] : generators) {
        ++i;
        if (minimum > thresh) {
            toUse = gen;
            break;
        }
    }
    if (toUse != nullptr) {
        toUse->generate(info, data);
    }
    else {
        for (long s = 0; s < info.bWidth * info.bHeight; s++) {
            data[s] = 0.0;
        }
    }
}


