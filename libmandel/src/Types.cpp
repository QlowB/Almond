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
}

