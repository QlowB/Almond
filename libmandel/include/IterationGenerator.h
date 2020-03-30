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

    template<typename T>
    class NaiveIRGenerator;
    class CompiledGenerator;
    class CompiledClGenerator;
    class CompiledClGeneratorDouble;

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


template<typename T>
class mnd::NaiveIRGenerator : public mnd::MandelGenerator
{
    const ir::Formula& form;
public:
    NaiveIRGenerator(const ir::Formula& irf, const mnd::Real& prec);
    NaiveIRGenerator(NaiveIRGenerator&&) = default;

    virtual void generate(const MandelInfo& info, float* data);
    double calc(ir::Node* expr, double a, double b, double x, double y);
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
    CompiledClGenerator(MandelDevice& device, const std::string& code);
    CompiledClGenerator(CompiledClGenerator&&) = default;
    virtual void generate(const MandelInfo& info, float* data);
};

class mnd::CompiledClGeneratorDouble : public mnd::ClGeneratorDouble
{
public:
    CompiledClGeneratorDouble(MandelDevice& device, const std::string& code);
    CompiledClGeneratorDouble(CompiledClGeneratorDouble&&) = default;
    virtual void generate(const MandelInfo& info, float* data);
};
#endif // WITH_OPENCL



#endif // MANDEL_ITERATIONGENERATOR_H

