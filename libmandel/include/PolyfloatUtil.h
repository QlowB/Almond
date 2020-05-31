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

        template<typename T>
        struct Triple
        {
            T a;
            T b;
            T c;
        };

        template<typename T>
        struct Quadruple
        {
            T a;
            T b;
            T c;
            T d;
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
        inline Triple<T> threeSum(T a, T b, T c)
        {
            auto[t1, t2] = twoSum(a, b);
            auto[r0, t3] = twoSum(t1, c);
            auto[r1, r2] = twoSum(t2, t3);
            return { r0, r1, r2 };
        }

        template<typename T>
        inline Pair<T> threeTwoSum(T a, T b, T c)
        {
            auto[t1, t2] = twoSum(a, b);
            auto[r0, t3] = twoSum(t1, c);
            return { r0, t2 + t3 };
        }

        template<typename T>
        inline Quadruple<T> fourSum(T a, T b, T c, T d)
        {
            auto[t1, t2] = twoSum(a, b);
            auto[t3, t4] = twoSum(t1, c);
            auto[r0, t5] = twoSum(t3, d);
            auto[r1, r2, r3] = threeSum(t2, t4, t5);
            return { r0, r1, r2, r3 };
        }

        template<typename T>
        inline Pair<T> fiveTwoSum(T a, T b, T c, T d, T e)
        {
            auto[t1, t2] = twoSum(a, b);
            auto[t3, t4] = twoSum(t1, c);
            auto[t5, t6] = twoSum(t3, d);
            auto[r0, t7] = twoSum(t5, e);
            return { r0, t2 + t4 + t6 + t7 };
        }

        template<typename T>
        inline Triple<T> sixThreeSum(T a, T b, T c, T d, T e, T f)
        {
            auto[a1, a2, a3] = threeSum(a, b, c);
            auto[b1, b2, b3] = threeSum(d, e, f);
            
            auto[r1, t1] = twoSum(a1, b1);
            auto[t2, t3] = twoSum(a2, b2);
            auto t4 = a3 + b3;

            auto[r2, t5] = twoSum(t1, t2);
            auto r3 = t4 + t3 + t5;
            return { r1, r2, r3 };
        }

        template<typename T>
        inline Triple<T> nineThreeSum(T a, T b, T c, T d, T e, T f, T g, T h, T i)
        {
            auto[a1, a2, a3] = threeSum(a, b, c);
            auto[b1, b2, b3] = threeSum(d, e, f);
            auto[c1, c2, c3] = threeSum(g, h, i);
            
            auto[r1, t1, t2] = threeSum(a1, b1, c1);
            auto[r2, t3, t4, t5] = fourSum(a2, b2, c2, t1);
            auto r3 = a3 + b3 + c3 + t2 + t3 + t4 + t5;
            return { r1, r2, r3 };
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

