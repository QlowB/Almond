#ifndef MANDEL_NAIVEIRGENERATOR_H
#define MANDEL_NAIVEIRGENERATOR_H

#include "IterationIR.h"
#include "Generators.h"
#include <memory>
#include <variant>

namespace mnd
{
    template<typename T>
    class NaiveIRGenerator;

    namespace eval
    {
        struct Load;
        struct Store;
        struct Add;
        struct Sub;
        struct Mul;
        struct Div;
        struct Neg;    
        struct Atan2;
        struct Pow;
        struct Cos;
        struct Sin;
        struct Exp;
        struct Ln;

        using EvalNode = std::variant<
            Load,
            Store,
            Add,
            Sub,
            Mul,
            Div,
            Neg,
            Atan2,
            Pow,
            Cos,
            Sin,
            Exp,
            Ln
        >;


        struct Load { size_t index; };
        struct Store
        {
            size_t index;
            std::unique_ptr<EvalNode> v;
        };
    
        struct BinaryOperation
        {
            std::unique_ptr<EvalNode> a;
            std::unique_ptr<EvalNode> b;
        };
    
        struct UnaryOperation
        {
            std::unique_ptr<EvalNode> a;
        };
    
        struct Add : BinaryOperation {};
        struct Sub : BinaryOperation {};
        struct Mul : BinaryOperation {};
        struct Div : BinaryOperation {};
    
        struct Neg : UnaryOperation {};
    
    
        struct Atan2 : BinaryOperation {};
        struct Pow : BinaryOperation {};
        struct Cos : UnaryOperation {};
        struct Sin : UnaryOperation {};
        struct Exp : UnaryOperation {};
        struct Ln : UnaryOperation {};



        template<typename T>
        struct EvalStruct
        {
            std::map<std::string, size_t> variableNames;
            std::vector<T> variables;

            void prepare(const T& zre, const T& zim, const T& cre, const T& cim)
            {
                auto z_re = variableNames.find("z_re");
                auto z_im = variableNames.find("z_im");
                auto c_re = variableNames.find("c_re");
                auto c_im = variableNames.find("c_im");
                if (z_re != variableNames.end())
                    variables[z_re->second] = zre;
                if (z_im != variableNames.end())
                    variables[z_im->second] = zre;
                if (c_re != variableNames.end())
                    variables[c_re->second] = zre;
                if (c_im != variableNames.end())
                    variables[c_im->second] = zre;
            }
        };
    }
}


template<typename T>
class mnd::NaiveIRGenerator : public mnd::MandelGenerator
{
    const ir::Formula& form;
    eval::EvalStruct<T> es;
    std::unique_ptr<eval::EvalNode> newz_re;
    std::unique_ptr<eval::EvalNode> newz_im;
    std::unique_ptr<eval::EvalNode> start_re;
    std::unique_ptr<eval::EvalNode> start_im;
public:
    NaiveIRGenerator(const ir::Formula& irf, mnd::Precision prec = mnd::getType<T>());
    NaiveIRGenerator(NaiveIRGenerator&&) = default;

    virtual void generate(const MandelInfo& info, float* data);
};

extern template class mnd::NaiveIRGenerator<float>;
extern template class mnd::NaiveIRGenerator<double>;
extern template class mnd::NaiveIRGenerator<mnd::DoubleDouble>;
extern template class mnd::NaiveIRGenerator<mnd::QuadDouble>;

#endif // MANDEL_NAIVEIRGENERATOR_H
