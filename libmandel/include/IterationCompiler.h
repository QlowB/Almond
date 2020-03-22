#ifndef MANDEL_ITERATIONCOMPILER_H
#define MANDEL_ITERATIONCOMPILER_H

#include "Generators.h"
#include "IterationIR.h"

namespace mnd
{
    struct ExecData;
    class CompiledGenerator;

    class MandelContext;

    mnd::ExecData compile(mnd::MandelContext& mndCtxt);

 
}
void squareTest();

class mnd::CompiledGenerator : public mnd::MandelGenerator
{
    std::unique_ptr<ExecData> execData;
public:
    CompiledGenerator(MandelContext& mndContext);
    CompiledGenerator(std::unique_ptr<ExecData> execData);
    CompiledGenerator(CompiledGenerator&&);
    virtual ~CompiledGenerator(void);
    virtual void generate(const MandelInfo& info, float* data);

    std::string dump(void) const;
};

namespace mnd
{
    CompiledGenerator compile(const ir::Formula& formula);
}


#endif // MANDEL_ITERATIONCOMPILER_H

