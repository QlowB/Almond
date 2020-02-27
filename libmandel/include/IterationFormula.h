#ifndef MANDEL_ITERATIONFORMULA_H
#define MANDEL_ITERATIONFORMULA_H


namespace mnd
{
    struct IterationFormula;

    struct Addition;
    struct Multiplication;
    struct Division;
    struct Pow;
}


struct mnd::IterationFormula
{
};


struct mnd::Addition : IterationFormula
{
    bool subtraction = false;
};


struct mnd::Multiplication : IterationFormula
{
};


struct mnd::Division : IterationFormula
{
};


struct mnd::Pow : IterationFormula
{
};



#endif // MANDEL_ITERATIONFORMULA_H


