#ifndef MANDEL_ITERATIONFORMULA_H
#define MANDEL_ITERATIONFORMULA_H

#include <variant>
#include <memory>
#include <string>
#include <stdexcept>
#include <optional>

#include "Real.h"

namespace mnd
{
    struct IterationFormula;

    struct Constant;
    struct Variable;
    struct Negation;
    struct BinaryOperation;
    struct Addition;
    struct Multiplication;
    struct Division;
    struct Pow;

    using Expression = std::variant<
            Constant,
            Variable,
            Negation,
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
    std::vector<std::string> variables;
    std::unique_ptr<Expression> expr;
    IterationFormula(void) = default;
    IterationFormula(std::unique_ptr<Expression> expr, const std::vector<std::string>& variables = { "c", "z" });
    IterationFormula(Expression expr, const std::vector<std::string>& variables = { "c", "z" });

    std::optional<std::string> findUnknownVariables(const Expression& expr);
    void optimize(void);
    bool containsVariable(const std::string& name) const;

    IterationFormula clone(void) const;
};


struct mnd::Constant
{
    mnd::Real re;
    mnd::Real im;
    inline Constant(const mnd::Real& value) :
        re{ value },
        im{ 0.0 }
    {}

    inline Constant(const mnd::Real& re, const mnd::Real& im) :
        re{ re },
        im{ im }
    {}
};


struct mnd::Variable
{
    std::string name;
};


struct mnd::Negation
{
    std::unique_ptr<Expression> operand;
    /*inline Negation(const Negation& other) :
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
    bool realExponent;
    bool integerExponent;
};


namespace mnd
{
}


#endif // MANDEL_ITERATIONFORMULA_H


