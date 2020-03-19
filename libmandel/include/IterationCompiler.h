#ifndef MANDEL_ITERATIONCOMPILER_H
#define MANDEL_ITERATIONCOMPILER_H

#include "Generators.h"

namespace mnd
{
    struct ExecData;
    class CompiledGenerator;

    class MandelContext;

    mnd::ExecData compile(mnd::MandelContext& mndCtxt);
}


class mnd::CompiledGenerator : public mnd::MandelGenerator
{
    std::unique_ptr<ExecData> execData;
public:
    CompiledGenerator(MandelContext& mndContext);
    virtual ~CompiledGenerator(void);
    virtual void generate(const MandelInfo& info, float* data);
};


#endif // MANDEL_ITERATIONCOMPILER_H

