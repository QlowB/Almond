#ifndef MANDEL_GENERATORS_H
#define MANDEL_GENERATORS_H

#include "MandelUtil.h"

#include <vector>
#include <map>
#include <utility>



namespace mnd
{
    class MandelGenerator;

    class AdaptiveGenerator;

    enum class GeneratorType : int;
    enum class Precision : int
    {
        FLOAT,
        DOUBLE_FLOAT,
        DOUBLE,
        DOUBLE_DOUBLE,
        FLOAT128,
        FLOAT256,
        FLOAT512,
        FIXED64,
        FIXED128,
        FIXED512,
        QUAD_DOUBLE,
        INF_PREC,
    };


    enum CpuExtension : int
    {
        NONE,
        X86_SSE2,
        X86_AVX,
        X86_AVX_FMA,
        X86_AVX_512,
        ARM_NEON,
    };

    Real getPrecision(Precision p);
    Real getPrecision(GeneratorType p);
    
    template<typename T>
    Real getPrecision(void);

    template<> Real getPrecision<float>();
    template<> Real getPrecision<double>();
    template<> Real getPrecision<DoubleDouble>();
    template<> Real getPrecision<QuadDouble>();
    template<> Real getPrecision<Fixed64>();
    template<> Real getPrecision<Fixed128>();
    template<> Real getPrecision<Fixed512>();
    template<> Real getPrecision<Float128>();
    template<> Real getPrecision<Float256>();
    template<> Real getPrecision<Float512>();

    template<typename T>
    Precision getType(void);
    template<> inline Precision getType<float>() { return Precision::FLOAT; }
    template<> inline Precision getType<double>() { return Precision::DOUBLE; }
    template<> inline Precision getType<DoubleDouble>() { return Precision::DOUBLE_DOUBLE; }
    template<> inline Precision getType<QuadDouble>() { return Precision::QUAD_DOUBLE; }
    template<> inline Precision getType<Fixed64>() { return Precision::FIXED64; }
    template<> inline Precision getType<Fixed128>() { return Precision::FIXED128; }
    template<> inline Precision getType<Fixed512>() { return Precision::FIXED512; }
    template<> inline Precision getType<Float128>() { return Precision::FLOAT128; }
    template<> inline Precision getType<Float256>() { return Precision::FLOAT256; }
    template<> inline Precision getType<Float512>() { return Precision::FLOAT512; }
}


enum class mnd::GeneratorType : int
{
    UNSPECIFIED,
    FLOAT,
    FLOAT_SSE2,
    FLOAT_AVX,
    FLOAT_AVX_FMA,
    FLOAT_AVX512,
    FLOAT_NEON,
    DOUBLE_FLOAT,
    DOUBLE,
    DOUBLE_SSE2,
    DOUBLE_AVX,
    DOUBLE_AVX_FMA,
    DOUBLE_AVX512,
    DOUBLE_NEON,
    DOUBLE_DOUBLE,
    DOUBLE_DOUBLE_AVX,
    DOUBLE_DOUBLE_AVX_FMA,
    QUAD_DOUBLE,
    FLOAT128,
    FLOAT256,
    FIXED64,
    FIXED128,
    FIXED512
};


class mnd::MandelGenerator
{
protected:
    Real precision;
    Precision type;
    CpuExtension extension;
public:
    MandelGenerator();
    inline MandelGenerator(Precision type) :
        type{ type },
        precision{ mnd::getPrecision(type) },
        extension{ mnd::CpuExtension::NONE }
    {
    }

    inline MandelGenerator(Precision type, CpuExtension extension) :
        type{ type },
        precision{ mnd::getPrecision(type) },
        extension{ extension }
    {
    }

    inline MandelGenerator(Precision type, CpuExtension extension, const Real& precision) :
        type{ type },
        precision{ precision },
        extension{ extension }
    {
    }

    virtual ~MandelGenerator(void);


    MandelGenerator(const MandelGenerator&) = delete;
    MandelGenerator& operator=(const MandelGenerator&) = delete;

    MandelGenerator(MandelGenerator&&) = default;
    MandelGenerator& operator=(MandelGenerator&&) = default;

    virtual void generate(const MandelInfo& info, float* data) = 0;
    virtual Real getPrecision(void) const;
};


class mnd::AdaptiveGenerator : public MandelGenerator
{
    std::map<Real, MandelGenerator*, std::greater<Real>> generators;
public:
    AdaptiveGenerator(void);
    AdaptiveGenerator(MandelGenerator* floatGen, MandelGenerator* doubleGen);
    virtual ~AdaptiveGenerator(void) = default;

    void addGenerator(const Real& precision, MandelGenerator& generator);
    void addGenerator(Precision p, MandelGenerator& generator);
    
    const std::map<Real, MandelGenerator*, std::greater<Real>>& getGenerators(void) const { return generators; }
    inline void clear(void) { generators.clear(); }

    virtual void generate(const MandelInfo& info, float* data) override;
};


#endif // MANDEL_GENERATORS_H
