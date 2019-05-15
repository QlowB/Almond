#ifndef MANDEL_CPUGENERATORSAVX_H
#define MANDEL_CPUGENERATORSAVX_H

#include "Generators.h"

namespace mnd
{
    class CpuGeneratorAvxFloat;
    class CpuGeneratorAvxDouble;
}

class mnd::CpuGeneratorAvxFloat
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};

class mnd::CpuGeneratorAvxDouble
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};

#endif // MANDEL_CPUGENERATORSAVX_H
