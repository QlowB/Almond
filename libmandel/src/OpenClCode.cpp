#include "OpenClCode.h"

#include "opencl/float.h"
#include "opencl/double.h"
#include "opencl/doublefloat.h"
#include "opencl/doubledouble.h"
#include "opencl/tripledouble.h"
#include "opencl/triplefloat.h"
#include "opencl/quaddouble.h"
#include "opencl/hexdouble.h"
#include "opencl/octadouble.h"

#include "opencl/fixed64.h"
#include "opencl/fixed128.h"
#include "opencl/fixed512.h"

#include "opencl_template/doubledouble_prelude.h"

namespace mnd
{
    std::string getFloat_cl() {
        return std::string{ (char*) float_cl, float_cl_len };
    }

    std::string getDouble_cl() {
        return std::string{ (char*) double_cl, double_cl_len };
    }

    std::string getDoubleFloat_cl() {
        return std::string{ (char*) doublefloat_cl, doublefloat_cl_len };
    }

    std::string getDoubleDouble_cl() {
        return std::string{ (char*) doubledouble_cl, doubledouble_cl_len };
    }

    std::string getTripleDouble_cl() {
        return std::string{ (char*) tripledouble_cl, tripledouble_cl_len };
    }

    std::string getTripleFloat_cl() {
        return std::string{ (char*) triplefloat_cl, triplefloat_cl_len };
    }

    std::string getQuadDouble_cl() {
        return std::string{ (char*) quaddouble_cl, quaddouble_cl_len };
    }

    std::string getHexDouble_cl() {
        return std::string{ (char*) hexdouble_cl, hexdouble_cl_len };
    }

    std::string getOctaDouble_cl() {
        return std::string{ (char*) octadouble_cl, octadouble_cl_len };
    }

    std::string getFixed64_cl() {
        return std::string{ (char*) fixed64_cl, fixed64_cl_len };
    }

    std::string getFixed128_cl() {
        return std::string{ (char*) fixed128_cl, fixed128_cl_len };
    }

    std::string getFixed512_cl() {
        return std::string{ (char*) fixed512_cl, fixed512_cl_len };
    }

    std::string getDoubleDouble_prelude() {
        return std::string{ (char*) doubledouble_prelude_cl, doubledouble_prelude_cl_len };
    }
}


