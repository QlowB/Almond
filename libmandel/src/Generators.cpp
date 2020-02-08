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


void AdaptiveGenerator::generate(const mnd::MandelInfo& info, float* data)
{
    double pixelW = info.view.width / info.bWidth;
    double pixelH = info.view.height / info.bHeight;
    double minimum = pixelW < pixelH ? pixelW : pixelH;

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

    }
}


