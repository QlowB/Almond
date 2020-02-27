#ifndef MANDEL_ITERATIONGENERATOR_H
#define MANDEL_ITERATIONGENERATOR_H

#include "Generators.h"
#include "IterationFormula.h"


namespace mnd
{
    class IterationGenerator;
}


class mnd::IterationGenerator : public mnd::MandelGenerator
{
    IterationFormula itf;
public:
    IterationGenerator(const IterationFormula& itf, const mnd::Real& prec);
};


#endif // MANDEL_ITERATIONGENERATOR_H

