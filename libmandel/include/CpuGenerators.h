#ifndef MANDEL_CPUGENERATORS_H
#define MANDEL_CPUGENERATORS_H

#include "Generators.h"
#include "Types.h"

namespace mnd
{
    template<typename T, mnd::HardwareFeature ex = mnd::NONE, bool parallel = true>
    class CpuGenerator;
}


template<typename T, mnd::HardwareFeature ex, bool parallel>
class mnd::CpuGenerator : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::getType<T>(), ex }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<typename T, bool parallel>
class mnd::CpuGenerator<T, mnd::NONE, parallel> : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::getType<T>(), mnd::NONE }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
template<bool parallel>
class mnd::CpuGenerator<float, mnd::X86_SSE2, parallel> : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::Precision::FLOAT, mnd::X86_SSE2 }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};

template<bool parallel>
class mnd::CpuGenerator<double, mnd::X86_SSE2, parallel> : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::Precision::DOUBLE, mnd::X86_SSE2 }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<bool parallel>
class mnd::CpuGenerator<float, mnd::X86_AVX, parallel> : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::Precision::FLOAT, mnd::X86_AVX }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<bool parallel>
class mnd::CpuGenerator<double, mnd::X86_AVX, parallel> : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::Precision::DOUBLE, mnd::X86_AVX }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<bool parallel>
class mnd::CpuGenerator<mnd::DoubleDouble, mnd::X86_AVX, parallel> : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::Precision::DOUBLE_DOUBLE, mnd::X86_AVX }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<bool parallel>
class mnd::CpuGenerator<mnd::TripleDouble, mnd::X86_AVX, parallel> : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::Precision::TRIPLE_DOUBLE, mnd::X86_AVX }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<bool parallel>
class mnd::CpuGenerator<float, mnd::X86_AVX_FMA, parallel> : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::Precision::FLOAT, mnd::X86_AVX_FMA }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<bool parallel>
class mnd::CpuGenerator<double, mnd::X86_AVX_FMA, parallel> : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::Precision::DOUBLE, mnd::X86_AVX_FMA }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<bool parallel>
class mnd::CpuGenerator<mnd::DoubleDouble, mnd::X86_AVX_FMA, parallel> : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::Precision::DOUBLE_DOUBLE, mnd::X86_AVX_FMA }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<bool parallel>
class mnd::CpuGenerator<mnd::QuadDouble, mnd::X86_AVX_FMA, parallel> : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::Precision::QUAD_DOUBLE, mnd::X86_AVX_FMA }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<bool parallel>
class mnd::CpuGenerator<mnd::HexDouble, mnd::X86_AVX_FMA, parallel> : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::Precision::HEX_DOUBLE, mnd::X86_AVX_FMA }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<bool parallel>
class mnd::CpuGenerator<float, mnd::X86_AVX_512, parallel> : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::Precision::FLOAT, mnd::X86_AVX_512 }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<bool parallel>
class mnd::CpuGenerator<double, mnd::X86_AVX_512, parallel> : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::Precision::DOUBLE, mnd::X86_AVX_512 }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};

#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) 
template<bool parallel>
class mnd::CpuGenerator<float, mnd::ARM_NEON, parallel> : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::Precision::FLOAT, mnd::ARM_NEON }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<bool parallel>
class mnd::CpuGenerator<double, mnd::ARM_NEON, parallel> : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::Precision::DOUBLE, mnd::ARM_NEON }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};


template<bool parallel>
class mnd::CpuGenerator<mnd::DoubleDouble, mnd::ARM_NEON, parallel> : public MandelGenerator
{
public:
    inline CpuGenerator(void) :
        MandelGenerator{ mnd::Precision::DOUBLE_DOUBLE, mnd::ARM_NEON }
    {
    }
    virtual void generate(const MandelInfo& info, float* data);
};

#endif


#endif // MANDEL_CPUGENERATORS_H

