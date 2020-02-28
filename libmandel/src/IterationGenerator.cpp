#include "IterationGenerator.h"


using mnd::IterationGenerator;
using mnd::IterationFormula;


IterationGenerator::IterationGenerator(IterationFormula itf,
                                       const mnd::Real& prec) :
    mnd::MandelGenerator{ prec },
    itf{ std::move(itf) }
{
}





