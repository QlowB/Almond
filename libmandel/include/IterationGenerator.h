#ifndef MANDEL_ITERATIONGENERATOR_H
#define MANDEL_ITERATIONGENERATOR_H

#include "Generators.h"
#include "ClGenerators.h"
#include "IterationFormula.h"

#include <utility>
#include <complex>

namespace mnd
{
    class IterationGenerator;

    class NaiveGenerator;
    class CompiledGenerator;
    class CompiledClGenerator;

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
    IterationGenerator(IterationFormula z0, IterationFormula zi, const mnd::Real& prec);
};


class mnd::NaiveGenerator : public mnd::IterationGenerator
{
public:
    NaiveGenerator(IterationFormula z0, IterationFormula zi, const mnd::Real& prec);
    NaiveGenerator(NaiveGenerator&&) = default;

    virtual void generate(const MandelInfo& info, float* data);
private:
    std::complex<double> iterate(std::complex<double> z, std::complex<double> c);
    std::complex<double> calc(mnd::Expression& expr, std::complex<double> z, std::complex<double> c);
};


#if defined(__x86_64__) || defined(_M_X64)
class mnd::CompiledGenerator : public mnd::MandelGenerator
{
    std::unique_ptr<ExecData> execData;
public:
    CompiledGenerator(std::unique_ptr<ExecData> execData);
    CompiledGenerator(CompiledGenerator&&);
    virtual ~CompiledGenerator(void);
    virtual void generate(const MandelInfo& info, float* data);

    std::string dump(void) const;
};
#endif


#ifdef WITH_OPENCL
class mnd::CompiledClGenerator : public mnd::ClGeneratorFloat
{
public:
    CompiledClGenerator(const MandelDevice& device, const std::string& code);
    CompiledClGenerator(CompiledClGenerator&&) = default;
    //virtual ~CompiledGenerator(void);
    //virtual void generate(const MandelInfo& info, float* data);
    virtual std::string getKernelCode(bool smooth) const override;
    virtual void generate(const MandelInfo& info, float* data);

    //std::string dump(void) const;
};
#endif // WITH_OPENCL



#endif // MANDEL_ITERATIONGENERATOR_H

