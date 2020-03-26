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

    mnd::ExecData compile(mnd::MandelContext& mndCtxt);
}
void squareTest();



namespace mnd
{
    CompiledGenerator compile(const ir::Formula& formula);
    std::unique_ptr<MandelGenerator> compileCl(const ir::Formula& formula, const MandelDevice& md);
}


#endif // MANDEL_ITERATIONCOMPILER_H

