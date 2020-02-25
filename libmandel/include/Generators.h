#ifndef MANDEL_GENERATORS_H
#define MANDEL_GENERATORS_H

#include "MandelUtil.h"

#include <vector>
#include <map>
#include <utility>



namespace mnd
{
    class MandelGenerator;
    class JuliaGenerator;

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


class mnd::MandelGenerator
{
protected:
    Real precision;
public:
    inline MandelGenerator(const Real& precision) :
        precision{ precision }
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


class mnd::JuliaGenerator : public MandelGenerator
{
public:
    inline JuliaGenerator(const Real& precision) :
        MandelGenerator{ precision }
    {
    }

    virtual ~JuliaGenerator(void);


    JuliaGenerator(const JuliaGenerator&) = delete;
    JuliaGenerator& operator=(const JuliaGenerator&) = delete;

    JuliaGenerator(JuliaGenerator&&) = default;
    JuliaGenerator& operator=(JuliaGenerator&&) = default;

    virtual void generate(const MandelInfo& info, float* data) = 0;
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
