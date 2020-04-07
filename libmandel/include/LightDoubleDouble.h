#ifndef MANDEL_LIGHTDOUBLEDOUBLE_H
#define MANDEL_LIGHTDOUBLEDOUBLE_H

#include <utility>

namespace mnd
{
    struct LightDoubleDouble;

    namespace ldd
    {
        using DoublePair = std::pair<double, double>;
        inline static DoublePair quickTwoSum(double a, double b)
        {
            double s = a + b;
            double e = b - (s - a);
            return { s, e };
        }

        inline DoublePair twoSum(double a, double b)
        {
            double s = a + b;
            double v = s - a;
            double e = (a - (s - v)) + (b - v);
            return { s, e };
        }

        inline DoublePair split(double a)
        {
            static const double splitter = double((1ULL << 27) + 1);
            double t = splitter * a;
            double ahi = t - (t - a);
            double alo = a - ahi;
            return { ahi, alo };
        }

        inline DoublePair twoProd(double a, double b)
        {
            double p = a * b;
            auto [ahi, alo] = split(a);
            auto [bhi, blo] = split(b);
            double e = ((ahi * bhi - p) + ahi * blo + alo * bhi) + alo * blo;
            return { p, e };
        }

        inline DoublePair twoProdFma(double a, double b)
        {
            double p = a * b;
            double e = std::fma(a, b, -p);
            return { p, e };
        }
    }
}


struct mnd::LightDoubleDouble
{
    double x[2];

    inline LightDoubleDouble(double val) :
        x{ val, 0 }
    {}

    inline LightDoubleDouble(double u, double l) :
        x{ u, l }
    {}

    inline LightDoubleDouble(mnd::ldd::DoublePair dp) :
        x{ dp.first, dp.second }
    {}

    double operator[]           (int i) const   { return x[i]; }
    const double& operator[]    (int i)         { return x[i]; }

private:

};


inline mnd::LightDoubleDouble operator+(const mnd::LightDoubleDouble& a,
    const mnd::LightDoubleDouble& b)
{
    auto[s, e] = mnd::ldd::twoSum(a[0], b[0]);
    e += a[1] + b[1];
    return mnd::ldd::quickTwoSum(s, e);
}

inline mnd::LightDoubleDouble operator-(const mnd::LightDoubleDouble& a,
    const mnd::LightDoubleDouble& b)
{
    auto[s, e] = mnd::ldd::twoSum(a[0], -b[0]);
    e += a[1] - b[1];
    return mnd::ldd::quickTwoSum(s, e);
}

inline mnd::LightDoubleDouble operator*(const mnd::LightDoubleDouble& a,
    const mnd::LightDoubleDouble& b)
{
    auto[p1, p2] = mnd::ldd::twoProd(a[0], b[0]);
    p2 += a[0] * b[1] + a[1] * b[0];
    return mnd::ldd::quickTwoSum(p1, p2);
}


#endif // MANDEL_LIGHTDOUBLEDOUBLE_H
