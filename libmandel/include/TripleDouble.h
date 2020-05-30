#ifndef MANDEL_TRIPLEDOUBLE_H
#define MANDEL_TRIPLEDOUBLE_H

#include "PolyfloatUtil.h"

namespace mnd
{
    struct TripleDouble;
}

struct mnd::TripleDouble
{
    double x[3];

    inline TripleDouble(double val) :
        x{ val, 0.0, 0.0 }
    {}

    inline TripleDouble(double a, double b, double c) :
        x{ a, b, c }
    {}

    double operator[]           (int i) const   { return x[i]; }
    const double& operator[]    (int i)         { return x[i]; }
};


inline mnd::TripleDouble operator+(const mnd::TripleDouble& a,
    const mnd::TripleDouble& b)
{
    auto[r0, t0] = mnd::pfu::twoSum(a[0], b[0]);
    auto[t1, t2] = mnd::pfu::twoSum(a[1], b[1]);
    auto[r1, t3] = mnd::pfu::twoSum(t0, t1);
    auto r2 = t2 + t3 + a[2] + b[2];

    auto[re1, t4] = mnd::pfu::quickTwoSum(r0, r1);
    auto[re2, re3] = mnd::pfu::quickTwoSum(t4, r2);
    return { re1, re2, re3 };
}


inline bool operator>(const mnd::TripleDouble& a, const mnd::TripleDouble& b)
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

inline mnd::TripleDouble operator-(const mnd::TripleDouble& a,
    const mnd::TripleDouble& b)
{
    auto[r0, t0] = mnd::pfu::twoSum(a[0], -b[0]);
    auto[t1, t2] = mnd::pfu::twoSum(a[1], -b[1]);
    auto[r1, t3] = mnd::pfu::twoSum(t0, t1);
    auto r2 = t2 + t3 + a[2] - b[2];

    auto[re1, t4] = mnd::pfu::quickTwoSum(r0, r1);
    auto[re2, re3] = mnd::pfu::quickTwoSum(t4, r2);
    return { re1, re2, re3 };
}


inline mnd::TripleDouble operator*(const mnd::TripleDouble& a,
    const mnd::TripleDouble& b)
{
    auto[p1_0, p2_0] = mnd::pfu::twoProd(a[0], b[0]);
    auto[p2_1, p3_0] = mnd::pfu::twoProd(a[0], b[1]);
    auto[p2_2, p3_1] = mnd::pfu::twoProd(a[1], b[0]);

    auto[t2, tl3] = mnd::pfu::threeTwoSum(p2_0, p2_1, p2_2);
    auto t3 = tl3 + p3_0 + p3_1 + a[1] * b[1] + a[2] * b[0] + a[0] * b[2];
    auto[re0, q2] = mnd::pfu::quickTwoSum(p1_0, t2);
    auto[re1, re2] = mnd::pfu::quickTwoSum(q2, t3);
    return { re0, re1, re2 };
}


#endif // MANDEL_TRIPLEDOUBLE_H

