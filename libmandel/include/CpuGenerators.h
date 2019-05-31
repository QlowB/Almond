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
        ARM_NEON,
    };

    template<typename T, mnd::CpuExtension ex = mnd::NONE, bool parallel = true, bool smooth = true>
    class CpuGenerator;


    /*
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
#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM)
    class CpuGeneratorNeonFloat;
    class CpuGeneratorNeonDouble;
#endif
*/
}


template<typename T, mnd::CpuExtension ex = mnd::NONE, bool parallel = true, bool smooth = true>
class mnd::CpuGenerator : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};


template<typename T, bool parallel, bool smooth>
class mnd::CpuGenerator<T, mnd::NONE, parallel, smooth> : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};


#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86) 
template<bool parallel, bool smooth>
class mnd::CpuGenerator<float, mnd::X86_SSE2, parallel, smooth> : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};

template<bool parallel, bool smooth>
class mnd::CpuGenerator<double, mnd::X86_SSE2, parallel, smooth> : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};


template<bool parallel, bool smooth>
class mnd::CpuGenerator<float, mnd::X86_AVX, parallel, smooth> : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};

template<bool parallel, bool smooth>
class mnd::CpuGenerator<double, mnd::X86_AVX, parallel, smooth> : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};

#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) 
template<typename T, bool parallel, bool smooth>
class mnd::CpuGenerator<T, mnd::ARM_NEON, parallel, smooth> : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};
#endif

/*
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

#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM)

class mnd::CpuGeneratorNeonFloat : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};


class mnd::CpuGeneratorNeonDouble : public Generator
{
public:
    virtual void generate(const MandelInfo& info, float* data);
};
#endif

*/

#endif // MANDEL_CPUGENERATORS_H
