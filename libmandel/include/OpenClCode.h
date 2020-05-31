#ifndef MANDEL_OPENCLCODE_H
#define MANDEL_OPENCLCODE_H

#include <string>

namespace mnd
{
    std::string getFloat_cl();
    std::string getDouble_cl();
    std::string getDoubleFloat_cl();
    std::string getDoubleDouble_cl();
    std::string getTripleDouble_cl();
    std::string getQuadDouble_cl();
    std::string getHexDouble_cl();
    std::string getFixed64_cl();
    std::string getFixed128_cl();
    std::string getFixed512_cl();
}

#endif // MANDEL_OPENCLCODE_H

