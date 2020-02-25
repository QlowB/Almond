#ifndef MANDEL_JULIAGENERATORS_H
#define MANDEL_JULIAGENERATORS_H

#include "Generators.h"



namespace mnd
{
    class JuliaGeneratorFloat;
}


class mnd::JuliaGeneratorFloat : public JuliaGenerator
{
protected:
    mnd::Real a;
    mnd::Real b;
public:
    JuliaGeneratorFloat(const mnd::Real& precision);
    virtual ~JuliaGeneratorFloat(void);

    virtual void generate(const MandelInfo& info, float* data);
};

#endif // MANDEL_JULIAGENERATORS_H

