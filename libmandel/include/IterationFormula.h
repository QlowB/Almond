#ifndef MANDEL_ITERATIONFORMULA_H
#define MANDEL_ITERATIONFORMULA_H

#include <variant>
#include <memory>

namespace mnd
{
    struct IterationFormula;

    struct Constant;
    struct Variable;
    struct UnaryOperation;
    struct BinaryOperation;
    struct Addition;
    struct Multiplication;
    struct Division;
    struct Pow;

    using Expression = std::variant<
            Constant,
            Variable,
            UnaryOperation,
            Addition,
            Multiplication,
            Division,
            Pow
    >;

    class FormulaVisitor;
}


struct mnd::IterationFormula
{
    std::unique_ptr<Expression> expr;
};


struct mnd::Constant
{
    double value;
};


struct mnd::Variable
{
    std::string name;
};


struct mnd::UnaryOperation
{
    std::unique_ptr<Expression> operand;
};


struct mnd::BinaryOperation
{
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
};


struct mnd::Addition : mnd::BinaryOperation
{
    bool subtraction = false;
};


struct mnd::Multiplication : mnd::BinaryOperation 
{
};


struct mnd::Division : mnd::BinaryOperation 
{
};


struct mnd::Pow : mnd::BinaryOperation 
{
};



#endif // MANDEL_ITERATIONFORMULA_H


