#ifndef MANDEL_TYPES_H
#define MANDEL_TYPES_H

#include <cinttypes>
#include <cmath>
#ifdef WITH_BOOST
#   include <boost/multiprecision/cpp_bin_float.hpp>
#   if defined(__GNUC__) || defined(__INTEL_COMPILER)
#       include <boost/multiprecision/float128.hpp>
#   endif
#   include <boost/multiprecision/cpp_int.hpp>
#endif

namespace mnd
{


#ifdef WITH_BOOST
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
    using Float128 = boost::multiprecision::float128;
#else
    using Float128 = boost::multiprecision::cpp_bin_float_quad;
#endif
    inline Float128 abs(const Float128& x) { return boost::multiprecision::abs(x); }
    inline Float128 floor(const Float128& x) { return boost::multiprecision::floor(x); }
    inline Float128 log(const Float128& x) { return boost::multiprecision::log(x); }

    using Real = Float128;
    using Integer = boost::multiprecision::int128_t;
#else
    using Real = double;
    using Integer = int64_t;
#endif

    inline double abs(double x) { return ::abs(x); }
    inline float abs(float x) { return ::abs(x); }
    inline double floor(double x) { return ::floor(x); }
    inline float floor(float x) { return ::floorf(x); }
    inline double log(double x) { return ::log(x); }
    inline float log(float x) { return ::logf(x); }
}


#endif // MANDEL_TYPES_H
