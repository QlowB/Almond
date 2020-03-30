#ifndef MANDEL_ITERATIONCOMPILER_H
#define MANDEL_ITERATIONCOMPILER_H

#include "Generators.h"
#include "ClGenerators.h"
#include "IterationIR.h"
#include <memory>

namespace mnd
{
    struct ExecData;
    class CompiledGenerator;
    class CompiledClGenerator;

    // forward declare
    class MandelContext;
    class MandelDevice;

    enum class GeneratorType : int;

    //mnd::ExecData compile(mnd::MandelContext& mndCtxt);

    std::vector<std::unique_ptr<mnd::MandelGenerator>> compileCpu(mnd::MandelContext& mndCtxt,
        const IterationFormula& z0,
        const IterationFormula& zi);

    std::vector<std::unique_ptr<mnd::MandelGenerator>> compileOpenCl(mnd::MandelDevice& dev,
        const IterationFormula& z0,
        const IterationFormula& zi);
}
//void squareTest();



namespace mnd
{
    CompiledGenerator compile(const ir::Formula& formula);
    std::unique_ptr<MandelGenerator> compileCl(const ir::Formula& formula, MandelDevice& md);
}


#endif // MANDEL_ITERATIONCOMPILER_H

