#ifndef MANDEL_ITERATIONIR_H
#define MANDEL_ITERATIONIR_H

#include <string>
#include <vector>

#include "IterationFormula.h"
#include "Arena.h"

namespace mnd
{
    namespace ir
    {
        struct Constant;
        struct Variable;
        struct Negation;
        struct BinaryOperation;
        struct Addition;
        struct Multiplication;

        using Node = std::variant<
            Constant,
            Variable,
            Negation,
            Addition,
            Multiplication
        >;

        class Formula
        {
            util::Arena<Node> nodeArena;
            Node* newA;
            Node* newB;
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


struct mnd::ir::Negation
{
    Node* value;
};


struct mnd::ir::BinaryOperation
{
    Node* a;
    Node* b;
};


struct mnd::ir::Addition : mnd::ir::BinaryOperation
{
};


struct mnd::ir::Multiplication : mnd::ir::BinaryOperation
{
};


#endif // MANDEL_ITERATIONIR_H
