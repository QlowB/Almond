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

    enum class Precision : int
    {
        FLOAT,
        DOUBLE_FLOAT,
        DOUBLE,
        DOUBLE_DOUBLE,
        FLOAT128,
        FIXED64,
        FIXED128,
        QUAD_DOUBLE,
        FLOAT256,
        INF_PREC,
    };

    Real getPrecision(Precision p);
    
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
}


class mnd::Generator
{
protected:
    Real precision;
public:
    inline Generator(const Real& precision) :
        precision{ precision }
    {
    }

    virtual ~Generator(void);


    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;

    Generator(Generator&&) = default;
    Generator& operator=(Generator&&) = default;

    virtual void generate(const MandelInfo& info, float* data) = 0;
    virtual Real getPrecision(void) const;
};


class mnd::AdaptiveGenerator : public Generator
{
    std::map<Real, Generator*, std::greater<Real>> generators;
public:
    AdaptiveGenerator(void);
    AdaptiveGenerator(Generator* floatGen, Generator* doubleGen);
    virtual ~AdaptiveGenerator(void) = default;

    void addGenerator(const Real& precision, Generator& generator);
    void addGenerator(Precision p, Generator& generator);
    
    const std::map<Real, Generator*, std::greater<Real>>& getGenerators(void) const { return generators; }
    inline void clear(void) { generators.clear(); }

    virtual void generate(const MandelInfo& info, float* data) override;
};


#endif // MANDEL_GENERATORS_H
