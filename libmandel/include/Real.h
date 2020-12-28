#ifndef MANDEL_REAL_H
#define MANDEL_REAL_H

#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>

namespace mnd
{
    using Float512 = boost::multiprecision::number<
        boost::multiprecision::backends::cpp_bin_float<
            496, boost::multiprecision::backends::digit_base_2, void, boost::int16_t, -16382, 16383>,
            boost::multiprecision::et_off>;

    using Float2048 = boost::multiprecision::number<
        boost::multiprecision::backends::cpp_bin_float<
            2016, boost::multiprecision::backends::digit_base_2, std::allocator<void>, boost::int32_t, -16777214, 16777215>,
            boost::multiprecision::et_off>;

    using Real = Float512;
    using Integer = boost::multiprecision::int512_t;

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

    std::string toString(const Real& num);
    std::string toLegibleString(const Real& num);
}


#endif // MANDEL_REAL_H

