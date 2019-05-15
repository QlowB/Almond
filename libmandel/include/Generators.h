#ifndef MANDEL_GENERATORS_H
#define MANDEL_GENERATORS_H

#include "MandelUtil.h"


namespace mnd
{
    class Generator;
}


class mnd::Generator
{
public:
    Generator(void) = default;
    virtual ~Generator(void);

    
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    
    Generator(Generator&&) = default;
    Generator& operator=(Generator&&) = default;

    virtual void generate(const MandelInfo& info, float* data) = 0;
};


#endif // MANDEL_GENERATORS_H
