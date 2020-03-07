#ifndef MANDEL_ITERATIONGENERATOR_H
#define MANDEL_ITERATIONGENERATOR_H

#include "Generators.h"
#include "IterationFormula.h"

#include <utility>
#include <complex>

namespace mnd
{
    class IterationGenerator;

    class NaiveGenerator;
}


class mnd::IterationGenerator : public mnd::MandelGenerator
{
protected:
    IterationFormula itf;
public:
    IterationGenerator(IterationFormula itf, const mnd::Real& prec);
};


class mnd::NaiveGenerator : public mnd::IterationGenerator
{
public:
    NaiveGenerator(IterationFormula itf, const mnd::Real& prec);

    virtual void generate(const MandelInfo& info, float* data);
private:
    std::complex<double> iterate(std::complex<double> z, std::complex<double> c);
    std::complex<double> calc(mnd::Expression& expr, std::complex<double> z, std::complex<double> c);
};

#endif // MANDEL_ITERATIONGENERATOR_H

