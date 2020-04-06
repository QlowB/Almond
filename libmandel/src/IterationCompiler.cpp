#include "IterationCompiler.h"

#include "ExecData.h"
#include "Mandel.h"
#include "OpenClInternal.h"
#include "OpenClCode.h"

#include <cmath>
#include <omp.h>
#include <any>
#include <string>

using namespace std::string_literals;
namespace mnd
{
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
            auto constant = cc.newDoubleConst(asmjit::ConstPool::kScopeLocal, mnd::convert<double>(c.value));
            auto reg = cc.newXmmSd();
            std::string commentStr = "move constant [";
            commentStr += std::to_string(mnd::convert<double>(c.value));
            commentStr += "]";
            cc.comment(commentStr.c_str());
            cc.movsd(reg, constant);
            return reg;
        }

        Reg operator()(const ir::Variable& v) {
            if (v.name == "z_re") {
                return a;
            }
            else if (v.name == "z_im") {
                return b;
            }
            else if (v.name == "c_re") {
                return x;
            }
            else if (v.name == "c_im") {
                return y;
            }
            else
                throw mnd::ParseError(std::string("unknown variable: ") + v.name);
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

        Reg operator()(const ir::Division& add) {
            auto res = cc.newXmmSd();
            cc.movapd(res, visitNode(*add.left));
            cc.divsd(res, visitNode(*add.right));
            return res;
        }

        static double myAtan2(double y, double x)
        {
            double result = ::atan2(y, x);
            printf("atan2(%f, %f) = %f\n", y, x, result);
            return result;
        }

        Reg operator()(const ir::Atan2& at2) {
            using namespace asmjit;
            auto y = visitNode(*at2.left);
            auto x = visitNode(*at2.right);

            auto arg = cc.newXmmSd();
            double(*atanFunc)(double, double) = ::atan2;
            cc.comment("call atan2");
            auto call = cc.call(imm(atanFunc), FuncSignatureT<double, double, double>(CallConv::kIdHost));
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
            cc.comment("call pow");
            auto call = cc.call(imm(powFunc), FuncSignatureT<double, double, double>(CallConv::kIdHost));
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
            cc.comment("call cos");
            auto call = cc.call(imm(cosFunc), FuncSignatureT<double, double>(CallConv::kIdHost));
            call->setArg(0, a);
            call->setRet(0, arg);
            return arg;
        }

        Reg operator()(const ir::Sin& s) {
            using namespace asmjit;
            auto a = visitNode(*s.value);

            auto arg = cc.newXmmSd();
            double(*sinFunc)(double) = ::sin;
            cc.comment("call sin");
            auto call = cc.call(imm(sinFunc), FuncSignatureT<double, double>(CallConv::kIdHost));
            call->setArg(0, a);
            call->setRet(0, arg);
            return arg;
        }

        Reg operator()(const ir::Exp& ex) {
            using namespace asmjit;
            auto a = visitNode(*ex.value);

            auto arg = cc.newXmmSd();
            double(*expFunc)(double) = ::exp;
            cc.comment("call exp");
            auto call = cc.call(imm(expFunc), FuncSignatureT<double, double>(CallConv::kIdHost));
            call->setArg(0, a);
            call->setRet(0, arg);
            return arg;
        }

        Reg operator()(const ir::Ln& l) {
            using namespace asmjit;
            auto a = visitNode(*l.value);

            auto arg = cc.newXmmSd();
            double(*logFunc)(double) = ::log;
            cc.comment("call log");
            auto call = cc.call(imm(logFunc), FuncSignatureT<double, double>(CallConv::kIdHost));
            call->setArg(0, a);
            call->setRet(0, arg);
            return arg;
        }
    };


    static void printD(double d) {
        printf("val: %f\n", d); fflush(stdout);
    }

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

        CompileVisitor formVisitor{ comp, a, b, x, y };
        auto startA = std::visit(formVisitor, *formula.startA);
        auto startB = std::visit(formVisitor, *formula.startB);
        comp.movapd(a, startA);
        comp.movapd(b, startB);

        comp.setArg(2, maxIter);
        //comp.movapd(a, x);
        //comp.movapd(b, y);

        //comp.xorpd(a, a);
        //comp.movapd(b, b);

        comp.xor_(k, k);

        comp.bind(startLoop);

        auto newA = std::visit(formVisitor, *formula.newA);
        auto newB = std::visit(formVisitor, *formula.newB);
        comp.movapd(a, newA);
        comp.movapd(b, newB);

        x86::Xmm aa = comp.newXmmSd();
        x86::Xmm bb = comp.newXmmSd();
        comp.movapd(aa, a);
        comp.mulsd(aa, a);
        comp.movapd(bb, b);
        comp.mulsd(bb, b);
        comp.addsd(bb, aa);
        //auto call = comp.call(imm(printD), FuncSignatureT<void, double>(CallConv::kIdHost));
        //call->setArg(0, bb);

        comp.comisd(bb, sixteen);
        comp.jnb(endLoop);

        comp.inc(k);
        comp.cmp(k, maxIter);
        comp.jle(startLoop);
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


    struct CompileVisitorAVXFloat
    {
        using Reg = asmjit::x86::Ymm;

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

        CompileVisitorAVXFloat(asmjit::x86::Compiler& cc, Reg& a, Reg& b, Reg& x, Reg& y) :
            cc{ cc },
            a{ a }, b{ b },
            x{ x }, y{ y }
        {
        }

        Reg operator()(const ir::Constant& c) {
            auto constant = cc.newFloatConst(asmjit::ConstPool::kScopeLocal, mnd::convert<float>(c.value));
            auto reg = cc.newYmmPs();
            std::string commentStr = "move constant [";
            commentStr += std::to_string(mnd::convert<double>(c.value));
            commentStr += "]";
            cc.comment(commentStr.c_str());
            cc.vbroadcastss(reg, constant);
            return reg;
        }

        Reg operator()(const ir::Variable& v) {
            if (v.name == "z_re") {
                return a;
            }
            else if (v.name == "z_im") {
                return b;
            }
            else if (v.name == "c_re") {
                return x;
            }
            else if (v.name == "c_im") {
                return y;
            }
            else
                throw mnd::ParseError(std::string("unknown variable: ") + v.name);
        }

        Reg operator()(const ir::Negation& n) {
            auto sub = cc.newYmmPs();
            cc.vxorps(sub, sub, sub);
            cc.vsubps(sub, sub, visitNode(*n.value));
            return sub;
        }

        Reg operator()(const ir::Addition& add) {
            auto res = cc.newYmmPs();
            cc.vaddps(res, visitNode(*add.left), visitNode(*add.right));
            return res;
        }

        Reg operator()(const ir::Subtraction& add) {
            auto res = cc.newYmmPs();
            cc.vsubps(res, visitNode(*add.left), visitNode(*add.right));
            return res;
        }

        Reg operator()(const ir::Multiplication& add) {
            auto res = cc.newYmmPs();
            cc.vmulps(res, visitNode(*add.left), visitNode(*add.right));
            return res;
        }

        Reg operator()(const ir::Division& add) {
            auto res = cc.newYmmPs();
            cc.vdivps(res, visitNode(*add.left), visitNode(*add.right));
            return res;
        }

        static double myAtan2(double y, double x)
        {
            double result = ::atan2(y, x);
            printf("atan2(%f, %f) = %f\n", y, x, result);
            return result;
        }

        Reg operator()(const ir::Atan2& at2) {
            using namespace asmjit;
            auto y = visitNode(*at2.left);
            auto x = visitNode(*at2.right);
            auto arg = cc.newYmmPs();
            /*
            double(*atanFunc)(double, double) = ::atan2;
            cc.comment("call atan2");
            auto call = cc.call(imm(atanFunc), FuncSignatureT<double, double, double>(CallConv::kIdHost));
            call->setArg(0, y);
            call->setArg(1, x);
            call->setRet(0, arg);*/
            return arg;
        }

        Reg operator()(const ir::Pow& p) {
            using namespace asmjit;
            auto a = visitNode(*p.left);
            auto b = visitNode(*p.right);

            auto arg = cc.newYmmPs();
            /*double(*powFunc)(double, double) = ::pow;
            cc.comment("call pow");
            auto call = cc.call(imm(powFunc), FuncSignatureT<double, double, double>(CallConv::kIdHost));
            call->setArg(0, a);
            call->setArg(1, b);
            call->setRet(0, arg);*/
            return arg;
        }

        Reg operator()(const ir::Cos& c) {
            using namespace asmjit;
            auto a = visitNode(*c.value);

            auto arg = cc.newYmmPs();
            /*double(*cosFunc)(double) = ::cos;
            cc.comment("call cos");
            auto call = cc.call(imm(cosFunc), FuncSignatureT<double, double>(CallConv::kIdHost));
            call->setArg(0, a);
            call->setRet(0, arg);*/
            return arg;
        }

        Reg operator()(const ir::Sin& s) {
            using namespace asmjit;
            auto a = visitNode(*s.value);

            auto arg = cc.newYmmPs();
            /*double(*sinFunc)(double) = ::sin;
            cc.comment("call sin");
            auto call = cc.call(imm(sinFunc), FuncSignatureT<double, double>(CallConv::kIdHost));
            call->setArg(0, a);
            call->setRet(0, arg);*/
            return arg;
        }

        Reg operator()(const ir::Exp& ex) {
            using namespace asmjit;
            auto a = visitNode(*ex.value);

            auto arg = cc.newYmmPs();
            /*double(*expFunc)(double) = ::exp;
            cc.comment("call exp");
            auto call = cc.call(imm(expFunc), FuncSignatureT<double, double>(CallConv::kIdHost));
            call->setArg(0, a);
            call->setRet(0, arg);*/
            return arg;
        }

        Reg operator()(const ir::Ln& l) {
            using namespace asmjit;
            auto a = visitNode(*l.value);

            auto arg = cc.newYmmPs();
            /*double(*logFunc)(double) = ::log;
            cc.comment("call log");
            auto call = cc.call(imm(logFunc), FuncSignatureT<double, double>(CallConv::kIdHost));
            call->setArg(0, a);
            call->setRet(0, arg);*/
            return arg;
        }
    };

    CompiledGeneratorVec compileAVXFloat(const ir::Formula& formula)
    {
        using namespace asmjit;
        std::unique_ptr<mnd::ExecData> ed = std::make_unique<mnd::ExecData>();
        JitRuntime& jitRuntime = *ed->jitRuntime;
        ed->code->init(jitRuntime.codeInfo());

        x86::Compiler& comp = *ed->compiler;

        x86::Mem sixteen = comp.newYmmConst(ConstPool::kScopeLocal, Data256::fromF32(16.0f));
        x86::Mem one = comp.newYmmConst(ConstPool::kScopeLocal, Data256::fromF32(1.0f));
        x86::Mem factors = comp.newYmmConst(ConstPool::kScopeLocal, Data256::fromF32(0, 1, 2, 3, 4, 5, 6, 7));

        Label startLoop = comp.newLabel();
        Label endLoop = comp.newLabel();
        x86::Gp maxIter = comp.newInt32();
        x86::Gp k = comp.newInt32();
        x86::Gp resPtr = comp.newGpq();
        x86::Ymm adder = comp.newYmmPs();
        x86::Ymm counter = comp.newYmmPs();
        x86::Xmm xorig = comp.newXmmSs();
        x86::Xmm yorig = comp.newXmmSs();
        x86::Ymm dx = comp.newYmmPs();
        x86::Ymm x = comp.newYmmPs();
        x86::Ymm y = comp.newYmmPs();
        x86::Ymm a = comp.newYmmPs();
        x86::Ymm b = comp.newYmmPs();
        comp.addFunc(FuncSignatureT<int, float, float, float, int, float*>(CallConv::kIdHost));

        comp.setArg(0, xorig);
        comp.setArg(1, yorig);
        comp.setArg(2, dx.xmm());
        comp.setArg(3, maxIter);
        comp.setArg(4, resPtr);

        comp.vmovaps(adder, one);
        comp.vxorps(counter, counter, counter);

        comp.vshufps(xorig, xorig, xorig, 0);
        comp.vshufps(yorig, yorig, yorig, 0);
        comp.vshufps(dx.half(), dx.half(), dx.half(), 0);
        comp.vinsertf128(x, xorig.ymm(), xorig, 1);
        comp.vinsertf128(y, yorig.ymm(), yorig, 1);
        comp.vinsertf128(dx, dx, dx.xmm(), 1);

        comp.vmulps(dx, dx, factors);
        comp.vaddps(x, x, dx);

        CompileVisitorAVXFloat formVisitor{ comp, a, b, x, y };
        auto startA = std::visit(formVisitor, *formula.startA);
        auto startB = std::visit(formVisitor, *formula.startB);
        comp.vmovaps(a, startA);
        comp.vmovaps(b, startB);

        comp.xor_(k, k);

        comp.bind(startLoop);


        auto newA = std::visit(formVisitor, *formula.newA);
        auto newB = std::visit(formVisitor, *formula.newB);
        comp.vmovaps(a, newA);
        comp.vmovaps(b, newB);

        x86::Ymm aa = comp.newYmmPs();
        x86::Ymm bb = comp.newYmmPs();
        x86::Ymm cmp = comp.newYmmPs();
        comp.vmulps(aa, a, a);
        comp.vmulps(bb, b, b);
        comp.vaddps(bb, bb, aa);
        comp.vcmpps(cmp, bb, sixteen, 18);
        comp.vandps(adder, adder, cmp);
        comp.vaddps(counter, counter, adder);

        comp.cmp(k, maxIter);
        comp.je(endLoop);
        comp.add(k, 1);

        comp.vtestps(cmp, cmp);
        comp.jne(startLoop);

        comp.bind(endLoop);

        comp.vmovups(x86::xmmword_ptr(resPtr), counter.half());
        comp.vextractf128(x86::xmmword_ptr(resPtr, 16), counter, 0x1);

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

        return CompiledGeneratorVec{ std::move(ed) };
    }


    struct OpenClVisitor
    {
        int varnameCounter = 0;
        std::stringstream code;
        std::string floatTypeName;

        OpenClVisitor(int startVarname, const std::string& floatTypeName) :
            varnameCounter{ startVarname },
            floatTypeName{ floatTypeName }
        {
        }

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
                if (!std::get_if<ir::Variable>(&node) && !std::get_if<ir::Constant>(&node)) {
                    std::string varname = createVarname();
                    code << floatTypeName << " " << varname << " = " << value << ";" << std::endl;
                    nodeData = varname;
                    return varname;
                }
                return value;
            }
        }

        std::string operator()(const ir::Constant& c) {
            return std::to_string(mnd::convert<double>(c.value)) + ((floatTypeName == "float") ? "f" : "");
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

        std::string operator()(const ir::Division& a) {
            return "("s + visitNode(*a.left) + ") / (" + visitNode(*a.right) + ")";
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
            return "log("s + visitNode(*a.value) + ")";
        }
    };

    std::string compileToOpenCl(const ir::Formula& formula)
    {
        OpenClVisitor z0Visitor{ 0, "float" };
        std::string startA = z0Visitor.visitNode(*formula.startA);
        std::string startB = z0Visitor.visitNode(*formula.startB);

        OpenClVisitor ocv{ z0Visitor.varnameCounter, "float" };
        std::string newA = ocv.visitNode(*formula.newA);
        std::string newB = ocv.visitNode(*formula.newB);

        std::string prelude =
            "__kernel void iterate(__global float* A, const int width, float xl, float yt, float pixelScaleX, float pixelScaleY, int max, int smooth, int julia, float juliaX, float juliaY) {\n"
            "   int index = get_global_id(0);\n"
            "   int ix = index % width;\n"
            "   int iy = index / width;\n"
            "   float c_re = ix * pixelScaleX + xl;\n"
            "   float c_im = iy * pixelScaleY + yt;\n";
        prelude += z0Visitor.code.str() +
            "   float z_re = " + startA + ";\n" +
            "   float z_im = " + startB + ";\n" +
            "\n"
            "   int n = 0;\n"
            "   while (n < max - 1) {\n";
        /*
        std::string orig = 
"       float aa = a * a;"
"       float bb = b * b;"
"       float ab = a * b;"
"       a = aa - bb + x;"
"       b = ab + ab + y;";
*/
    
        std::string after = 
"       if (z_re * z_re + z_im * z_im > 16) break;\n"
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
        code += "z_re = " + newA + ";\n";
        code += "z_im = " + newB + ";\n";
        code += after;
        //code = mnd::getFloat_cl();
        printf("cl: %s\n", code.c_str()); fflush(stdout);
        return code;
    }

    std::string compileToOpenClDouble(const ir::Formula& formula)
    {
        OpenClVisitor z0Visitor{ 0, "double" };
        std::string startA = z0Visitor.visitNode(*formula.startA);
        std::string startB = z0Visitor.visitNode(*formula.startB);

        OpenClVisitor ocv{ z0Visitor.varnameCounter, "double" };
        std::string newA = ocv.visitNode(*formula.newA);
        std::string newB = ocv.visitNode(*formula.newB);

        std::string prelude =
            "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
            "__kernel void iterate(__global float* A, const int width, double xl, double yt, double pixelScaleX, double pixelScaleY, int max, int smooth, int julia, double juliaX, double juliaY) {\n"
            "   int index = get_global_id(0);\n"
            "   int ix = index % width;\n"
            "   int iy = index / width;\n"
            "   double c_re = ix * pixelScaleX + xl;\n"
            "   double c_im = iy * pixelScaleY + yt;\n";
        prelude += z0Visitor.code.str() +
            "   double z_re = " + startA + ";\n" +
            "   double z_im = " + startB + ";\n" +
            "\n"
            "   int n = 0;\n"
            "   while (n < max - 1) {\n";

        std::string after = 
            "       if (z_re * z_re + z_im * z_im > 16) break;\n"
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
        code += "z_re = " + newA + ";\n";
        code += "z_im = " + newB + ";\n";
        code += after;
        //code = mnd::getFloat_cl();
        printf("cld: %s\n", code.c_str()); fflush(stdout);
        return code;
    }

#ifdef WITH_OPENCL
    std::unique_ptr<MandelGenerator> compileCl(const ir::Formula& formula, MandelDevice& md)
    {
        return std::make_unique<CompiledClGenerator>(md, compileToOpenCl(formula));
    }

    std::unique_ptr<MandelGenerator> compileClDouble(const ir::Formula& formula, MandelDevice& md)
    {
        return std::make_unique<CompiledClGeneratorDouble>(md, compileToOpenClDouble(formula));
    }
#endif

    std::vector<std::unique_ptr<mnd::MandelGenerator>> compileCpu(mnd::MandelContext& mndCtxt,
        const IterationFormula& z0,
        const IterationFormula& zi)
    {
        std::vector<std::unique_ptr<mnd::MandelGenerator>> vec;

        IterationFormula z0o = z0.clone();
        IterationFormula zio = zi.clone();
        z0o.optimize();
        zio.optimize();

        ir::Formula irf = mnd::expand(z0o, zio);
        irf.optimize();
        printf("ir: %s\n", irf.toString().c_str()); fflush(stdout);
        auto dg = std::make_unique<CompiledGenerator>(compile(irf));
        printf("asm: %s\n", dg->dump().c_str()); fflush(stdout);
        vec.push_back(std::move(dg));
        if (mndCtxt.getCpuInfo().hasAvx()) {
            auto dgavx = std::make_unique<CompiledGeneratorVec>(compileAVXFloat(irf));
            printf("asm avxvec: %s\n", dgavx->dump().c_str()); fflush(stdout);
            vec.push_back(std::move(dgavx));
        }
        
        //auto dg = std::make_unique<NaiveIRGenerator>(*irf, mnd::getPrecision<double>());

        //vec.push_back(std::move(ng));
        return vec;
    }

    std::vector<std::unique_ptr<mnd::MandelGenerator>> compileOpenCl(mnd::MandelDevice& dev,
        const IterationFormula& z0,
        const IterationFormula& zi)
    {
        std::vector<std::unique_ptr<mnd::MandelGenerator>> vec;
        IterationFormula z0o = z0.clone();
        IterationFormula zio = zi.clone();
        z0o.optimize();
        zio.optimize();
        printf("if: %s\n", mnd::toString(*zio.expr).c_str()); fflush(stdout);

        ir::Formula irf = mnd::expand(z0o, zio);
        irf.optimize();
        printf("ir: %s\n", irf.toString().c_str()); fflush(stdout);
        auto fl = compileCl(irf, dev);
        vec.push_back(std::move(fl));
        if (dev.supportsDouble()) {
            irf.clearNodeData();
            auto fld = compileClDouble(irf, dev);
            vec.push_back(std::move(fld));
        }

        return vec;// { { mnd::GeneratorType::FLOAT, std::move(fl) } };
    }

    GeneratorCollection compileFormula(mnd::MandelContext& mndCtxt, const IterationFormula& z0,
        const IterationFormula& zi)
    {
        GeneratorCollection cr;
        cr.cpuGenerators = compileCpu(mndCtxt, z0, zi);
        for (auto& dev : mndCtxt.getDevices()) {
            auto gens = compileOpenCl(*dev, z0, zi);
            std::move(gens.begin(), gens.end(), std::back_inserter(cr.clGenerators));
        }

        cr.adaptiveGenerator = std::make_unique<mnd::AdaptiveGenerator>();
        if (!cr.clGenerators.empty()) {
            cr.adaptiveGenerator->addGenerator(mnd::getPrecision<float>(), *cr.clGenerators[0]);
        }
        if (!cr.cpuGenerators.empty()) {
            cr.adaptiveGenerator->addGenerator(mnd::getPrecision<double>(), *cr.cpuGenerators[0]);
        }

        return cr;
    }
}

mnd::GeneratorCollection::GeneratorCollection(void) :
    cpuGenerators{},
    clGenerators{},
    adaptiveGenerator{ nullptr }
{
}
