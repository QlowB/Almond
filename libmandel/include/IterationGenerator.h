#ifndef MANDEL_ITERATIONGENERATOR_H
#define MANDEL_ITERATIONGENERATOR_H

#include "Generators.h"
#include "ClGenerators.h"
#include "IterationFormula.h"
#include "IterationIR.h"

#include <utility>
#include <complex>

namespace mnd
{
    class IterationGenerator;

    class NaiveGenerator;

    class CompiledGenerator;
    class CompiledGeneratorVec;
    class CompiledClGenerator;
    class CompiledClGeneratorDouble;
    class CompiledClGeneratorDoubleDouble;
    class CompiledClGeneratorQuadDouble;

    // forward declaration
    struct ExecData;
    class MandelDevice;
}


class mnd::IterationGenerator : public mnd::MandelGenerator
{
protected:
    IterationFormula z0;
    IterationFormula zi;
public:
    IterationGenerator(IterationFormula z0, IterationFormula zi,
            mnd::Precision prec,
            mnd::HardwareFeature ex = mnd::HardwareFeature::NONE);
};


class mnd::NaiveGenerator : public mnd::IterationGenerator
{
public:
    NaiveGenerator(IterationFormula z0, IterationFormula zi,
            mnd::Precision prec,
            mnd::HardwareFeature ex = mnd::HardwareFeature::NONE);

    virtual void generate(const MandelInfo& info, float* data);
private:
    std::complex<double> iterate(std::complex<double> z, std::complex<double> c);
    std::complex<double> calc(mnd::Expression& expr, std::complex<double> z, std::complex<double> c);
};


#ifdef WITH_ASMJIT
#if defined(__x86_64__) || defined(_M_X64)
class mnd::CompiledGenerator : public mnd::MandelGenerator
{
protected:
    std::unique_ptr<ExecData> execData;
public:
    CompiledGenerator(std::unique_ptr<ExecData> execData,
        mnd::Precision prec = mnd::Precision::DOUBLE,
        mnd::HardwareFeature ex = mnd::HardwareFeature::NONE);
    CompiledGenerator(const CompiledGenerator&) = delete;
    CompiledGenerator(CompiledGenerator&&);
    virtual ~CompiledGenerator(void);
    virtual void generate(const MandelInfo& info, float* data) override;

    std::string dump(void) const;
};


class mnd::CompiledGeneratorVec : public mnd::CompiledGenerator
{
public:
    CompiledGeneratorVec(std::unique_ptr<ExecData> execData);
    CompiledGeneratorVec(const CompiledGeneratorVec&) = delete;
    CompiledGeneratorVec(CompiledGeneratorVec&&);
    virtual ~CompiledGeneratorVec(void);
    virtual void generate(const MandelInfo& info, float* data) override;
};
#endif
#endif // WITH_ASMJIT


#ifdef WITH_OPENCL
class mnd::CompiledClGenerator : public mnd::ClGeneratorFloat
{
public:
    CompiledClGenerator(MandelDevice& device, const std::string& code);
    virtual void generate(const MandelInfo& info, float* data) override;
};

class mnd::CompiledClGeneratorDouble : public mnd::ClGeneratorDouble
{
public:
    CompiledClGeneratorDouble(MandelDevice& device, const std::string& code);
};

class mnd::CompiledClGeneratorDoubleDouble : public mnd::ClGeneratorDoubleDouble
{
public:
    CompiledClGeneratorDoubleDouble(MandelDevice& device, const std::string& code);
};

class mnd::CompiledClGeneratorQuadDouble : public mnd::ClGeneratorQuadDouble
{
public:
    inline CompiledClGeneratorQuadDouble(MandelDevice& device, const std::string& code) :
        ClGeneratorQuadDouble{ device, code } {}
};

#endif // WITH_OPENCL



#endif // MANDEL_ITERATIONGENERATOR_H

