#include "IterationCompiler.h"

#include <asmjit/asmjit.h>
#include <cmath>
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


    struct CompileVisitor
    {
        using Reg = asmjit::x86::Xmm;

        asmjit::x86::Compiler& cc;
        Reg& a;
        Reg& b;
        Reg& x;
        Reg& y;

        CompileVisitor(asmjit::x86::Compiler& cc, Reg& a, Reg& b, Reg& x, Reg& y) :
            cc{ cc },
            a{ a }, b{ b },
            x{ x }, y{ y }
        {
        }

        Reg operator()(const ir::Constant& c) {
            auto constant = cc.newDoubleConst(asmjit::ConstPool::kScopeLocal, c.value);
            auto reg = cc.newXmmSd();
            cc.movsd(reg, constant);
            return reg;
        }

        Reg operator()(const ir::Variable& v) {
            if (v.name == "a") {
                return a;
            }
            else if (v.name == "b") {
                return b;
            }
            else if (v.name == "x") {
                return x;
            }
            else if (v.name == "y") {
                return y;
            }
            else
                throw "unknown variable";
        }

        Reg operator()(const ir::Negation& n) {
            auto sub = cc.newXmmSd();
            cc.xorpd(sub, sub);
            cc.subsd(sub, std::visit((*this), *n.value));
            return sub;
        }

        Reg operator()(const ir::Addition& add) {
            auto res = cc.newXmmSd();
            cc.movapd(res, std::visit((*this), *add.left));
            cc.addsd(res, std::visit((*this), *add.right));
            return res;
        }

        Reg operator()(const ir::Subtraction& add) {
            auto res = cc.newXmmSd();
            cc.movapd(res, std::visit((*this), *add.left));
            cc.subsd(res, std::visit((*this), *add.right));
            return res;
        }

        Reg operator()(const ir::Multiplication& add) {
            auto res = cc.newXmmSd();
            cc.movapd(res, std::visit((*this), *add.left));
            cc.mulsd(res, std::visit((*this), *add.right));
            return res;
        }

        Reg operator()(const ir::Atan2& at2) {
            using namespace asmjit;
            auto y = std::visit((*this), *at2.left);
            auto x = std::visit((*this), *at2.right);

            auto arg = cc.newXmmSd();
            double(*atanFunc)(double, double) = ::atan2;
            auto call = cc.call(imm(atanFunc), FuncSignatureT<double, double, double>(CallConv::kIdHostCDecl));
            call->setArg(0, y);
            call->setArg(1, x);
            call->setRet(0, arg);
            return arg;
        }

        Reg operator()(const ir::Pow& p) {
            using namespace asmjit;
            auto a = std::visit((*this), *p.left);
            auto b = std::visit((*this), *p.right);

            auto arg = cc.newXmmSd();
            double(*powFunc)(double, double) = ::pow;
            auto call = cc.call(imm(powFunc), FuncSignatureT<double, double, double>(CallConv::kIdHostCDecl));
            call->setArg(0, a);
            call->setArg(1, b);
            call->setRet(0, arg);
            return arg;
        }

        Reg operator()(const ir::Cos& c) {
            using namespace asmjit;
            auto a = std::visit((*this), *c.value);

            auto arg = cc.newXmmSd();
            double(*cosFunc)(double) = ::cos;
            auto call = cc.call(imm(cosFunc), FuncSignatureT<double, double>(CallConv::kIdHostCDecl));
            call->setArg(0, a);
            call->setRet(0, arg);
            return arg;
        }

        Reg operator()(const ir::Sin& s) {
            using namespace asmjit;
            auto a = std::visit((*this), *s.value);

            auto arg = cc.newXmmSd();
            double(*sinFunc)(double) = ::sin;
            auto call = cc.call(imm(sinFunc), FuncSignatureT<double, double>(CallConv::kIdHostCDecl));
            call->setArg(0, a);
            call->setRet(0, arg);
            return arg;
        }
    };


    CompiledGenerator compile(const ir::Formula& formula)
    {
        using namespace asmjit;
        std::unique_ptr<mnd::ExecData> ed = std::make_unique<mnd::ExecData>();
        JitRuntime& jitRuntime = *ed->jitRuntime;
        ed->code->init(jitRuntime.codeInfo());

        x86::Compiler& comp = *ed->compiler;

        x86::Mem sixteen = comp.newDoubleConst(ConstPool::kScopeLocal, 16.0);

        Label startLoop = comp.newLabel();
        Label endLoop = comp.newLabel();
        x86::Gp maxIter = comp.newInt32();
        x86::Gp k = comp.newInt32();
        x86::Xmm x = comp.newXmmSd();
        x86::Xmm y = comp.newXmmSd();
        x86::Xmm a = comp.newXmmSd();
        x86::Xmm b = comp.newXmmSd();
        comp.addFunc(FuncSignatureT<int, double, double, int>(CallConv::kIdHost));
        comp.setArg(0, x);
        comp.setArg(1, y);
        comp.setArg(2, maxIter);
        comp.movapd(a, x);
        comp.movapd(b, y);

        comp.xor_(k, k);

        comp.bind(startLoop);

        CompileVisitor cv{ comp, a, b, x, y };
        auto newA = std::visit(cv, *formula.newA);
        auto newB = std::visit(cv, *formula.newB);
        comp.movapd(a, newA);
        comp.movapd(b, newB);

        x86::Xmm aa = comp.newXmmSd();
        x86::Xmm bb = comp.newXmmSd();
        comp.movapd(aa, a);
        comp.mulsd(aa, a);
        comp.movapd(bb, b);
        comp.mulsd(bb, b);
        comp.addsd(bb, aa);

        comp.comisd(bb, sixteen);
        comp.jle(endLoop);

        comp.inc(k);
        comp.cmp(k, maxIter);
        comp.jne(startLoop);
        comp.bind(endLoop);
        comp.ret(k);
        comp.endFunc();
        auto err = comp.finalize();
        if (err == asmjit::kErrorOk) {
            err = jitRuntime.add(&ed->iterationFunc, ed->code.get());
            if (err != asmjit::kErrorOk)
                throw "error adding function";
        }
        else {
            throw "error compiling";
        }
        return CompiledGenerator{ std::move(ed) };
    }
}


using mnd::CompiledGenerator;



CompiledGenerator::CompiledGenerator(mnd::MandelContext& mndContext) :
    MandelGenerator{ 1.0e-15 },
    execData{ std::make_unique<ExecData>(compile(mndContext)) }
{
}


CompiledGenerator::CompiledGenerator(std::unique_ptr<mnd::ExecData> execData) :
    MandelGenerator{ 1.0e-15 },
    execData{ std::move(execData) }
{
}


CompiledGenerator::CompiledGenerator(CompiledGenerator&&) = default;


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


    mnd::ExecData compile(mnd::MandelContext& mndCtxt, const IterationFormula& formula)
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
        comp.addFunc(FuncSignatureT<int, double, double, int>(CallConv::kIdHost));
        comp.setArg(0, x);
        comp.setArg(1, y);
        comp.setArg(2, maxIter);
        comp.movapd(a, x);
        comp.movapd(b, y);

        comp.xor_(k, k);

        comp.bind(startLoop);

        // loop body

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


void squareTest()
{
    mnd::Expression power = mnd::Pow{
        std::make_unique<mnd::Expression>(mnd::Variable{ "z" }),
        std::make_unique<mnd::Expression>(mnd::Constant{ 2.3 })
    };

    mnd::IterationFormula fmla(std::move(power));

    mnd::ir::Formula p = mnd::expand(fmla);

    mnd::ExecData ed;
    JitRuntime& jitRuntime = *ed.jitRuntime;
    ed.code->init(jitRuntime.codeInfo());

    x86::Compiler& comp = *ed.compiler;

    comp.addFunc(FuncSignatureT<double, double, double>(CallConv::kIdHost));
    x86::Xmm x = comp.newXmmSd();
    x86::Xmm y = comp.newXmmSd();
    x86::Xmm a = comp.newXmmSd();
    x86::Xmm b = comp.newXmmSd();
    comp.setArg(0, x);
    comp.setArg(1, y);
    comp.movapd(a, x);
    comp.movapd(b, y);


    mnd::CompileVisitor cv{ comp, a, b, x, y };
    auto newA = std::visit(cv, *p.newA);
    auto newB = std::visit(cv, *p.newB);
    comp.movapd(a, newA);
    comp.movapd(b, newB);
    comp.ret(b);
    comp.endFunc();
    comp.finalize();

    double (*func)(double, double);

    jitRuntime.add(&func, ed.code.get());

    double result = func(1.0, 3.0);
    printf("result: %f\n", result);
}



