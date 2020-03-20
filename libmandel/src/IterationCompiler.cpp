#include "IterationCompiler.h"

#include <asmjit/asmjit.h>
#include "Mandel.h"
#include <omp.h>

namespace mnd
{
    struct ExecData
    {
        std::unique_ptr<asmjit::JitRuntime> jitRuntime;
        std::unique_ptr<asmjit::CodeHolder> code;
        std::unique_ptr<asmjit::x86::Compiler> compiler;
        void* iterationFunc;

        ExecData(void) :
            jitRuntime{ std::make_unique<asmjit::JitRuntime>() },
            code{ std::make_unique<asmjit::CodeHolder>() },
            compiler{ nullptr },
            iterationFunc{ nullptr }
        {
            code->init(jitRuntime->codeInfo());
            compiler = std::make_unique<asmjit::x86::Compiler>(code.get());
        }

        ExecData(ExecData&&) = default;
        ExecData(const ExecData&) = delete;
        ExecData& operator=(ExecData&&) = default;
        ExecData& operator=(const ExecData&) = delete;

        ~ExecData(void) = default;
    };
}


using mnd::CompiledGenerator;



CompiledGenerator::CompiledGenerator(mnd::MandelContext& mndContext) :
    MandelGenerator{ 1.0e-15 },
    execData{ std::make_unique<ExecData>(compile(mndContext)) }
{
}


CompiledGenerator::~CompiledGenerator(void)
{
}

    
/*__declspec(noinline)
int iter(double x, double y, int maxIter)
{
    int k = 0;

    double a = x;
    double b = y;

    for (k = 0; k < maxIter; k++) {
        double aa = a * a;
        double bb = b * b;
        double abab = a * b + a * b;
        a = aa - bb + x;
        b = abab + y;
        if (aa + bb >= 16)
            break;
    }

    return k;
}*/



void CompiledGenerator::generate(const mnd::MandelInfo& info, float* data)
{
    using IterFunc = int (*)(double, double, int);

    omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for schedule(static, 1)
    for (int i = 0; i < info.bHeight; i++) {
        double y = mnd::convert<double>(info.view.y + info.view.height * i / info.bHeight);
        for (int j = 0; j < info.bWidth; j++) {
            double x = mnd::convert<double>(info.view.x + info.view.width * j / info.bWidth);
            IterFunc iterFunc = asmjit::ptr_as_func<IterFunc>(this->execData->iterationFunc);
            int k = iterFunc(x, y, info.maxIter);
            data[i * info.bWidth + j] = k;
        }
    }
}




std::string CompiledGenerator::dump(void) const
{
    asmjit::String d;
    execData->compiler->dump(d);
    return d.data();
}


using namespace asmjit;

struct Visitor
{

};


namespace mnd
{
    mnd::ExecData compile(mnd::MandelContext& mndCtxt)
    {

        mnd::ExecData ed;
        JitRuntime& jitRuntime = *ed.jitRuntime;
        ed.code->init(jitRuntime.codeInfo());

        x86::Compiler& comp = *ed.compiler;

        x86::Mem sixteen = comp.newDoubleConst(ConstPool::kScopeLocal, 16.0);

        Label startLoop = comp.newLabel();
        Label endLoop = comp.newLabel();

        x86::Gp maxIter = comp.newInt32();
        x86::Gp k = comp.newInt32();
        x86::Xmm x = comp.newXmmSd();
        x86::Xmm y = comp.newXmmSd();
        x86::Xmm a = comp.newXmmSd();
        x86::Xmm b = comp.newXmmSd();
        x86::Xmm aa = comp.newXmmSd();
        x86::Xmm bb = comp.newXmmSd();
        x86::Xmm t = comp.newXmmSd();
        x86::Xmm sumSq = comp.newXmmSd();

        comp.addFunc(FuncSignatureT<int, double, double, int>(CallConv::kIdHost));
        comp.setArg(0, x);
        comp.setArg(1, y);
        comp.setArg(2, maxIter);

        comp.movapd(a, x);
        comp.movapd(b, y);

        comp.xor_(k, k);

        comp.bind(startLoop);
        comp.movapd(aa, a);
        comp.movapd(bb, b);
        comp.mulsd(aa, a);
        comp.mulsd(bb, b);
        comp.mulsd(b, a);
        comp.movapd(sumSq, aa);
        comp.addsd(sumSq, bb);
        comp.addsd(b, b);
        comp.addsd(aa, x);
        comp.comisd(sumSq, sixteen);
        comp.addsd(b, y);
        comp.subsd(aa, bb);
        comp.jle(endLoop);
        comp.movapd(a, aa);

        comp.inc(k);
        comp.cmp(k, maxIter);
        comp.jne(startLoop);
        comp.bind(endLoop);
        comp.ret(k);
        comp.endFunc();
        auto err = comp.finalize();
        if (err == asmjit::kErrorOk) {
            err = jitRuntime.add(&ed.iterationFunc, ed.code.get());
            if (err != asmjit::kErrorOk)
                throw "error adding function";
        }
        else {
            throw "error compiling";
        }
        return ed;
    }
}




