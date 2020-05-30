#ifndef MANDEL_POLYFLOATUTIL_H
#define MANDEL_POLYFLOATUTIL_H

namespace mnd
{
    namespace pfu
    {
        template<typename T>
        struct Pair
        {
            T a;
            T b;
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

        template<typename T>
        inline Pair<T> threeTwoSum(T a, T b, T c)
        {
            auto[t1, t2] = twoSum(a, b);
            auto[r0, t3] = twoSum(t1, c);
            return { r0, t2 + t3 };
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

        template<typename T>
        inline Pair<T> twoProdFma(T a, T b)
        {
            T p = a * b;
            //T e = std::fma(a, b, -p);
            T e = 0;
            return { p, e };
        }
    }
}

#endif // MANDEL_POLYFLOATUTIL_H

