#ifndef MANDEL_GENERATORS_H
#define MANDEL_GENERATORS_H

#include "MandelUtil.h"

#include <vector>
#include <map>
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
    std::map<Real, Generator*, std::greater<Real>> generators;
public:
    AdaptiveGenerator(void) = default;
    AdaptiveGenerator(Generator* floatGen, Generator* doubleGen);
    virtual ~AdaptiveGenerator(void) = default;

    void addGenerator(const Real& precision, Generator& generator);

    virtual void generate(const MandelInfo& info, float* data);
};


#endif // MANDEL_GENERATORS_H
