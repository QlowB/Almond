#ifndef MANDEL_CPUGENERATORS_H
#define MANDEL_CPUGENERATORS_H

#include "Generators.h"

namespace mnd
{
    class CpuGeneratorFloat;
    class CpuGeneratorDouble;
}


class mnd::CpuGeneratorFloat : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};


class mnd::CpuGeneratorDouble : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};

#endif // MANDEL_CPUGENERATORS_H
