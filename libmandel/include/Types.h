#ifndef MANDEL_TYPES_H
#define MANDEL_TYPES_H

#include <cinttypes>
#include <cmath>
#include <string>
#include "Fixed.h"

#ifndef WITH_QD
#define WITH_QD
#endif

#ifdef WITH_BOOST
#   include <boost/multiprecision/cpp_bin_float.hpp>
#   if defined(__GNUC__) || defined(__INTEL_COMPILER)
//#       include <boost/multiprecision/float128.hpp>
#   endif
#   include <boost/multiprecision/cpp_int.hpp>
#   include <boost/functional/hash.hpp>
#endif

#include <qd/dd_real.h>
#include <qd/qd_real.h>

#include "LightDoubleDouble.h"
#include "TripleDouble.h"

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
    inline Float128 sqrt(const Float128& x) { return boost::multiprecision::sqrt(x); }
    inline Float128 floor(const Float128& x) { return boost::multiprecision::floor(x); }
    inline Float128 log(const Float128& x) { return boost::multiprecision::log(x); }
    inline Float128 log2(const Float128& x) { return boost::multiprecision::log2(x); }
    inline Float128 pow(const Float128& x, const Float128& y) { return boost::multiprecision::pow(x, y); }
    inline Float128 atan2(const Float128& y, const Float128& x) { return boost::multiprecision::atan2(y, x); }
    inline Float128 cos(const Float128& x) { return boost::multiprecision::cos(x); }
    inline Float128 sin(const Float128& x) { return boost::multiprecision::sin(x); }
    inline Float128 exp(const Float128& x) { return boost::multiprecision::exp(x); }

    using Float256 = boost::multiprecision::number<
        boost::multiprecision::backends::cpp_bin_float<
            240, boost::multiprecision::backends::digit_base_2, void, boost::int16_t, -16382, 16383>,
            boost::multiprecision::et_off>;

    //using Float256 = long double;
    inline Float256 abs(const Float256& x) { return boost::multiprecision::abs(x); }
    inline Float256 sqrt(const Float256& x) { return boost::multiprecision::sqrt(x); }
    inline Float256 floor(const Float256& x) { return boost::multiprecision::floor(x); }
    inline Float256 log(const Float256& x) { return boost::multiprecision::log(x); }
    inline Float256 log2(const Float256& x) { return boost::multiprecision::log2(x); }
    inline Float256 pow(const Float256& x, const Float256& y) { return boost::multiprecision::pow(x, y); }
    inline Float256 atan2(const Float256& y, const Float256& x) { return boost::multiprecision::atan2(y, x); }
    inline Float256 cos(const Float256& x) { return boost::multiprecision::cos(x); }
    inline Float256 sin(const Float256& x) { return boost::multiprecision::sin(x); }
    inline Float256 exp(const Float256& x) { return boost::multiprecision::exp(x); }

    using Float512 = boost::multiprecision::number<
        boost::multiprecision::backends::cpp_bin_float<
            496, boost::multiprecision::backends::digit_base_2, void, boost::int16_t, -16382, 16383>,
            boost::multiprecision::et_off>;

    inline Float512 abs(const Float512& x) { return boost::multiprecision::abs(x); }
    inline Float512 sqrt(const Float512& x) { return boost::multiprecision::sqrt(x); }
    inline Float512 floor(const Float512& x) { return boost::multiprecision::floor(x); }
    inline Float512 log(const Float512& x) { return boost::multiprecision::log(x); }
    inline Float512 log2(const Float512& x) { return boost::multiprecision::log2(x); }
    inline Float512 pow(const Float512& x, const Float512& y) { return boost::multiprecision::pow(x, y); }
    inline Float512 atan2(const Float512& y, const Float512& x) { return boost::multiprecision::atan2(y, x); }
    inline Float512 cos(const Float512& x) { return boost::multiprecision::cos(x); }
    inline Float512 sin(const Float512& x) { return boost::multiprecision::sin(x); }
    inline Float512 exp(const Float512& x) { return boost::multiprecision::exp(x); }

    using Real = Float512;
    using Integer = boost::multiprecision::int512_t;
    /*boost::multiprecision::number<
        boost::multiprecision::backends::cpp_bin_float<
            1500, boost::multiprecision::backends::digit_base_2, void, boost::int16_t, -16382, 16383>,
            boost::multiprecision::et_off>;

    inline Real abs(const Real& x) { return boost::multiprecision::abs(x); }
    inline Real sqrt(const Real& x) { return boost::multiprecision::sqrt(x); }
    inline Real floor(const Real& x) { return boost::multiprecision::floor(x); }
    inline Real log(const Real& x) { return boost::multiprecision::log(x); }
    inline Real log2(const Real& x) { return boost::multiprecision::log2(x); }
    inline Real pow(const Real& x, const Real& y) { return boost::multiprecision::pow(x, y); }
    inline Real atan2(const Real& y, const Real& x) { return boost::multiprecision::atan2(y, x); }
    inline Real cos(const Real& x) { return boost::multiprecision::cos(x); }
    inline Real sin(const Real& x) { return boost::multiprecision::sin(x); }
    inline Real exp(const Real& x) { return boost::multiprecision::exp(x); }
*/
#else
    using Real = double;
    using Integer = int64_t;
#endif


    using DoubleDouble = dd_real;
    using QuadDouble = qd_real;

    inline DoubleDouble abs(const DoubleDouble& x) { return ::abs(x); }
    inline DoubleDouble sqrt(const DoubleDouble& x) { return ::sqrt(x); }
    inline DoubleDouble floor(const DoubleDouble& x) { return ::floor(x); }
    inline DoubleDouble log(const DoubleDouble& x) { return ::log(x); }
    inline DoubleDouble log2(const DoubleDouble& x) { return ::log(x) / ::log(DoubleDouble(2.0)); }
    inline DoubleDouble pow(const DoubleDouble& x, const DoubleDouble& y) { return ::pow(x, y); }
    inline DoubleDouble atan2(const DoubleDouble& y, const DoubleDouble& x) { return ::atan2(y, x); }
    inline DoubleDouble cos(const DoubleDouble& x) { return ::cos(x); }
    inline DoubleDouble sin(const DoubleDouble& x) { return ::sin(x); }
    inline DoubleDouble exp(const DoubleDouble& x) { return ::exp(x); }


    inline QuadDouble abs(const QuadDouble& x) { return ::abs(x); }
    inline QuadDouble sqrt(const QuadDouble& x) { return ::sqrt(x); }
    inline QuadDouble floor(const QuadDouble& x) { return ::floor(x); }
    inline QuadDouble log(const QuadDouble& x) { return ::log(x); }
    inline QuadDouble log2(const QuadDouble& x) { return ::log(x) / ::log(QuadDouble(2.0)); }
    inline QuadDouble pow(const QuadDouble& x, const QuadDouble& y) { return ::pow(x, y); }
    inline QuadDouble atan2(const QuadDouble& y, const QuadDouble& x) { return ::atan2(y, x); }
    inline QuadDouble cos(const QuadDouble& x) { return ::cos(x); }
    inline QuadDouble sin(const QuadDouble& x) { return ::sin(x); }
    inline QuadDouble exp(const QuadDouble& x) { return ::exp(x); }

    inline double abs(double x) { return ::abs(x); }
    inline float abs(float x) { return ::abs(x); }
    inline double sqrt(double x) { return ::sqrt(x); }
    inline float sqrt(float x) { return ::sqrtf(x); }
    inline double floor(double x) { return ::floor(x); }
    inline float floor(float x) { return ::floorf(x); }
    inline double log(double x) { return ::log(x); }
    inline float log(float x) { return ::logf(x); }
    inline double log2(double x) { return ::log2(x); }
    inline float log2(float x) { return ::log2f(x); }
    inline double atan2(double x, double y) { return ::atan2(x, y); }
    inline float atan2(float x, float y) { return ::atan2(x, y); }
    inline double pow(double x, double y) { return ::pow(x, y); }
    inline float pow(float x, float y) { return ::powf(x, y); }
    inline double cos(double x) { return ::cos(x); }
    inline float cos(float x) { return ::cos(x); }
    inline double sin(double x) { return ::sin(x); }
    inline float sin(float x) { return ::sin(x); }
    inline double exp(double x) { return ::exp(x); }
    inline float exp(float x) { return ::exp(x); }


    template<typename T, typename U>
    T convert(const U& x)
    {
        return static_cast<T>(x);
    }

#if defined(WITH_BOOST)
    template<>
    inline DoubleDouble convert<DoubleDouble, Real>(const Real& x)
    {
        double s = static_cast<double>(x);
        double e = static_cast<double>(x - s);
        return DoubleDouble{ s, e };
    }

    template<>
    inline LightDoubleDouble convert<LightDoubleDouble, Real>(const Real& x)
    {
        double s = static_cast<double>(x);
        double e = static_cast<double>(x - s);
        return LightDoubleDouble{ s, e };
    }

    template<>
    inline float convert<float, DoubleDouble>(const DoubleDouble& x)
    {
        return float(x.x[0] + x.x[1]);
    }

    template<>
    inline float convert<float, LightDoubleDouble>(const LightDoubleDouble& x)
    {
        return float(x[0] + x[1]);
    }

    template<>
    inline Real convert<Real, LightDoubleDouble>(const LightDoubleDouble& x)
    {
        return Real{ x[0] } + x[1];
    }

    template<>
    inline TripleDouble convert<TripleDouble, Real>(const Real& x)
    {
        double s = static_cast<double>(x);
        Real tmp = x - s;
        double e1 = static_cast<double>(tmp);
        tmp = tmp - e1;
        double e2 = static_cast<double>(tmp);
        return TripleDouble { s, e1, e2 };
    }

    template<>
    inline Real convert<Real, TripleDouble>(const TripleDouble& x)
    {
        return Real{ x[0] } + Real{ x[1] } + Real{ x[2] };
    }

    template<>
    inline float convert<float, TripleDouble>(const TripleDouble& x)
    {
        return float(x.x[0] + x.x[1] + x.x[2]);
    }

    template<>
    inline QuadDouble convert<QuadDouble, Real>(const Real& x)
    {
        double s = static_cast<double>(x);
        Real tmp = x - s;
        double e1 = static_cast<double>(tmp);
        tmp = tmp - e1;
        double e2 = static_cast<double>(tmp);
        tmp = tmp - e2;
        double e3 = static_cast<double>(tmp);
        return QuadDouble{ s, e1, e2, e3 };
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

    template<>
    inline Fixed64 convert<Fixed64, Real>(const Real& x)
    {
        return Fixed64{ static_cast<int64_t>(x * 0xFFFFFFFFFFFFLL), true };
    }

    template<>
    inline Fixed128 convert<Fixed128, Real>(const Real& x)
    {
        mnd::Integer i = mnd::Integer(x * mnd::pow(Real(1LL << 32), 3));
        uint64_t lo = uint64_t(i & 0xFFFFFFFFFFFFFFFFULL);
        uint64_t hi = uint64_t((i >> 64) & 0xFFFFFFFFFFFFFFFFULL);
        return Fixed128(hi, lo);
    }
#endif

    std::string toString(const Real& num);
    std::string toLegibleString(const Real& num);


/*
    template<typename T>
    constexpr mnd::Real precision(void);
*/
}


#endif // MANDEL_TYPES_H
