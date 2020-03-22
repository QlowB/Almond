#ifndef MANDEL_ITERATIONIR_H
#define MANDEL_ITERATIONIR_H

#include <string>
#include <vector>
#include <variant>

#include "IterationFormula.h"
#include "Arena.h"

namespace mnd
{
    namespace ir
    {
        struct Constant;
        struct Variable;
        struct UnaryOperation;
        struct Negation;
        struct BinaryOperation;
        struct Addition;
        struct Subtraction;
        struct Multiplication;
        struct Atan2;
        struct Pow;
        struct Cos;
        struct Sin;

        using Node = std::variant<
            Constant,
            Variable,
            Negation,
            Addition,
            Subtraction,
            Multiplication,
            Atan2,
            Pow,
            Cos,
            Sin
        >;

        struct Formula
        {
            util::Arena<Node> nodeArena;
            Node* newA;
            Node* newB;

            std::string toString(void) const;
        };
    }

    ir::Formula expand(const mnd::IterationFormula& fmla);
}


struct mnd::ir::Constant
{
    double value;
};


struct mnd::ir::Variable
{
    std::string name;
};


struct mnd::ir::UnaryOperation
{
    Node* value;
};


struct mnd::ir::Negation : mnd::ir::UnaryOperation
{
};


struct mnd::ir::BinaryOperation
{
    Node* left;
    Node* right;
};


struct mnd::ir::Addition : mnd::ir::BinaryOperation
{
};


struct mnd::ir::Subtraction : mnd::ir::BinaryOperation
{
};


struct mnd::ir::Multiplication : mnd::ir::BinaryOperation
{
};


struct mnd::ir::Atan2 : mnd::ir::BinaryOperation
{
};


struct mnd::ir::Pow : mnd::ir::BinaryOperation
{
};


struct mnd::ir::Cos : mnd::ir::UnaryOperation
{
};


struct mnd::ir::Sin : mnd::ir::UnaryOperation
{
};


#endif // MANDEL_ITERATIONIR_H
