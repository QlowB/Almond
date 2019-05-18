#ifndef MANDEL_CPUGENERATORS_H
#define MANDEL_CPUGENERATORS_H

#include "Generators.h"

namespace mnd
{
    class CpuGeneratorFloat;
    class CpuGeneratorDouble;
    class CpuGenerator128;

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86) 
    class CpuGeneratorSse2Float;
    class CpuGeneratorSse2Double;

    class CpuGeneratorAvxFloat;
    class CpuGeneratorAvxDouble;

    class CpuGeneratorAvx512Float;
    class CpuGeneratorAvx512Double;
#endif
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


class mnd::CpuGenerator128 : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};



#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86) 
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


class mnd::CpuGeneratorAvx512Float : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};


class mnd::CpuGeneratorAvx512Double : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};
#endif 

#endif // MANDEL_CPUGENERATORS_H
