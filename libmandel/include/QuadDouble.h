#ifndef MANDEL_QUADDOUBLE_H
#define MANDEL_QUADDOUBLE_H

#include "PolyfloatUtil.h"

namespace mnd
{
    struct QuadDouble;

    inline mnd::pfu::Quadruple<double> renorm2(double x0, double x1, double x2, double x3, double x4)
    {
        auto [st1, t4] = mnd::pfu::quickTwoSum(x3, x4);
        auto [st2, t3] = mnd::pfu::quickTwoSum(x2, st1);
        auto [st3, t2] = mnd::pfu::quickTwoSum(x1, st2);
        auto [t0, t1] = mnd::pfu::quickTwoSum(x0, st3);

        auto e = t0;

        auto [r0, e1] = mnd::pfu::quickTwoSum(e, t1);
        auto [r1, e2] = mnd::pfu::quickTwoSum(e1, t2);
        auto [r2, e3] = mnd::pfu::quickTwoSum(e2, t3);
        auto r3 = e3 + t4;
        return { r0, r1, r2, r3 };
    }
}

struct mnd::QuadDouble
{
    double x[4];

    inline QuadDouble(double val) :
        x{ val, 0.0, 0.0, 0.0 }
    {}

    inline QuadDouble(double a, double b, double c, double d) :
        x{ a, b, c, d }
    {}

    double operator[]           (int i) const   { return x[i]; }
    const double& operator[]    (int i)         { return x[i]; }
};


inline mnd::QuadDouble operator+(const mnd::QuadDouble& a,
    const mnd::QuadDouble& b)
{
    auto[s0, e0] = mnd::pfu::twoSum(a[0], b[0]);
    auto[s1, e1] = mnd::pfu::twoSum(a[1], b[1]);
    auto[s2, e2] = mnd::pfu::twoSum(a[2], b[2]);
    auto[s3, e3] = mnd::pfu::twoSum(a[3], b[3]);
    auto r0 = s0;

    auto [r1, t0] = mnd::pfu::twoSum(s1, e0);
    auto [r2, t1, t2] = mnd::pfu::threeSum(s2, e1, t0);
    auto [r3, t3, _t4] = mnd::pfu::threeSum(s3, e2, t1);
    auto [r4, _t5, _t6] = mnd::pfu::threeSum(e3, t3, t2);

    auto[re0, er1] = mnd::pfu::quickTwoSum(r0, r1);
    auto[re1, er2] = mnd::pfu::quickTwoSum(er1, r2);
    auto[re2, er3] = mnd::pfu::quickTwoSum(er2, r3);
    auto[re3, er4] = mnd::pfu::quickTwoSum(er3, r4);
    return { re0, re1, re2, re3 };
}


inline bool operator>(const mnd::QuadDouble& a, const mnd::QuadDouble& b)
{
    if (a[0] > b[0])
        return true;
    if (a[0] < b[0])
        return false;
    if (a[1] > b[1])
        return true;
    if (a[1] < b[1])
        return false;
    if (a[2] > b[2])
        return true;
    if (a[2] < b[2])
        return false;
    return a[3] > b[3];
}


inline mnd::QuadDouble operator-(const mnd::QuadDouble& a,
    const mnd::QuadDouble& b)
{
    return a + mnd::QuadDouble{ -b[0], -b[1], -b[2], -b[3] };
}


inline mnd::QuadDouble operator*(const mnd::QuadDouble& a,
    const mnd::QuadDouble& b)
{
    auto[a0, b0] = mnd::pfu::twoProd(a[0], b[0]);
    auto[b1, c0] = mnd::pfu::twoProd(a[0], b[1]);
    auto[b2, c1] = mnd::pfu::twoProd(a[1], b[0]);
    auto[c2, d0] = mnd::pfu::twoProd(a[0], b[2]);
    auto[c3, d1] = mnd::pfu::twoProd(a[1], b[1]);
    auto[c4, d2] = mnd::pfu::twoProd(a[2], b[0]);

    auto d5 = a[3] * b[0];
    auto d6 = a[2] * b[1];
    auto d7 = a[1] * b[2];
    auto d8 = a[0] * b[3];

    auto r0 = a0;
    auto[r1, c5, d3] = mnd::pfu::threeSum(b0, b1, b2);
    auto[r2, d4, e0] = mnd::pfu::sixThreeSum(c0, c1, c2, c3, c4, c5);
    auto[r3, e1, ex_] = mnd::pfu::nineThreeSum(d0, d1, d2, d3, d4, d5, d6, d7, d8);
    auto r4 = e0 + e1;

    auto [n0, n1, n2, n3] = mnd::renorm2(r0, r1, r2, r3, r4);

    return { n0, n1, n2, n3 };
}


#endif // MANDEL_QUADDOUBLE_H

