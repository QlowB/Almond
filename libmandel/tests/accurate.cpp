#include "../include/TripleDouble.h"
#include "../include/Types.h"

#include <vector>
#include <boost/random.hpp>

const int nTests = 1000;

using namespace boost::multiprecision;
using namespace boost::random;

std::vector<mnd::Real> generateRandom(int len, int seed)
{
    uniform_real_distribution<mnd::Real> dist(0, 1);
    uniform_int_distribution<int> expDist(-30, 30);
    independent_bits_engine<mt19937, std::numeric_limits<cpp_bin_float_50>::digits, cpp_int> gen(seed);

    std::vector<mnd::Real> numbers;
    for (int i = 0; i < len; i++) {
        mnd::Real m = dist(gen);
        int e = expDist(gen);
        numbers.push_back(m * pow(mnd::Real(2), e));
    }
    return numbers;
}


template<typename T, typename Binary>
mnd::Real maxErr(Binary func)
{
    mnd::Real maxRelErr = 0.0;
    auto a = generateRandom(nTests, 123);
    auto b = generateRandom(nTests, 456);
    for (int i = 0; i < nTests; i++) {
        T v1 = mnd::convert<T>(a[i]);
        T v2 = mnd::convert<T>(b[i]);

        mnd::Real r1 = mnd::convert<mnd::Real>(v1);
        mnd::Real r2 = mnd::convert<mnd::Real>(v2);
        //std::cout << r1 << " --- " << r2 << std::endl;

        T res = func(v1, v2);
        mnd::Real corrRes = func(r1, r2);
        mnd::Real relErr = abs(corrRes - mnd::convert<mnd::Real>(res)) / corrRes;
        if (relErr > maxRelErr)
            maxRelErr = relErr;
    }
    return maxRelErr;
}



int main()
{
    mnd::Real doubleDoubleAdd = maxErr<mnd::LightDoubleDouble>([] (const auto& a, const auto& b) { return a + b; });
    mnd::Real doubleDoubleMul = maxErr<mnd::LightDoubleDouble>([] (const auto& a, const auto& b) { return a * b; });
    std::cout << "max double double add error: " << doubleDoubleAdd << std::endl;
    std::cout << "max double double mul error: " << doubleDoubleMul << std::endl;

    mnd::Real tripleDoubleAdd = maxErr<mnd::TripleDouble>([] (const auto& a, const auto& b) { return a + b; });
    mnd::Real tripleDoubleMul = maxErr<mnd::TripleDouble>([] (const auto& a, const auto& b) { return a * b; });
    std::cout << std::setprecision(10) << std::scientific;
    std::cout << "max triple double add error: " << tripleDoubleAdd << std::endl;
    std::cout << "max triple double mul error: " << tripleDoubleMul << std::endl;


}


