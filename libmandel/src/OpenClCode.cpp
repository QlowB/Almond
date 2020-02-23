#include "OpenClCode.h"

#include "opencl/fixed64.h"
#include "opencl/fixed128.h"
#include "opencl/fixed512.h"

namespace mnd
{
    const char* getFixed64_cl() {
        return (char*) fixed64_cl;
    }

    const char* getFixed128_cl() {
        return (char*) fixed128_cl;
    }

    const char* getFixed512_cl() {
        return (char*) fixed512_cl;
    }
}


