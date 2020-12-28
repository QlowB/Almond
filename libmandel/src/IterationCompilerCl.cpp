#include "IterationCompiler.h"
#include "NaiveIRGenerator.h"

#include "Mandel.h"
#ifdef WITH_ASMJIT
#include "ExecData.h"
#endif // WITH_ASMJIT

#include "OpenClInternal.h"
#include "OpenClCode.h"

#include <cmath>
#include <omp.h>
#include <any>
#include <string>

using namespace std::string_literals;

#ifdef WITH_OPENCL
namespace mnd
{
    struct FloatTraits
    {
        static std::string getPrelude() {
            return
                "__kernel void iterate(__global float* A, const int width,"
                "                      float xl, float yt, float pixelScaleX, float pixelScaleY,"
                "                      int max, int smooth, int julia, float juliaX, float juliaY) {\n"
                "   int index = get_global_id(0);\n"
                "   int ix = index % width;\n"
                "   int iy = index / width;\n"
                "   float c_re = ix * pixelScaleX + xl;\n"
                "   float c_im = iy * pixelScaleY + yt;\n";
        }

        static std::string getEscapeCheck() {
            return "   if (z_re * z_re + z_im * z_im >= 16) break;";
        }

        static std::string getPostlude() {
            return
                "   if (n >= max - 1) {\n"
                "       A[index] = max;\n"
                "   }\n"
                "   else {\n"
                "       A[index] = ((float) n);\n"
                "   }\n"
                "}\n";
        }

        static std::string getTypeName() {
            return "float";
        }

        static std::string constant(const mnd::Real& v) {
            return "((float) "s + std::to_string(mnd::convert<float>(v)) + ")";
        }
        static std::string neg(const std::string& v) {
            return "-"s + v;
        }
        static std::string add(const std::string& a, const std::string& b) {
            return "("s + a + ") + (" + b + ")";
        }
        static std::string sub(const std::string& a, const std::string& b) {
            return "("s + a + ") - (" + b + ")";
        }
        static std::string mul(const std::string& a, const std::string& b) {
            return "("s + a + ") * (" + b + ")";
        }
        static std::string div(const std::string& a, const std::string& b) {
            return "("s + a + ") / (" + b + ")";
        }
        static std::string atan2(const std::string& a, const std::string& b) {
            return "atan2("s + a + ", " + b + ")";
        }
        static std::string pow(const std::string& a, const std::string& b) {
            return "pow("s + a + ", " + b + ")";
        }
        static std::string cos(const std::string& x) {
            return "cos("s + x + ")";
        }
        static std::string sin(const std::string& x) {
            return "sin("s + x + ")";
        }
        static std::string exp(const std::string& x) {
            return "exp("s + x + ")";
        }
        static std::string ln(const std::string& x) {
            return "log("s + x + ")";
        }
    };

    struct DoubleTraits
    {
        static std::string getPrelude() {
            return
                "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
                "__kernel void iterate(__global float* A, const int width,"
                "                      double xl, double yt, double pixelScaleX, double pixelScaleY,"
                "                      int max, int smooth, int julia, double juliaX, double juliaY) {\n"
                "   int index = get_global_id(0);\n"
                "   int ix = index % width;\n"
                "   int iy = index / width;\n"
                "   double c_re = ix * pixelScaleX + xl;\n"
                "   double c_im = iy * pixelScaleY + yt;\n";
        }

        static std::string getEscapeCheck() {
            return "   if (z_re * z_re + z_im * z_im >= 16) break;";
        }

        static std::string getPostlude() {
            return
                "   if (n >= max - 1) {\n"
                "       A[index] = max;\n"
                "   }\n"
                "   else {\n"
                "       A[index] = ((float) n);\n"
                "   }\n"
                "}\n";
        }

        static std::string getTypeName() {
            return "double";
        }

        static std::string constant(const mnd::Real& v) {
            return mnd::toString(v);
        }
        static std::string neg(const std::string& v) {
            return "-"s + v;
        }
        static std::string add(const std::string& a, const std::string& b) {
            return "("s + a + ") + (" + b + ")";
        }
        static std::string sub(const std::string& a, const std::string& b) {
            return "("s + a + ") - (" + b + ")";
        }
        static std::string mul(const std::string& a, const std::string& b) {
            return "("s + a + ") * (" + b + ")";
        }
        static std::string div(const std::string& a, const std::string& b) {
            return "("s + a + ") / (" + b + ")";
        }
        static std::string atan2(const std::string& a, const std::string& b) {
            return "atan2("s + a + ", " + b + ")";
        }
        static std::string pow(const std::string& a, const std::string& b) {
            return "pow("s + a + ", " + b + ")";
        }
        static std::string cos(const std::string& x) {
            return "cos("s + x + ")";
        }
        static std::string sin(const std::string& x) {
            return "sin("s + x + ")";
        }
        static std::string exp(const std::string& x) {
            return "exp("s + x + ")";
        }
        static std::string ln(const std::string& x) {
            return "log("s + x + ")";
        }
    };

    struct DoubleDoubleTraits
    {
        static std::string getPrelude() {
            return cl_src::doubledouble_prelude_cl;
        }

        static std::string getEscapeCheck() {
            return "   if (z_re.s0 * z_re.s0 + z_im.s0 * z_im.s0 >= 16) break;";
        }

        static std::string getPostlude() {
            return DoubleTraits::getPostlude();
        }

        static std::string getTypeName() {
            return "double2";
        }

        static std::string constant(const mnd::Real& v) {
            mnd::DoubleDouble dd = mnd::convert<mnd::DoubleDouble>(v);
            return "(double2)(" + std::to_string(dd.x[0]) + ", " + std::to_string(dd.x[1]) + ")";
        }
        static std::string neg(const std::string& v) {
            return "(double2)(-"s + v + ".s0, -" + v + ".s1)";
        }
        static std::string add(const std::string& a, const std::string& b) {
            return "add("s + a + ", " + b + ")";
        }
        static std::string sub(const std::string& a, const std::string& b) {
            return "add("s + a + ", " + neg(b) + ")";
        }
        static std::string mul(const std::string& a, const std::string& b) {
            return "mul("s + a + ", " + b + ")";
        }
        static std::string div(const std::string& a, const std::string& b) {
            return "div("s + a + ") / (" + b + ")";
        }
        static std::string atan2(const std::string& a, const std::string& b) {
            return "atan2("s + a + ", " + b + ")";
        }
        static std::string pow(const std::string& a, const std::string& b) {
            return "pow("s + a + ", " + b + ")";
        }
        static std::string cos(const std::string& x) {
            return "cos("s + x + ")";
        }
        static std::string sin(const std::string& x) {
            return "sin("s + x + ")";
        }
        static std::string exp(const std::string& x) {
            return "exp("s + x + ")";
        }
        static std::string ln(const std::string& x) {
            return "log("s + x + ")";
        }
    };

    struct QuadDoubleTraits
    {
        static std::string getPrelude() {
            return cl_src::quaddouble_prelude_cl;
        }

        static std::string getEscapeCheck() {
            return DoubleDoubleTraits::getEscapeCheck();
        }

        static std::string getPostlude() {
            return DoubleDoubleTraits::getPostlude();
        }

        static std::string getTypeName() {
            return "double4";
        }

        static std::string constant(const mnd::Real& v) {
            mnd::QuadDouble dd = mnd::convert<mnd::QuadDouble>(v);
            return "(double4)(" + std::to_string(dd.x[0]) + ", " + std::to_string(dd.x[1]) +
                           ", " + std::to_string(dd.x[2]) + ", " + std::to_string(dd.x[3]) + ")";
        }
        static std::string neg(const std::string& v) {
            return "(double4)(-"s + v + ".s0, -" + v + ".s1, -" + v + ".s2, -" + v + ".s3)";
        }
        static std::string add(const std::string& a, const std::string& b) {
            return "add("s + a + ", " + b + ")";
        }
        static std::string sub(const std::string& a, const std::string& b) {
            return "add("s + a + ", " + neg(b) + ")";
        }
        static std::string mul(const std::string& a, const std::string& b) {
            return "mul("s + a + ", " + b + ")";
        }
        static std::string div(const std::string& a, const std::string& b) {
            return "div("s + a + ") / (" + b + ")";
        }
        static std::string atan2(const std::string& a, const std::string& b) {
            return "atan2("s + a + ", " + b + ")";
        }
        static std::string pow(const std::string& a, const std::string& b) {
            return "pow("s + a + ", " + b + ")";
        }
        static std::string cos(const std::string& x) {
            return "cos("s + x + ")";
        }
        static std::string sin(const std::string& x) {
            return "sin("s + x + ")";
        }
        static std::string exp(const std::string& x) {
            return "exp("s + x + ")";
        }
        static std::string ln(const std::string& x) {
            return "log("s + x + ")";
        }
    };

    template<typename Traits>
    class ClTranspiler
    {
        int varnameCounter = 0;
        std::stringstream code;
    public:
        ClTranspiler(void) = default;

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
                    code << Traits::getTypeName() << " " << varname << " = " << value << ";" << std::endl;
                    nodeData = varname;
                    return varname;
                }
                return value;
            }
        }

        std::string createVarname(void)
        {
            return "tmp"s + std::to_string(varnameCounter++);
        }

        std::string compile(const ir::Formula& formula)
        {
            code << Traits::getPrelude();

            std::string startA = visitNode(*formula.startA);
            std::string startB = visitNode(*formula.startB);

            code <<
            "    " << Traits::getTypeName() << " z_re = " << startA << ";\n" <<
            "    " << Traits::getTypeName() << " z_im = " << startB << ";\n" <<
            "    int n = 0;\n"
            "    while (n < max - 1) {\n";

            std::string newA = visitNode(*formula.newA);
            std::string newB = visitNode(*formula.newB);

            code <<
            "    z_re = " << newA << ";\n" <<
            "    z_im = " << newB << ";\n";

            code << Traits::getEscapeCheck();
            code <<
            "    n++;\n"
            "}\n";

            code << Traits::getPostlude();
            return code.str();
        }

        std::string operator()(const ir::Constant& c) {
            return Traits::constant(c.value);
        }
        std::string operator()(const ir::Variable& v) {
            return v.name;
        }
        std::string operator()(const ir::Negation& n) {
            return Traits::neg(visitNode(*n.value));
        }
        std::string operator()(const ir::Addition& a) {
            return Traits::add(visitNode(*a.left), visitNode(*a.right));
        }
        std::string operator()(const ir::Subtraction& a) {
            return Traits::sub(visitNode(*a.left), visitNode(*a.right));
        }
        std::string operator()(const ir::Multiplication& a) {
            return Traits::mul(visitNode(*a.left), visitNode(*a.right));
        }
        std::string operator()(const ir::Division& a) {
            return Traits::div(visitNode(*a.left), visitNode(*a.right));
        }
        std::string operator()(const ir::Atan2& a) {
            return Traits::atan2(visitNode(*a.left), visitNode(*a.right));
        }
        std::string operator()(const ir::Pow& a) {
            return Traits::pow(visitNode(*a.left), visitNode(*a.right));
        }
        std::string operator()(const ir::Cos& a) {
            return Traits::cos(visitNode(*a.value));
        }

        std::string operator()(const ir::Sin& a) {
            return Traits::sin(visitNode(*a.value));
        }

        std::string operator()(const ir::Exp& a) {
            return Traits::exp(visitNode(*a.value));
        }

        std::string operator()(const ir::Ln& a) {
            return Traits::ln(visitNode(*a.value));
        }
    };

    std::unique_ptr<MandelGenerator> compileClFloat(const ir::Formula& formula, MandelDevice& md)
    {
        ClTranspiler<FloatTraits> clt;
        std::string code = clt.compile(formula);
        return std::make_unique<CompiledClGenerator>(md, code);
    }

    std::unique_ptr<MandelGenerator> compileClDouble(const ir::Formula& formula, MandelDevice& md)
    {
        ClTranspiler<DoubleTraits> clt;
        std::string code = clt.compile(formula);
        return std::make_unique<CompiledClGeneratorDouble>(md, code);
    }

    std::unique_ptr<MandelGenerator> compileClDoubleDouble(const ir::Formula& formula, MandelDevice& md)
    {
        ClTranspiler<DoubleDoubleTraits> clt;
        std::string code = clt.compile(formula);
        return std::make_unique<CompiledClGeneratorDoubleDouble>(md, code);
    }

    std::unique_ptr<MandelGenerator> compileClQuadDouble(const ir::Formula& formula, MandelDevice& md)
    {
        ClTranspiler<QuadDoubleTraits> clt;
        std::string code = clt.compile(formula);
        return std::make_unique<CompiledClGeneratorQuadDouble>(md, code);
    }
}

#endif

