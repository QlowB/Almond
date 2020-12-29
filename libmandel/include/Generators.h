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

    enum class Precision : int
    {
        FLOAT,
        DOUBLE_FLOAT,
        TRIPLE_FLOAT,
        DOUBLE,
        DOUBLE_DOUBLE,
        TRIPLE_DOUBLE,
        QUAD_DOUBLE,
        HEX_DOUBLE,
        OCTA_DOUBLE,
        FLOAT128,
        FLOAT256,
        FLOAT512,
        FIXED64,
        FIXED128,
        FIXED512,
        INF_PREC,
    };


    enum HardwareFeature : int
    {
        NONE,
        X86_SSE2,
        X86_AVX,
        X86_AVX_FMA,
        X86_AVX_512,
        ARM_NEON,
    };

    std::string toString(Precision);
    std::string toString(HardwareFeature);

    Real getPrecision(Precision p);

    template<typename T>
    Real getPrecision(void);

    template<typename T>
    Precision getType(void);

    class MandelDevice;
}


class mnd::MandelGenerator
{
protected:
    Real precision;
    Precision type;
    HardwareFeature extension;
public:
    MandelGenerator(void);
    inline MandelGenerator(Precision type) :
        precision{ mnd::getPrecision(type) },
        type{ type },
        extension{ mnd::HardwareFeature::NONE }
    {
    }

    inline MandelGenerator(Precision type, HardwareFeature extension) :
        precision{ mnd::getPrecision(type) },
        type{ type },
        extension{ extension }
    {
    }

    inline MandelGenerator(Precision type, HardwareFeature extension, const Real& precision) :
        precision{ precision },
        type{ type },
        extension{ extension }
    {
    }

    virtual ~MandelGenerator(void);


    MandelGenerator(const MandelGenerator&) = default;
    MandelGenerator& operator=(const MandelGenerator&) = default;

    MandelGenerator(MandelGenerator&&) = default;
    MandelGenerator& operator=(MandelGenerator&&) = default;

    virtual void generate(const MandelInfo& info, float* data) = 0;

    virtual mnd::MandelDevice* getDevice(void);

    virtual Real getPrecision(void) const;
    virtual Precision getType(void) const;
    virtual HardwareFeature getExtension(void) const;
};


class mnd::AdaptiveGenerator : public MandelGenerator
{
    std::map<Real, MandelGenerator*, std::greater<Real>> generators;
public:
    AdaptiveGenerator(void);
    AdaptiveGenerator(AdaptiveGenerator&) = delete;
    AdaptiveGenerator(AdaptiveGenerator&&) = default;
    AdaptiveGenerator(MandelGenerator* floatGen, MandelGenerator* doubleGen);
    virtual ~AdaptiveGenerator(void) = default;

    void addGenerator(const Real& precision, MandelGenerator& generator);
    void addGenerator(Precision p, MandelGenerator& generator);
    void addGenerator(MandelGenerator& generator);
    
    const std::map<Real, MandelGenerator*, std::greater<Real>>& getGenerators(void) const { return generators; }
    inline void clear(void) { generators.clear(); }

    virtual void generate(const MandelInfo& info, float* data) override;
};


#endif // MANDEL_GENERATORS_H
