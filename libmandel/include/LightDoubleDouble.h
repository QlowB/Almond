#ifndef MANDEL_LIGHTDOUBLEDOUBLE_H
#define MANDEL_LIGHTDOUBLEDOUBLE_H

namespace mnd
{
    struct LightDoubleDouble;
    struct LightDoubleFloat;

    namespace ldd
    {
        template<typename T>
        struct Pair {
            T first;
            T second;
        };

        using DoublePair = Pair<double>;
        using FloatPair = Pair<float>;

        template<typename T>
        inline static Pair<T> quickTwoSum(T a, T b)
        {
            T s = a + b;
            T e = b - (s - a);
            return { s, e };
        }

        template<typename T>
        inline Pair<T> twoSum(T a, T b)
        {
            T s = a + b;
            T v = s - a;
            T e = (a - (s - v)) + (b - v);
            return { s, e };
        }

        inline Pair<double> split(double a)
        {
            static const double splitter = double((1ULL << 27) + 1);
            double t = splitter * a;
            double ahi = t - (t - a);
            double alo = a - ahi;
            return { ahi, alo };
        }

        template<typename T>
        inline Pair<T> twoProd(T a, T b)
        {
            T p = a * b;
            auto [ahi, alo] = split(a);
            auto [bhi, blo] = split(b);
            T e = ((ahi * bhi - p) + ahi * blo + alo * bhi) + alo * blo;
            return { p, e };
        }

        /*template<typename T>
        inline Pair<T> twoProdFma(T a, T b)
        {
            T p = a * b;
            T e = std::fma(a, b, -p);
            return { p, e };
        }*/
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


struct mnd::LightDoubleFloat
{
    float x[2];

    inline LightDoubleFloat(double val)
    {
        x[0] = float(val);
        x[1] = float(val - double(x[0]));
    }

    inline LightDoubleFloat(float u, float l) :
        x{ u, l }
    {}

    inline LightDoubleFloat(mnd::ldd::FloatPair dp) :
        x{ dp.first, dp.second }
    {}

    float operator[]           (int i) const   { return x[i]; }
    const float& operator[]    (int i)         { return x[i]; }

private:
};


inline mnd::LightDoubleFloat operator+(const mnd::LightDoubleFloat& a,
    const mnd::LightDoubleFloat& b)
{
    auto[s, e] = mnd::ldd::twoSum(a[0], b[0]);
    e += a[1] + b[1];
    return mnd::ldd::quickTwoSum(s, e);
}

inline mnd::LightDoubleFloat operator-(const mnd::LightDoubleFloat& a,
    const mnd::LightDoubleFloat& b)
{
    auto[s, e] = mnd::ldd::twoSum(a[0], -b[0]);
    e += a[1] - b[1];
    return mnd::ldd::quickTwoSum(s, e);
}

inline mnd::LightDoubleFloat operator*(const mnd::LightDoubleFloat& a,
    const mnd::LightDoubleFloat& b)
{
    auto[p1, p2] = mnd::ldd::twoProd(a[0], b[0]);
    p2 += a[0] * b[1] + a[1] * b[0];
    return mnd::ldd::quickTwoSum(p1, p2);
}

#endif // MANDEL_LIGHTDOUBLEDOUBLE_H
