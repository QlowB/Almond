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
namespace mnd
{
    struct OpenClDDVisitor
    {
        int varnameCounter = 0;
        std::stringstream code;
        std::string floatTypeName;

        OpenClDDVisitor(int startVarname, const std::string& floatTypeName) :
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

    std::string compileToOpenClDoubleDouble(const ir::Formula& formula)
    {
        OpenClDDVisitor z0Visitor{ 0, "double2" };
        std::string startA = z0Visitor.visitNode(*formula.startA);
        std::string startB = z0Visitor.visitNode(*formula.startB);

        OpenClDDVisitor ocv{ z0Visitor.varnameCounter, "double2" };
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
    std::unique_ptr<MandelGenerator> compileClDoubleDouble(const ir::Formula& formula, MandelDevice& md)
    {
        return nullptr;
        //return std::make_unique<CompiledClGeneratorDouble>(md, compileToOpenClDouble(formula));
    }
#endif
}


