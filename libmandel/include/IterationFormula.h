#ifndef MANDEL_ITERATIONFORMULA_H
#define MANDEL_ITERATIONFORMULA_H


namespace mnd
{
    struct IterationFormula;

    struct Expression;
    struct Constant;
    struct Variable;
    struct UnaryOperation;
    struct BinaryOperation;
    struct Addition;
    struct Multiplication;
    struct Division;
    struct Pow;
}


struct mnd::IterationFormula
{
    std::unique_ptr<Expression> expr;
};


struct mnd::Expression
{
};


struct mnd::Constant : mnd::Expression
{
    double value;
};


struct mnd::Variable : mnd::Expression
{
    std::string name;
};


struct mnd::UnaryOperation : mnd::Expression
{
    std::unique_ptr<Expression> operand;
};


struct mnd::BinaryOperation : mnd::Expression
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


