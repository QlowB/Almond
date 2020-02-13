#ifndef MANDEL_TYPES_H
#define MANDEL_TYPES_H

#include <cinttypes>
#ifdef WITH_BOOST
#   include <boost/multiprecision/cpp_bin_float.hpp>
#   include <boost/multiprecision/cpp_int.hpp>
#endif

namespace mnd
{


#ifdef WITH_BOOST
    using Float128 = boost::multiprecision::cpp_bin_float_quad;

    using Real = double;
    using Integer = boost::multiprecision::int128_t;
#else
    using Real = double;
    using Integer = int64_t;
#endif


}


#endif // MANDEL_TYPES_H
