#include "Real.h"
#include <sstream>



namespace mnd
{
    std::string toString(const Real& num)
    {
        return num.str();
    }

    std::string toLegibleString(const Real& num)
    {
        if (num == Real(0.0)) {
            return "0";
        }
        int exponent = int(std::floor(static_cast<float>(mnd::log(num)) / ::logf(10.000001f)));
        float fac = static_cast<float>(num / mnd::pow(Real(10), Real(exponent)));
        std::stringstream ss;
        ss.precision(3);
        ss << std::fixed << fac << "e";
        if (exponent < 0)
            ss << "-" << -exponent;
        else
            ss << "+" << exponent;
        return ss.str();
    }
}


