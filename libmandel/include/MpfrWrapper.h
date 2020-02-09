#ifndef MANDEL_MPFRWRAPPER_H
#define MANDEL_MPFRWRAPPER_H

#include <mpfr.h>
#include <string>

namespace mnd
{
    template<unsigned int bits>
    class MpfrFloat;
}


template<unsigned int bits>
class MpfrFloat
{
    mpfr_t val;
public:
    inline MpfrFloat(void)
    {
        mpfr_init2(val, bits);
    }

    inline MpfrFloat(const MpfrFloat<bits>& other) : MpfrFloat()
    {
        mpfr_set(val, other, MPFR_RNDN);
    }

    inline MpfrFloat(MpfrFloat<bits>&& other)
    {
        val->_mpfr_d = nullptr;
        mpfr_swap(val, other, MPFR_RNDN);
    }

    inline ~MpfrFloat(void)
    {
        if (val->_mpfr_d != nullptr)
            mpfr_clear(val);
    }

    inline MpfrFloat<bits>& operator=(const MpfrFloat<bits>& other)
    {
        mpfr_set(val, other, MPFR_RNDN);
    }

    inline MpfrFloat<bits>& operator=(MpfrFloat<bits>&& other)
    {
        mpfr_swap(val, other, MPFR_RNDN);
    }

    inline MpfrFloat(double v)
    {
        mpfr_init2(val, bits);
        mpfr_set_d(val, v, MPFR_RNDD);
    }

    inline MpfrFloat(const char* str)
    {
        mpfr_init2(val, bits);
        mpfr_set_str(val, str, 10, MPFR_RNDD);
    }

    inline MpfrFloat<bits>& operator+=(const MpfrFloat<bits>& other)
    {
        mpfr_add(val, val, other.val);
    }

    inline MpfrFloat<bits>& operator-=(const MpfrFloat<bits>& other)
    {
        mpfr_sub(val, val, other.val);
    }

    inline MpfrFloat<bits>& operator*=(const MpfrFloat<bits>& other)
    {
        mpfr_mul(val, val, other.val);
    }

    inline MpfrFloat<bits>& operator/=(const MpfrFloat<bits>& other)
    {
        mpfr_div(val, val, other.val);
    }

    inline MpfrFloat<bits> operator+(const MpfrFloat<bits>& other) const
    {
        MpfrFloat<bits> result = *this;
        result += other;
        return result;
    }

    inline MpfrFloat<bits> operator-(const MpfrFloat<bits>& other) const
    {
        MpfrFloat<bits> result = *this;
        result -= other;
        return result;
    }

    inline MpfrFloat<bits> operator*(const MpfrFloat<bits>& other) const
    {
        MpfrFloat<bits> result = *this;
        result *= other;
        return result;
    }

    inline MpfrFloat<bits> operator/(const MpfrFloat<bits>& other) const
    {
        MpfrFloat<bits> result = *this;
        result /= other;
        return result;
    }

    inline std::string toString(void) const
    {
        char buf[32 + bits / 2];
        int len = mpfr_snprintf(buf, sizeof buf, "%R", val);
        if (len > sizeof buf) {
            // error
        }
        return std::string(buf);
    }
};

#endif // MANDEL_MPFRWRAPPER_H
