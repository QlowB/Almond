#ifndef MANDEL_OCTADOUBLE_H
#define MANDEL_OCTADOUBLE_H

#include "PolyfloatUtil.h"

#ifndef _MSC_VER
#   define likely(x)      __builtin_expect(!!(x), 1) 
#else
#   define likely(x)      (x)
#endif

namespace mnd
{
    struct OctaDouble;
}

struct mnd::OctaDouble
{
    double x[8];

    inline OctaDouble(double val) :
        x{ val, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }
    {}

    inline OctaDouble(double a, double b, double c, double d, double e, double f, double g, double h) :
        x{ a, b, c, d, e, f, g, h }
    {}

    double operator[]           (int i) const   { return x[i]; }
    const double& operator[]    (int i)         { return x[i]; }
};


inline mnd::OctaDouble operator+(const mnd::OctaDouble& a,
    const mnd::OctaDouble& b)
{
    auto[a0, a1] = mnd::pfu::twoSum(a[0], b[0]);
    auto[b0, b1] = mnd::pfu::twoSum(a[1], b[1]);
    auto[c0, c1] = mnd::pfu::twoSum(a[2], b[2]);
    auto[d0, d1] = mnd::pfu::twoSum(a[3], b[3]);
    auto[e0, e1] = mnd::pfu::twoSum(a[4], b[4]);
    auto[f0, f1] = mnd::pfu::twoSum(a[5], b[5]);
    auto[g0, g1] = mnd::pfu::twoSum(a[6], b[6]);

    if (likely(a1 + b0 != b0)) {
        auto t0 = a0;
        auto [t1, p1] = mnd::pfu::twoSum(a1, b0);
        auto [t2, p2] = mnd::pfu::threeTwoSum(b1, c0, p1);
        auto [t3, p3] = mnd::pfu::threeTwoSum(c1, d0, p2);
        auto [t4, p4] = mnd::pfu::threeTwoSum(d1, e0, p3);
        auto [t5, p5] = mnd::pfu::threeTwoSum(e1, f0, p4);
        auto [t6, p6] = mnd::pfu::threeTwoSum(f1, g0, p5);
        auto t7 = a[7] + b[7] + g1 + p6;

        auto[re0, er1] = mnd::pfu::quickTwoSum(t0, t1);
        auto[re1, e2] = mnd::pfu::quickTwoSum(er1, t2);
        auto[re2, e3] = mnd::pfu::quickTwoSum(e2, t3);
        auto[re3, e4] = mnd::pfu::quickTwoSum(e3, t4);
        auto[re4, e5] = mnd::pfu::quickTwoSum(e4, t5);
        auto[re5, e6] = mnd::pfu::quickTwoSum(e5, t6);
        auto[re6, re7] = mnd::pfu::quickTwoSum(e6, t7);

        //return { t1, t2, t3, t4, t5, t6 };
        return { re0, re1, re2, re3, re4, re5, re6, re7 };
    }
    auto t0 = a0;
    auto [t1, p1] = mnd::pfu::twoSum(a1, b0);
    auto [t2, p2, p3] = mnd::pfu::threeSum(b1, c0, p1);
    auto [t3, p4, p5, p6] = mnd::pfu::fourSum(c1, d0, p2, p3);
    auto [t4, p7, p8] = mnd::pfu::sixThreeSum(d1, e0, p4, p5, p6, 0.0);
    auto [t5, p9, p10] = mnd::pfu::sixThreeSum(e1, f0, p7, p8, 0.0, 0.0);
    auto [t6, p11] = mnd::pfu::fiveTwoSum(f1, g0, p9, p10, 0.0);
    auto t7 = a[7] + b[7] + g1 + p11;

    auto[re0, er1] = mnd::pfu::quickTwoSum(t0, t1);
    auto[re1, e2] = mnd::pfu::quickTwoSum(er1, t2);
    auto[re2, e3] = mnd::pfu::quickTwoSum(e2, t3);
    auto[re3, e4] = mnd::pfu::quickTwoSum(e3, t4);
    auto[re4, e5] = mnd::pfu::quickTwoSum(e4, t5);
    auto[re5, e6] = mnd::pfu::quickTwoSum(e5, t6);
    auto[re6, re7] = mnd::pfu::quickTwoSum(e6, t7);

    //return { t1, t2, t3, t4, t5, t6 };
    return { re0, re1, re2, re3, re4, re5, re6, re7 };
}


inline bool operator>(const mnd::OctaDouble& a, const mnd::OctaDouble& b)
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


inline mnd::OctaDouble operator-(const mnd::OctaDouble& a,
    const mnd::OctaDouble& b)
{
    return a + mnd::OctaDouble{ -b[0], -b[1], -b[2], -b[3], -b[4], -b[5], -b[6], -b[7] };
}


inline mnd::OctaDouble operator*(const mnd::OctaDouble& a,
    const mnd::OctaDouble& b)
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

    auto[p6_5, p7_0] = mnd::pfu::twoProd(a[5], b[0]);
    auto[p6_6, p7_1] = mnd::pfu::twoProd(a[4], b[1]);
    auto[p6_7, p7_2] = mnd::pfu::twoProd(a[3], b[2]);
    auto[p6_8, p7_3] = mnd::pfu::twoProd(a[2], b[3]);
    auto[p6_9, p7_4] = mnd::pfu::twoProd(a[1], b[4]);
    auto[p6_10, p7_5] = mnd::pfu::twoProd(a[0], b[5]);

    auto[p7_6, p8_0] = mnd::pfu::twoProd(a[6], b[0]);
    auto[p7_7, p8_1] = mnd::pfu::twoProd(a[5], b[1]);
    auto[p7_8, p8_2] = mnd::pfu::twoProd(a[4], b[2]);
    auto[p7_9, p8_3] = mnd::pfu::twoProd(a[3], b[3]);
    auto[p7_10, p8_4] = mnd::pfu::twoProd(a[2], b[4]);
    auto[p7_11, p8_5] = mnd::pfu::twoProd(a[1], b[5]);
    auto[p7_12, p8_6] = mnd::pfu::twoProd(a[0], b[6]);

    auto t1 = p1_0;
    auto[t2, tl3, tl4] = mnd::pfu::threeSum(p2_0, p2_1, p2_2);
    auto[t3, tl4_2, tl5] = mnd::pfu::sixThreeSum(p3_0, p3_1, p3_2, p3_3, p3_4, tl3);
    auto[t4, tl5_2, tl6] = mnd::pfu::nineThreeSum(p4_0, p4_1, p4_2, p4_3, p4_4, p4_5, p4_6, tl4, tl4_2);
    auto[x1, x2, x3] = mnd::pfu::nineThreeSum(p5_0, p5_1, p5_2, p5_3, p5_4, p5_5, p5_6, p5_7, p5_8);
    auto[t5, tl6_1, tl7] = mnd::pfu::sixThreeSum(x1, x2, x3, tl5, tl5_2, 0.0);
    auto[x4, x5, x6] = mnd::pfu::nineThreeSum(p6_0, p6_1, p6_2, p6_3, p6_4, p6_5, p6_6, p6_7, p6_8);
    auto[t6, tl7_1, tl8] = mnd::pfu::nineThreeSum(x4, x5, x6, tl6, tl6_1, p6_9, p6_10, 0.0, 0.0);
    auto[x7, x8, x9] = mnd::pfu::nineThreeSum(p7_0, p7_1, p7_2, p7_3, p7_4, p7_5, p7_6, p7_7, p7_8);
    auto[t7, tl8_1, tl9] = mnd::pfu::nineThreeSum(x7, x8, x9, tl7, tl7_1, p7_9, p7_10, p7_11, p7_12);

    auto t8 = tl8 + tl8_1 + tl9 + p8_0 + p8_1 + p8_2 + p8_3 + p8_4 + p8_5 + p8_6 +
        a[7] * b[0] + a[6] * b[1] + a[5] * b[2] + a[5] * b[3] +
        a[3] * b[4] + a[2] * b[5] + a[1] * b[6] + a[0] * b[7];

    auto[re0, e1] = mnd::pfu::quickTwoSum(t1, t2);
    auto[re1, e2] = mnd::pfu::quickTwoSum(e1, t3);
    auto[re2, e3] = mnd::pfu::quickTwoSum(e2, t4);
    auto[re3, e4] = mnd::pfu::quickTwoSum(e3, t5);
    auto[re4, e5] = mnd::pfu::quickTwoSum(e4, t6);
    auto[re5, e6] = mnd::pfu::quickTwoSum(e5, t7);
    auto[re6, re7] = mnd::pfu::quickTwoSum(e6, t8);

    return { re0, re1, re2, re3, re4, re5, re6, re7 };

    /*double r[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    for (int i = 0; i < 8; i++) {
#pragma GCC unroll(8)
        for (int j = 0; i + j < 8; j++) {
            auto[s, e] = mnd::pfu::twoProd(a[i], b[j]);

#pragma GCC unroll(8)
            for (int k = 0; k < 8; k++) {
                auto[x1, x2, x3] = mnd::pfu::threeSum(r[k], s, e);
                r[k] = x1;
                s = x2;
                e = x3;
            }
        }
    }

    return mnd::OctaDouble{ r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7] };
    */
}


#endif // MANDEL_OCTADOUBLE_H

