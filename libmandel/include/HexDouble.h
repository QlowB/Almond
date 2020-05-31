#ifndef MANDEL_HEXDOUBLE_H
#define MANDEL_HEXDOUBLE_H

#include "PolyfloatUtil.h"

namespace mnd
{
    struct HexDouble;
}

struct mnd::HexDouble
{
    double x[6];

    inline HexDouble(double val) :
        x{ val, 0.0, 0.0, 0.0, 0.0, 0.0 }
    {}

    inline HexDouble(double a, double b, double c, double d, double e, double f) :
        x{ a, b, c, d, e, f }
    {}

    double operator[]           (int i) const   { return x[i]; }
    const double& operator[]    (int i)         { return x[i]; }
};


inline mnd::HexDouble operator+(const mnd::HexDouble& a,
    const mnd::HexDouble& b)
{
    auto[a0, a1] = mnd::pfu::twoSum(a[0], b[0]);
    auto[b0, b1] = mnd::pfu::twoSum(a[1], b[1]);
    auto[c0, c1] = mnd::pfu::twoSum(a[2], b[2]);
    auto[d0, d1] = mnd::pfu::twoSum(a[3], b[3]);
    auto[e0, e1] = mnd::pfu::twoSum(a[4], b[4]);

    auto t0 = a0;
    auto [t1, p1] = mnd::pfu::twoSum(a1, b0);
    auto [t2, p2, p3] = mnd::pfu::threeSum(b1, c0, p1);
    auto [t3, p4, p5, p6] = mnd::pfu::fourSum(c1, d0, p2, p3);
    auto [t4, p7] = mnd::pfu::fiveTwoSum(d1, e0, p4, p5, p6);
    auto t5 = a[5] + b[5] + e1 + p7;

    auto[re0, er1] = mnd::pfu::quickTwoSum(t0, t1);
    auto[re1, e2] = mnd::pfu::quickTwoSum(er1, t2);
    auto[re2, e3] = mnd::pfu::quickTwoSum(e2, t3);
    auto[re3, e4] = mnd::pfu::quickTwoSum(e3, t4);
    auto[re4, re5] = mnd::pfu::quickTwoSum(e4, t5);

    //return { t1, t2, t3, t4, t5, t6 };
    return { re0, re1, re2, re3, re4, re5 };
}


inline bool operator>(const mnd::HexDouble& a, const mnd::HexDouble& b)
{
    if (a[0] > b[0])
        return true;
    if (a[0] < b[0])
        return false;
    if (a[1] > b[1])
        return true;
    if (a[1] < b[1])
        return false;
    return a[2] > b[2];
}


inline mnd::HexDouble operator-(const mnd::HexDouble& a,
    const mnd::HexDouble& b)
{
    return a + mnd::HexDouble{ -b[0], -b[1], -b[2], -b[3], -b[4], -b[5] };
}


inline mnd::HexDouble operator*(const mnd::HexDouble& a,
    const mnd::HexDouble& b)
{
    auto[p1_0, p2_0] = mnd::pfu::twoProd(a[0], b[0]);

    auto[p2_1, p3_0] = mnd::pfu::twoProd(a[0], b[1]);
    auto[p2_2, p3_1] = mnd::pfu::twoProd(a[1], b[0]);

    auto[p3_2, p4_0] = mnd::pfu::twoProd(a[2], b[0]);
    auto[p3_3, p4_1] = mnd::pfu::twoProd(a[1], b[1]);
    auto[p3_4, p4_2] = mnd::pfu::twoProd(a[0], b[2]);

    auto[p4_3, p5_0] = mnd::pfu::twoProd(a[3], b[0]);
    auto[p4_4, p5_1] = mnd::pfu::twoProd(a[2], b[1]);
    auto[p4_5, p5_2] = mnd::pfu::twoProd(a[1], b[2]);
    auto[p4_6, p5_3] = mnd::pfu::twoProd(a[0], b[3]);

    auto[p5_4, p6_0] = mnd::pfu::twoProd(a[4], b[0]);
    auto[p5_5, p6_1] = mnd::pfu::twoProd(a[3], b[1]);
    auto[p5_6, p6_2] = mnd::pfu::twoProd(a[2], b[2]);
    auto[p5_7, p6_3] = mnd::pfu::twoProd(a[1], b[3]);
    auto[p5_8, p6_4] = mnd::pfu::twoProd(a[0], b[4]);

    auto t1 = p1_0;
    auto[t2, tl3, tl4] = mnd::pfu::threeSum(p2_0, p2_1, p2_2);
    auto[t3, tl4_2, tl5] = mnd::pfu::sixThreeSum(p3_0, p3_1, p3_2, p3_3, p3_4, tl3);
    auto[t4, tl5_2, tl6] = mnd::pfu::nineThreeSum(p4_0, p4_1, p4_2, p4_3, p4_4, p4_5, p4_6, tl4, tl4_2);
    auto[x1, x2, x3] = mnd::pfu::nineThreeSum(p5_0, p5_1, p5_2, p5_3, p5_4, p5_5, p5_6, p5_7, p5_8);
    auto[t5, tl6_1, tl7] = mnd::pfu::sixThreeSum(x1, x2, x3, tl5, tl5_2, 0.0);

    auto t6 = tl6 + tl6_1 + tl7 + p6_0 + p6_1 + p6_2 + p6_3 + p6_4 +
        a[5] * b[0] + a[4] * b[1] + a[3] * b[2] + a[2] * b[3] + a[1] * b[4] + a[0] * b[5];

    auto[re0, e1] = mnd::pfu::quickTwoSum(t1, t2);
    auto[re1, e2] = mnd::pfu::quickTwoSum(e1, t3);
    auto[re2, e3] = mnd::pfu::quickTwoSum(e2, t4);
    auto[re3, e4] = mnd::pfu::quickTwoSum(e3, t5);
    auto[re4, re5] = mnd::pfu::quickTwoSum(e4, t6);

    //return { t1, t2, t3, t4, t5, t6 };
    return { re0, re1, re2, re3, re4, re5 };
}


#endif // MANDEL_HEXDOUBLE_H

