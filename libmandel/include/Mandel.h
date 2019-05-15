#ifndef MANDEL_MANDEL_H
#define MANDEL_MANDEL_H

#include <vector>
#include <memory>

#include "MandelUtil.h"
#include "Generators.h"

namespace mnd 
{
    class MandelContext;

    struct MandelViewport;
    struct MandelInfo;


    extern MandelContext initializeContext(void);
}


class mnd::MandelContext
{
private:
    friend MandelContext initializeContext(void);

    std::vector<std::unique_ptr<Generator>> generators;
    MandelContext(void) = default;
public:
};


#endif // MANDEL_MANDEL_H
