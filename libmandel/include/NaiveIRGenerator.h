#ifndef MANDEL_NAIVEIRGENERATOR_H
#define MANDEL_NAIVEIRGENERATOR_H

#include "IterationIR.h"
#include "Generators.h"

namespace mnd
{
    template<typename T>
    class NaiveIRGenerator;
}


template<typename T>
class mnd::NaiveIRGenerator : public mnd::MandelGenerator
{
    const ir::Formula& form;
public:
    NaiveIRGenerator(const ir::Formula& irf, mnd::Precision prec = mnd::getType<T>());
    NaiveIRGenerator(NaiveIRGenerator&&) = default;

    virtual void generate(const MandelInfo& info, float* data);
};


#endif // MANDEL_NAIVEIRGENERATOR_H
