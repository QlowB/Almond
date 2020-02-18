#ifndef MANDEL_TYPES_H
#define MANDEL_TYPES_H

#include <cinttypes>
#include <cmath>
#include <string>
#include "Fixed.h"

#define WITH_QD

#ifdef WITH_BOOST
#   include <boost/multiprecision/cpp_bin_float.hpp>
#   if defined(__GNUC__) || defined(__INTEL_COMPILER)
//#       include <boost/multiprecision/float128.hpp>
#   endif
#   include <boost/multiprecision/cpp_int.hpp>
#   include <boost/functional/hash.hpp>
#endif

#ifdef WITH_QD
#   include <qd/dd_real.h>
#   include <qd/qd_real.h>
#endif

namespace mnd
{


#ifdef WITH_BOOST
#   if 0//defined(__GNUC__) || defined(__INTEL_COMPILER)
    using Float128 = boost::multiprecision::float128;
#   else
    using Float128 = boost::multiprecision::cpp_bin_float_quad;
    /*using Float128 = boost::multiprecision::number<
        boost::multiprecision::backends::cpp_bin_float<
            112, boost::multiprecision::backends::digit_base_2, void, boost::int16_t, -16382, 16383>,
            boost::multiprecision::et_off>;*/
#   endif
    inline Float128 abs(const Float128& x) { return boost::multiprecision::abs(x); }
    inline Float128 floor(const Float128& x) { return boost::multiprecision::floor(x); }
    inline Float128 log(const Float128& x) { return boost::multiprecision::log(x); }
    inline Float128 log2(const Float128& x) { return boost::multiprecision::log2(x); }
    inline Float128 pow(const Float128& x, const Float128& y) { return boost::multiprecision::pow(x, y); }

    using Float256 = boost::multiprecision::number<
        boost::multiprecision::backends::cpp_bin_float<
            240, boost::multiprecision::backends::digit_base_2, void, boost::int16_t, -16382, 16383>,
            boost::multiprecision::et_off>;

    //using Float256 = long double;
    inline Float256 abs(const Float256& x) { return boost::multiprecision::abs(x); }
    inline Float256 floor(const Float256& x) { return boost::multiprecision::floor(x); }
    inline Float256 log(const Float256& x) { return boost::multiprecision::log(x); }
    inline Float256 log2(const Float256& x) { return boost::multiprecision::log2(x); }
    inline Float256 pow(const Float256& x, const Float256& y) { return boost::multiprecision::pow(x, y); }

    using Real = Float256;
    using Integer = boost::multiprecision::int256_t;
#else
    using Real = double;
    using Integer = int64_t;
#endif


#ifdef WITH_QD
    using DoubleDouble = dd_real;
    using QuadDouble = qd_real;

    inline DoubleDouble abs(const DoubleDouble& x) { return ::abs(x); }
    inline DoubleDouble floor(const DoubleDouble& x) { return ::floor(x); }
    inline DoubleDouble log(const DoubleDouble& x) { return ::log(x); }
    inline DoubleDouble log2(const DoubleDouble& x) { return ::log(x) / ::log(DoubleDouble(2.0)); }
    inline DoubleDouble pow(const DoubleDouble& x, const DoubleDouble& y) { return ::pow(x, y); }


    inline QuadDouble abs(const QuadDouble& x) { return ::abs(x); }
    inline QuadDouble floor(const QuadDouble& x) { return ::floor(x); }
    inline QuadDouble log(const QuadDouble& x) { return ::log(x); }
    inline QuadDouble log2(const QuadDouble& x) { return ::log(x) / ::log(QuadDouble(2.0)); }
    inline QuadDouble pow(const QuadDouble& x, const QuadDouble& y) { return ::pow(x, y); }
#endif

    inline double abs(double x) { return ::abs(x); }
    inline float abs(float x) { return ::abs(x); }
    inline double floor(double x) { return ::floor(x); }
    inline float floor(float x) { return ::floorf(x); }
    inline double log(double x) { return ::log(x); }
    inline float log(float x) { return ::logf(x); }
    inline double log2(double x) { return ::log2(x); }
    inline float log2(float x) { return ::log2f(x); }
    inline double pow(double x, double y) { return ::pow(x, y); }
    inline float pow(float x, float y) { return ::powf(x, y); }


    template<typename T, typename U>
    T convert(const U& x)
    {
        return static_cast<T>(x);
    }

#if defined(WITH_BOOST) && defined(WITH_QD)
    template<>
    inline DoubleDouble convert<DoubleDouble, Real>(const Real& x)
    {
        std::string s = x.str();
        return DoubleDouble(s.c_str());
    }

    template<>
    inline float convert<float, DoubleDouble>(const DoubleDouble& x)
    {
        return float(x.x[0] + x.x[1]);
    }

    template<>
    inline QuadDouble convert<QuadDouble, Real>(const Real& x)
    {
        std::string s = x.str();
        return QuadDouble(s.c_str());
    }

    template<>
    inline float convert<float, QuadDouble>(const QuadDouble& x)
    {
        return float(x.x[0] + x.x[1] + x.x[2] + x.x[3]);
    }

    template<>
    inline float convert<float, Fixed512>(const Fixed512& x)
    {
        return float(Real(x));
    }
#endif

    std::string toString(const Real& num);


/*
    template<typename T>
    constexpr mnd::Real precision(void);
*/
}


#endif // MANDEL_TYPES_H
