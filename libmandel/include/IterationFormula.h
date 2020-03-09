#ifndef MANDEL_ITERATIONFORMULA_H
#define MANDEL_ITERATIONFORMULA_H

#include <variant>
#include <memory>
#include <string>

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

    struct ParseError : std::runtime_error
    {
        ParseError(const std::string& err) :
            std::runtime_error(err.c_str())
        {}
    };

    Expression parse(const std::string& formula);

    std::string toString(const mnd::Expression&);
}


struct mnd::IterationFormula
{
    std::unique_ptr<Expression> expr;
    IterationFormula(Expression expr);
};


struct mnd::Constant
{
    double value;
    inline Constant(double value) :
        value{ value }
    {}
    inline Constant() :
        value{ 1010 }
    {}
};


struct mnd::Variable
{
    std::string name;
};


struct mnd::UnaryOperation
{
    std::unique_ptr<Expression> operand;
    /*inline UnaryOperation(const UnaryOperation& other) :
        operand{ std::make_unique<Expression>(*other.operand) }
    {}*/
};


struct mnd::BinaryOperation
{
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    /*inline BinaryOperation(const BinaryOperation& other) :
        left{ std::make_unique<Expression>(*other.left) },
        right{ std::make_unique<Expression>(*other.right) }
    {}*/
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


namespace mnd
{
}


#endif // MANDEL_ITERATIONFORMULA_H


