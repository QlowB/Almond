#ifndef MANDEL_GENERATORS_H
#define MANDEL_GENERATORS_H

#include "MandelUtil.h"

#include <vector>
#include <utility>



namespace mnd
{
    class Generator;

    class AdaptiveGenerator;
}


class mnd::Generator
{
public:
    Generator(void) = default;
    virtual ~Generator(void);


    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;

    Generator(Generator&&) = default;
    Generator& operator=(Generator&&) = default;

    virtual void generate(const MandelInfo& info, float* data) = 0;
};


class mnd::AdaptiveGenerator : public Generator
{
    std::vector<std::pair<Real, Generator*>> generators;
public:
    AdaptiveGenerator(Generator* floatGen, Generator* doubleGen);
    AdaptiveGenerator(Generator* floatGen, Generator* doubleGen, Generator* quadGen);
    virtual ~AdaptiveGenerator(void) = default;

    virtual void generate(const MandelInfo& info, float* data);
};


#endif // MANDEL_GENERATORS_H
