#ifndef MANDEL_CPUGENERATORS_H
#define MANDEL_CPUGENERATORS_H

#include "Generators.h"

namespace mnd
{
    enum CpuExtension
    {
        NONE,
        X86_SSE2,
        X86_AVX,
        X86_AVX_FMA,
        ARM_NEON,
    };

    template<typename T, mnd::CpuExtension ex = mnd::NONE, bool parallel = true>
    class CpuGenerator;
}


template<typename T, mnd::CpuExtension ex, bool parallel>
class mnd::CpuGenerator : public Generator
{
public:
    inline CpuGenerator(void) :
        Generator{ mnd::getPrecision<T>() }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<typename T, bool parallel>
class mnd::CpuGenerator<T, mnd::NONE, parallel> : public Generator
{
public:
    inline CpuGenerator(void) :
        Generator{ mnd::getPrecision<T>() }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86) 
template<bool parallel>
class mnd::CpuGenerator<float, mnd::X86_SSE2, parallel> : public Generator
{
public:
    inline CpuGenerator(void) :
        Generator{ mnd::getPrecision<float>() }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};

template<bool parallel>
class mnd::CpuGenerator<double, mnd::X86_SSE2, parallel> : public Generator
{
public:
    inline CpuGenerator(void) :
        Generator{ mnd::getPrecision<double>() }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<bool parallel>
class mnd::CpuGenerator<float, mnd::X86_AVX, parallel> : public Generator
{
public:
    inline CpuGenerator(void) :
        Generator{ mnd::getPrecision<float>() }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};

template<bool parallel>
class mnd::CpuGenerator<double, mnd::X86_AVX, parallel> : public Generator
{
public:
    inline CpuGenerator(void) :
        Generator{ mnd::getPrecision<double>() }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};

template<bool parallel>
class mnd::CpuGenerator<mnd::DoubleDouble, mnd::X86_AVX, parallel> : public Generator
{
public:
    inline CpuGenerator(void) :
        Generator{ mnd::getPrecision<DoubleDouble>() }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};

#else //if defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) 
template<bool parallel>
class mnd::CpuGenerator<float, mnd::ARM_NEON, parallel> : public Generator
{
public:
    CpuGenerator(void);
    inline CpuGenerator(void) :
        Generator{ mnd::getPrecision<float>() }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};

template<bool parallel>
class mnd::CpuGenerator<double, mnd::ARM_NEON, parallel> : public Generator
{
public:
    inline CpuGenerator(void) :
        Generator{ mnd::getPrecision<double>() }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};
#endif


template<bool parallel>
class mnd::CpuGenerator<double, mnd::X86_AVX_FMA, parallel> : public Generator
{
public:
    inline CpuGenerator(void) :
        Generator{ mnd::getPrecision<double>() }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<bool parallel>
class mnd::CpuGenerator<float, mnd::X86_AVX_FMA, parallel> : public Generator
{
public:
    inline CpuGenerator(void) :
        Generator{ mnd::getPrecision<float>() }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<bool parallel>
class mnd::CpuGenerator<mnd::DoubleDouble, mnd::X86_AVX_FMA, parallel> : public Generator
{
public:
    inline CpuGenerator(void) :
        Generator{ mnd::getPrecision<DoubleDouble>() }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


#endif // MANDEL_CPUGENERATORS_H
