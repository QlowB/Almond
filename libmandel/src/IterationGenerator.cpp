#include "IterationGenerator.h"


using mnd::IterationGenerator;
using mnd::IterationFormula;


IterationGenerator::IterationGenerator(const IterationFormula& itf,
                                       const mnd::Real& prec) :
    mnd::MandelGenrator{ prec },
    itf{ itf }
{
}





