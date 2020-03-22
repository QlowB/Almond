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


#ifdef WITH_OPENCL
class mnd::CompiledClGenerator : public mnd::ClGeneratorFloat
{
public:
    CompiledClGenerator(const MandelDevice& device, const std::string& code);
    //virtual ~CompiledGenerator(void);
    //virtual void generate(const MandelInfo& info, float* data);
    virtual std::string getKernelCode(bool smooth) const override;
    virtual void generate(const MandelInfo& info, float* data);

    //std::string dump(void) const;
};
#endif // WITH_OPENCL


namespace mnd
{
    CompiledGenerator compile(const ir::Formula& formula);
    std::unique_ptr<MandelGenerator> compileCl(const ir::Formula& formula, const MandelDevice& md);
}


#endif // MANDEL_ITERATIONCOMPILER_H

