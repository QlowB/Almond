#include "IterationCompiler.h"

#include <asmjit/asmjit.h>
#include "Mandel.h"

using namespace asmjit;

struct Visitor
{

};

void compile(mnd::MandelContext& mndCtxt)
{
    JitRuntime& jitRuntime = mndCtxt.getJitRuntime();

    CodeHolder code;
    code.init(jitRuntime.codeInfo());

    x86::Compiler compiler;
    x86::Gp maxIter;
    x86::Gp k;
    x86::Xmm x;
    x86::Xmm y;
    x86::Xmm a;
    x86::Xmm b;
    compiler.addFunc(FuncSignatureT<int, double, double, int>(CallConv::kIdHost));
    compiler.setArg(0, x);
    compiler.setArg(1, x);
    compiler.setArg(2, maxIter);

    auto err = compiler.finalize();

}





