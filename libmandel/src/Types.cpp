#include "Types.h"
#include <sstream>


namespace mnd
{
#ifdef WITH_BOOST
    std::string toString(const Real& num)
    {
        return num.str();
    }
#else // !WITH_BOOST
    std::string toString(const Real& num)
    {
        std::stringstream ss;
        ss << num;
        return ss.str();
    }
#endif // WITH_BOOST


/*
    template<>
    constexpr Real precision<float>(void)
    {
        return 1.0e-7;
    }

    template<>
    constexpr Real precision<double>(void)
    {
        return 1.0e-15;
    }

#ifdef WITH_QD
    template<>
    constexpr Real precision<DoubleDouble>(void)
    {
        return Real("1.0e-29");
    }

    template<>
    constexpr Real precision<QuadDouble>(void)
    {
        return Real("1.0e-57");
    }
#endif // WITH_QD
#ifdef WITH_BOOST
    template<>
    constexpr Real precision<Float128>(void)
    {
        return Real("1.0e-29");
    }

    template<>
    constexpr Real precision<Float256>(void)
    {
        return Real("1.0e-60");
    }
#endif // WITH_BOOST
*/
}

