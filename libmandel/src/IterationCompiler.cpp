#include "IterationCompiler.h"

#include "Mandel.h"
#include "OpenClInternal.h"
#include "OpenClCode.h"

#include <asmjit/asmjit.h>
#include <cmath>
#include <omp.h>
#include <any>
#include <string>

using namespace std::string_literals;
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

        Reg visitNode(ir::Node& node)
        {
            auto& nodeData = std::visit([] (auto& n) -> std::any& { return n.nodeData; }, node);
            if (Reg* regPtr = std::any_cast<Reg>(&nodeData)) {
                return *regPtr;
            }
            else {
                Reg reg = std::visit(*this, node);
                nodeData = reg;
                return reg;
            }
        }

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
            cc.subsd(sub, visitNode(*n.value));
            return sub;
        }

        Reg operator()(const ir::Addition& add) {
            auto res = cc.newXmmSd();
            cc.movapd(res, visitNode(*add.left));
            cc.addsd(res, visitNode(*add.right));
            return res;
        }

        Reg operator()(const ir::Subtraction& add) {
            auto res = cc.newXmmSd();
            cc.movapd(res, visitNode(*add.left));
            cc.subsd(res, visitNode(*add.right));
            return res;
        }

        Reg operator()(const ir::Multiplication& add) {
            auto res = cc.newXmmSd();
            cc.movapd(res, visitNode(*add.left));
            cc.mulsd(res, visitNode(*add.right));
            return res;
        }

        Reg operator()(const ir::Atan2& at2) {
            using namespace asmjit;
            auto y = visitNode(*at2.left);
            auto x = visitNode(*at2.right);

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
            auto a = visitNode(*p.left);
            auto b = visitNode(*p.right);

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
            auto a = visitNode(*c.value);

            auto arg = cc.newXmmSd();
            double(*cosFunc)(double) = ::cos;
            auto call = cc.call(imm(cosFunc), FuncSignatureT<double, double>(CallConv::kIdHostCDecl));
            call->setArg(0, a);
            call->setRet(0, arg);
            return arg;
        }

        Reg operator()(const ir::Sin& s) {
            using namespace asmjit;
            auto a = visitNode(*s.value);

            auto arg = cc.newXmmSd();
            double(*sinFunc)(double) = ::sin;
            auto call = cc.call(imm(sinFunc), FuncSignatureT<double, double>(CallConv::kIdHostCDecl));
            call->setArg(0, a);
            call->setRet(0, arg);
            return arg;
        }

        Reg operator()(const ir::Exp& ex) {
            using namespace asmjit;
            auto a = visitNode(*ex.value);

            auto arg = cc.newXmmSd();
            double(*expFunc)(double) = ::exp;
            auto call = cc.call(imm(expFunc), FuncSignatureT<double, double>(CallConv::kIdHostCDecl));
            call->setArg(0, a);
            call->setRet(0, arg);
            return arg;
        }

        Reg operator()(const ir::Ln& l) {
            using namespace asmjit;
            auto a = visitNode(*l.value);

            auto arg = cc.newXmmSd();
            double(*logFunc)(double) = ::log;
            auto call = cc.call(imm(logFunc), FuncSignatureT<double, double>(CallConv::kIdHostCDecl));
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


    struct OpenClVisitor
    {
        int varnameCounter = 0;
        std::stringstream code;

        std::string createVarname(void)
        {
            return "tmp"s + std::to_string(varnameCounter++);
        }

        std::string visitNode(ir::Node& node)
        {
            auto& nodeData = std::visit([] (auto& n) -> std::any& { return n.nodeData; }, node);
            if (std::string* var = std::any_cast<std::string>(&nodeData)) {
                return *var;
            }
            else {
                std::string value = std::visit(*this, node);
                std::string varname = createVarname();
                code << "float " << varname << " = " << value << ";" << std::endl;
                nodeData = varname;
                return varname;
            }
        }

        std::string operator()(const ir::Constant& c) {
            return std::to_string(c.value);
        }

        std::string operator()(const ir::Variable& v) {
            return v.name;
        }

        std::string operator()(const ir::Negation& n) {
            return "-("s + visitNode(*n.value) + ")";
        }

        std::string operator()(const ir::Addition& a) {
            return "("s + visitNode(*a.left) + ") + (" + visitNode(*a.right) + ")";
        }

        std::string operator()(const ir::Subtraction& a) {
            return "("s + visitNode(*a.left) + ") - (" + visitNode(*a.right) + ")";
        }

        std::string operator()(const ir::Multiplication& a) {
            return "("s + visitNode(*a.left) + ") * (" + visitNode(*a.right) + ")";
        }

        std::string operator()(const ir::Atan2& a) {
            return "atan2("s + visitNode(*a.left) + ", " + visitNode(*a.right) + ")";
        }

        std::string operator()(const ir::Pow& a) {
            return "pow("s + visitNode(*a.left) + ", " + visitNode(*a.right) + ")";
        }

        std::string operator()(const ir::Cos& a) {
            return "cos("s + visitNode(*a.value) + ")";
        }

        std::string operator()(const ir::Sin& a) {
            return "sin("s + visitNode(*a.value) + ")";
        }

        std::string operator()(const ir::Exp& a) {
            return "exp("s + visitNode(*a.value) + ")";
        }

        std::string operator()(const ir::Ln& a) {
            return "ln("s + visitNode(*a.value) + ")";
        }
    };

    std::string compileToOpenCl(const ir::Formula& formula)
    {
        OpenClVisitor ocv;
        std::string newA = ocv.visitNode(*formula.newA);
        std::string newB = ocv.visitNode(*formula.newB);
        std::string prelude = 
"__kernel void iterate(__global float* A, const int width, float xl, float yt, float pixelScaleX, float pixelScaleY, int max, int smooth, int julia, float juliaX, float juliaY) {\n"
"   int index = get_global_id(0);\n"
"   int ix = index % width;\n"
"   int iy = index / width;\n"
"   float a = ix * pixelScaleX + xl;\n"
"   float b = iy * pixelScaleY + yt;\n"
"   float x = a;\n"
"   float y = b;\n"
"\n"
"   int n = 0;\n"
"   while (n < max - 1) {\n";

        std::string orig = 
"       float aa = a * a;"
"       float bb = b * b;"
"       float ab = a * b;"
"       a = aa - bb + x;"
"       b = ab + ab + y;";

    
        std::string after = 
"       if (a * a + b * b > 16) break;\n"
"       n++;\n"
"   }\n"
"   if (n >= max - 1) {\n"
"       A[index] = max;\n"
"   }\n"
"   else {\n"
"       A[index] = ((float)n);\n"
"   }\n"
"}\n";


        std::string code = prelude + ocv.code.str();
        code += "a = " + newA + ";\n";
        code += "b = " + newB + ";\n";
        code += after;
        //code = mnd::getFloat_cl();
        printf("cl: %s\n", code.c_str());
        return code;
    }

#ifdef WITH_OPENCL
    std::unique_ptr<MandelGenerator> compileCl(const ir::Formula& formula, const MandelDevice& md)
    {
        return std::make_unique<CompiledClGenerator>(md, compileToOpenCl(formula));
    }
#endif
}


using mnd::CompiledGenerator;
using mnd::CompiledClGenerator;



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


#ifdef WITH_OPENCL
CompiledClGenerator::CompiledClGenerator(const mnd::MandelDevice& device, const std::string& code) :
    ClGeneratorFloat{ device.getClDevice().device, code }
{
}


std::string CompiledClGenerator::getKernelCode(bool smooth) const
{
    return "";
}

void CompiledClGenerator::generate(const mnd::MandelInfo& info, float* data)
{
    ::size_t bufferSize = info.bWidth * info.bHeight * sizeof(float);

    cl::Buffer buffer_A(context, CL_MEM_WRITE_ONLY, bufferSize);
    float pixelScaleX = float(info.view.width / info.bWidth);
    float pixelScaleY = float(info.view.height / info.bHeight);

    static cl::Kernel iterate = cl::Kernel(program, "iterate");
    iterate.setArg(0, buffer_A);
    iterate.setArg(1, int(info.bWidth));
    iterate.setArg(2, float(info.view.x));
    iterate.setArg(3, float(info.view.y));
    iterate.setArg(4, float(pixelScaleX));
    iterate.setArg(5, float(pixelScaleY));
    iterate.setArg(6, int(info.maxIter));
    iterate.setArg(7, int(info.smooth ? 1 : 0));
    iterate.setArg(8, int(info.julia ? 1 : 0));
    iterate.setArg(9, float(info.juliaX));
    iterate.setArg(10, float(info.juliaY));

    queue.enqueueNDRangeKernel(iterate, 0, cl::NDRange(info.bWidth * info.bHeight));

    queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, bufferSize, data);
}

#endif // WITH_OPENCL

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



