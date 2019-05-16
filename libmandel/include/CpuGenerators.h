#ifndef MANDEL_CPUGENERATORS_H
#define MANDEL_CPUGENERATORS_H

#include "Generators.h"

namespace mnd
{
    class CpuGeneratorFloat;
    class CpuGeneratorDouble;

    class CpuGeneratorSse2Float;
    class CpuGeneratorSse2Double;

    class CpuGeneratorAvxFloat;
    class CpuGeneratorAvxDouble;
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


class mnd::CpuGeneratorSse2Float : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};


class mnd::CpuGeneratorSse2Double : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};


class mnd::CpuGeneratorAvxFloat : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};


class mnd::CpuGeneratorAvxDouble : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};

#endif // MANDEL_CPUGENERATORS_H
