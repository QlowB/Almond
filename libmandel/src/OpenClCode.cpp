#include "OpenClCode.h"

#include "opencl/float.h"
#include "opencl/double.h"
#include "opencl/doublefloat.h"
#include "opencl/doubledouble.h"
#include "opencl/quaddouble.h"

#include "opencl/fixed64.h"
#include "opencl/fixed128.h"
#include "opencl/fixed512.h"

namespace mnd
{
    const char* getFloat_cl() {
        return (char*) float_cl;
    }

    const char* getDouble_cl() {
        return (char*) float_cl;
    }

    const char* getDoubleFloat_cl() {
        return (char*) doublefloat_cl;
    }

    const char* getDoubleDouble_cl() {
        return (char*) doubledouble_cl;
    }

    const char* getQuadDouble_cl() {
        return (char*) quaddouble_cl;
    }

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


