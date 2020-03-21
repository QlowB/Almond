#include "IterationIR.h"



using namespace mnd;


namespace mnd
{

    struct ConvertVisitor
    {
        util::Arena<ir::Node>& arena;

    };

    ir::Formula expand(const mnd::IterationFormula& fmla)
    {
        ir::Formula formula;
    }
}

