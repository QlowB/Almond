#ifndef MANDEL_FIXED128_H
#define MANDEL_FIXED128_H

#include <cinttypes>
#include <cmath>
#include <utility>
#include <array>
#include <vector>

struct Fixed128
{
    uint64_t upper;
    uint64_t lower;

    Fixed128(const Fixed128&) = default;
    ~Fixed128() = default;


    inline Fixed128(uint64_t upper, uint64_t lower) :
        upper{ upper }, lower{ lower }
    {
    }

    inline Fixed128(uint32_t a, uint32_t b, uint32_t c, uint32_t d) :
        upper{ (uint64_t(a) << 32) | b }, lower{ (uint64_t(c) << 32) | d }
    {
    }

    inline Fixed128(double x)
    {
        const double twoToThe32 = double(0x100000000ULL);
        upper = uint64_t(int64_t(x * twoToThe32));
        double remainder = x - double(upper) / twoToThe32;
        lower = uint64_t(int64_t(remainder * twoToThe32 * twoToThe32 * twoToThe32));
        /*int integerPart = ::floor(x);
        double fractionalPart = x - integerPart;
        upper = int64_t(integerPart) << 32;
        upper |= uint64_t(fractionalPart * (1ULL << 32)) & 0xFFFFFFFFULL;
        lower = 0;// uint64_t(fractionalPart * (1ULL << 32) * (1ULL << 63) * 2);*/
    }

    inline Fixed128 operator + (const Fixed128& other) const {
        uint64_t lowerAdded = lower + other.lower;
        uint64_t upperAdded = upper + other.upper + (lowerAdded < lower);
        return Fixed128{ upperAdded, lowerAdded };
    }
    
    inline Fixed128& operator +=(const Fixed128& other) {
        uint64_t lowerAdded = lower + other.lower;
        upper += other.upper + (lowerAdded < lower);
        lower = lowerAdded;
        return *this;
    }

    inline Fixed128 operator - (const Fixed128& other) const {
        uint64_t lowerSubbed = lower - other.lower;
        uint64_t upperSubbed = upper - other.upper - (lowerSubbed > lower);
        return Fixed128{ upperSubbed, lowerSubbed };
    }

    inline Fixed128 operator - (void) const {
        return this->operator~() + Fixed128{ 0, 0, 0, 1 };
    }

//private:
    static inline std::pair<uint64_t, uint64_t> mul64(int64_t a, int64_t b) {
        int32_t aa[2] = { int32_t(a >> 32), int32_t(a & 0xFFFFFFFF) };
        int32_t bb[2] = { int32_t(b >> 32), int32_t(b & 0xFFFFFFFF) };

        int32_t res[4];
        int64_t temp = int64_t(aa[1]) * bb[1];
        res[3] = temp & 0xFFFFFFFF;
        int32_t carry = temp >> 32;
        temp = int64_t(aa[0]) * bb[1] + int64_t(aa[1]) * bb[0] + carry;
        res[2] = temp & 0xFFFFFFFF;
        carry = temp >> 32;
        temp = int64_t(aa[0]) * bb[0] + carry;
        res[1] = temp & 0xFFFFFFFF;
        res[0] = temp >> 32;

        return std::make_pair(uint64_t((int64_t(res[0]) << 32) | res[1]), uint64_t((int64_t(res[2]) << 32) | res[3]));
    }

    static inline std::pair<uint64_t, uint64_t> mulu64(uint64_t a, uint64_t b) {
        uint32_t aa[2] = { uint32_t(a >> 32), uint32_t(a & 0xFFFFFFFF) };
        uint32_t bb[2] = { uint32_t(b >> 32), uint32_t(b & 0xFFFFFFFF) };

        uint32_t res[4];
        uint64_t temp = uint64_t(aa[1]) * bb[1];
        res[3] = temp & 0xFFFFFFFF;
        uint32_t carry = temp >> 32;
        temp = uint64_t(aa[0]) * bb[1] + uint64_t(aa[1]) * bb[0] + carry;
        res[2] = temp & 0xFFFFFFFF;
        carry = temp >> 32;
        temp = uint64_t(aa[0]) * bb[0] + carry;
        res[1] = temp & 0xFFFFFFFF;
        res[0] = temp >> 32;

        return std::make_pair((uint64_t(res[0]) << 32) | res[1], (uint64_t(res[2]) << 32) | res[3] );
    }

public:
    inline Fixed128 operator * (const Fixed128& other) const {
        if (this->operator!=(Fixed128{ 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF }) && isNegative()) {
            return -(other * this->operator-());
        }
        if (other.isNegative()) {
            return -((-other) * (*this));
        }
        auto [uuc, uu] = mulu64(upper, other.upper);
        auto [ulc, ul] = mulu64(upper, other.lower);
        auto [luc, lu] = mulu64(lower, other.upper);
        auto [llc, ll] = mulu64(lower, other.lower);

        uint64_t res[4] = { 0, 0, 0, 0 };
        res[3] = ll;
        res[2] += lu;
        res[2] += ul;
        if (res[2] < ul)
            res[1]++;
        res[2] += llc;
        if (res[2] < llc)
            res[1]++;
        res[1] += uu;
        if (res[1] < uu)
            res[0]++;
        res[1] += ulc;
        if (res[1] < ulc)
            res[0]++;
        res[1] += luc;
        if (res[1] < luc)
            res[0]++;
        res[0] += uuc;


        return Fixed128{ uint32_t(res[0] & 0xFFFFFFFF), uint32_t(int64_t(res[1]) >> 32), uint32_t(res[1] & 0xFFFFFFFF), uint32_t(int64_t(res[2]) >> 32) };

        /*if (isNegative()) {
            return -(this->operator-() * other);
        }
        if (other.isNegative()) {
            return -(*this * (-other));
        }

        bool otherNegative = other.isNegative();

        uint32_t quarters[4] = {
            (upper >> 32) & 0xFFFFFFFF,
            upper & 0xFFFFFFFF,
            (lower >> 32) & 0xFFFFFFFF,
            lower & 0xFFFFFFFF
        };

        auto [a, ra] = other.mul(quarters[0]);
        auto [b, rb] = other.mul(quarters[1]);
        auto [c, rc] = other.mul(quarters[2]);
        auto [d, rd] = other.mul(quarters[3]);
        b.arshift(1);
        c.arshift(2);
        d.arshift(3);
        Fixed128 carries = { uint32_t(rb), uint32_t(rc), uint32_t(rd), 0 };
        Fixed128 result = a + b + c + d + carries;
        return result;*/
    }

    inline std::pair<Fixed128, uint32_t> mul(uint32_t factor) const {
        uint32_t quarters[4] = {
            uint32_t(upper >> 32) & 0xFFFFFFFF,
            uint32_t(upper) & 0xFFFFFFFF,
            uint32_t(lower >> 32) & 0xFFFFFFFF,
            uint32_t(lower) & 0xFFFFFFFF
        };
        uint32_t newQ[4];
        uint32_t carry = 0;
        for (int i = 3; i >= 0; i--) {
            int64_t prod = int64_t(quarters[i]) * factor + carry;
            newQ[i] = prod & 0xFFFFFFFF;
            carry = prod >> 32;
        }
        /*    newQ[i] = quarters[i] * factor;
        uint64_t tempLower = newQ[3];
        uint64_t newLower = tempLower + (newQ[2] << 32);
        uint64_t newUpper = (newQ[2] >> 32) + newQ[1] + (newQ[0] << 32) + (newLower < tempLower ? 1 : 0);*/
        return std::make_pair(Fixed128{ newQ[0], newQ[1], newQ[2], newQ[3] }, carry);
    }

    /*
    inline void arshift(int fac32) {
        uint32_t temp = 0;
        switch (fac32) {
        case 0:
            return;
        case 1:
            temp = upper & 0xFFFFFFFF;
            upper = uint64_t(int64_t(upper) >> 32);
            lower >>= 32;
            lower |= uint64_t(temp) << 32;
        case 2:
            lower = upper;
            upper = uint64_t(int64_t(upper) >> 63);
        case 3:
            lower = uint64_t(int64_t(upper) >> 32);
            upper = uint64_t(int64_t(upper) >> 63);
        default:
            lower = uint64_t(int64_t(upper) >> 63);
            upper = uint64_t(int64_t(upper) >> 63);
        }
    }*/
    /*
    inline Fixed128 operator * (const Fixed128& other) const {
        int32_t quarters[4] = {
            (upper >> 32) & 0xFFFFFFFF,
            upper & 0xFFFFFFFF,
            (lower >> 32) & 0xFFFFFFFF,
            lower & 0xFFFFFFFF
        };

        int32_t otherQuarters[4] = {
            (other.upper >> 32) & 0xFFFFFFFF,
            other.upper & 0xFFFFFFFF,
            (other.lower >> 32) & 0xFFFFFFFF,
            other.lower & 0xFFFFFFFF
        };

        int64_t prods[4][4];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4 && j + i < 5; j++) {
                if (i == 0 || j == 0)
                    prods[i][j] = int64_t(quarters[i]) * int64_t(otherQuarters[j]);
                else
                    prods[i][j] = uint64_t(uint32_t(quarters[i])) * uint64_t(uint32_t(otherQuarters[j]));
            }
        }

        Fixed128 ret = { 0, 0 };
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4 && j + i < 5; j++) {
                if (i == 0 || j == 0)
                    ret.addSigned(prods[i][j], i + j);
                else
                    ret.add(prods[i][j], i + j);
            }
        }
        return ret;
        
        /*
        int64_t x00 = int64_t(quarters[0]) * int64_t(otherQuarters[0]);
        int64_t x01 = int64_t(quarters[0]) * int64_t(otherQuarters[1]);
        int64_t x02 = int64_t(quarters[0]) * int64_t(otherQuarters[2]);
        int64_t x03 = int64_t(quarters[0]) * int64_t(otherQuarters[3]);
        int64_t x10 = int64_t(quarters[1]) * int64_t(otherQuarters[0]);
        int64_t x11 = int64_t(quarters[1]) * int64_t(otherQuarters[1]);
        int64_t x12 = int64_t(quarters[1]) * int64_t(otherQuarters[2]);
        int64_t x13 = int64_t(quarters[1]) * int64_t(otherQuarters[3]);
        int64_t x20 = int64_t(quarters[2]) * int64_t(otherQuarters[0]);
        int64_t x21 = int64_t(quarters[2]) * int64_t(otherQuarters[1]);
        int64_t x22 = int64_t(quarters[2]) * int64_t(otherQuarters[2]);
        int64_t x30 = int64_t(quarters[3]) * int64_t(otherQuarters[0]);
        int64_t x31 = int64_t(quarters[3]) * int64_t(otherQuarters[1]);

        Fixed128 ret = { 0, 0 };
        /*uint32_t newQuarters[4] = {
            x00,
            x01 + x10,
            x02 + x11 + x20,
            x03 + x12 + x21 + x30,
        };*//*
        ret.add(x00, 0);
        ret.add(x01 + x10, 1);
        ret.add(x02 + x11 + x20, 2);
        ret.add(x03 + x12 + x21 + x30, 3);
        ret.add(x13 + x22 + x31, 4);

        return ret;*/
    /*}*/

private:
    inline void add(uint64_t val, int b32offset) {
        switch (b32offset) {
        case 0:
            upper += val << 32;
            return;
        case 1:
            upper += val;
            return;
        case 2:
            upper += val >> 32;
            lower += val << 32;
            return;
        case 3: {
            uint64_t newLower = lower + val;
            if (newLower < lower) upper++;
            lower = newLower;
            return;
        }
        case 4:
            uint64_t newLower = lower + (val >> 32);
            if (lower > newLower) upper++;
            lower += newLower;
            return;
        }
    }
    inline void addSigned(int64_t val, int b32offset) {
        switch (b32offset) {
        case 0:
            upper += val << 32;
            return;
        case 1:
            upper += val;
            return;
        case 2:
            upper += val >> 32;
            lower += val << 32;
            return;
        case 3:
            lower += val;
            if (val < 0) upper--;
            return;
        
        case 4: {
            uint64_t newLower = lower + (val >> 32);
            if (lower > newLower) upper++;
            lower = newLower;
            return;
        }
        default:
            if (val < 0) {
                if (lower == 0) upper--;
                lower--;
            }
            return;
        }
    }
public:

    inline Fixed128 operator / (const Fixed128& other) {
        if (this->operator!=(Fixed128{ 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF }) && isNegative()) {
            return -((-(*this)) / other);
        }
        if (other != Fixed128{ 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF } && other.isNegative()) {
            return -((*this) / (-other));
        }

        using u256 = std::array<uint64_t, 4>;

        u256 bigDividend = { upper, lower, 0, 0 };
        u256 bigDivisor = { 0, 0, other.upper, other.lower };

        auto twice = [] (u256& x) {
            bool carry = false;
            for (int i = 3; i >= 0; i--) {
                bool oldCarry = carry;
                carry = x[i] & 0x1000000000000000ULL;
                x[i] <<= 1;
                if (oldCarry) x[i] ++;
            }
        };
        
        auto geq = [] (const u256& a, const u256& b) -> bool {
            for (int i = 0; i < 4; i++) {
                if (a[i] > b[i])
                    return true;
                if (a[i] < b[i])
                    return false;
            }
            return true;
        };

        auto sub = [] (u256& a, const u256& b) -> bool {
            bool carry = false;
            for (int i = 3; i >= 0; i--) {
                uint64_t oldA = a[i];
                a[i] -= b[i];
                carry = oldA < a[i];
            }
            return carry;
        };
        
        auto add = [] (u256& a, const u256& b) -> bool {
            bool carry = false;
            for (int i = 3; i >= 0; i--) {
                uint64_t oldA = a[i];
                a[i] += b[i];
                carry = oldA > a[i];
            }
            return carry;
        };

        u256 growingCount = { 0, 0, 0, 1 };
        u256 quotient = { 0, 0, 0, 0 };

        std::vector<u256> growingStack = { bigDivisor };

        while (true) {
            u256 beforeSub = bigDividend;
            const u256& gr = growingStack[growingStack.size() - 1];
            if (!sub(bigDividend, gr)) {
                add(quotient, growingCount);
                u256 tw = gr; twice(tw);
                growingStack.push_back(tw);
            }
            else if (geq(bigDivisor, bigDividend)) {
                break;
            }
            else {
                bigDividend = beforeSub;
                growingStack.pop_back();
            }
        }

        return Fixed128{ quotient[2], quotient[3] };
    }

    bool isNegative(void) const {
        return upper >> 63;
    }

    operator double(void) const {
        const int64_t twoToThe32 = 0x100000000ULL;
        return double(int64_t(upper)) / twoToThe32 + int64_t(lower) / twoToThe32 / twoToThe32 / twoToThe32;
    }

    inline Fixed128 operator ~ (void) const {
        return Fixed128{ ~upper, ~lower };
    }

    inline bool operator == (const Fixed128& other) const {
        return upper == other.upper && lower == other.lower;
    }

    inline bool operator != (const Fixed128& other) const {
        return !operator==(other);
    }

    inline bool operator < (const Fixed128& other) const {
        return upper < other.upper || (upper == other.upper && lower < other.lower);
    }

    inline bool operator <= (const Fixed128& other) const {
        return operator<(other) || operator==(other);
    }

    inline bool operator > (const Fixed128& other) const {
        return upper > other.upper || (upper == other.upper && lower > other.lower);
    }

    inline bool operator >= (const Fixed128& other) const {
        return operator>(other) || operator==(other);
    }
};

struct Fixed64
{
    bool sign;
    uint64_t bits;

    Fixed64(const Fixed64&) = default;
    ~Fixed64() = default;


    inline Fixed64(uint64_t bits, bool /* dummy */) :
        bits{ bits }
    {
    }

    inline Fixed64(double x)
    {
        if (x < 0) {
            sign = true;
            x *= -1;
        }
        else {
            sign = false;
        }
        int integerPart = int(x);
        double fractionalPart = x - integerPart;
        bits = uint64_t(integerPart) << 32;
        bits |= uint64_t(fractionalPart * (1ULL << 32)) & 0xFFFFFFFF;
    }

    inline Fixed64 operator + (const Fixed64& other) {
        return Fixed64{ bits + other.bits, true };
    }
    
    inline Fixed64& operator +=(const Fixed64& other) {
        bits += other.bits;
        return *this;
    }

    inline Fixed64 operator - (const Fixed64& other) {
        return Fixed64{ bits - other.bits, true };
    }

    inline Fixed64 operator * (const Fixed64& other) {
        /*int32_t upper = bits >> 32;
        uint32_t lower = uint32_t(bits & 0xFFFFFFFF);
        int64_t upup = int64_t(upper) * int64_t(upper);
        int64_t loup = int64_t(upper) * int64_t(lower);
        int64_t lolo = int64_t(lower) * int64_t(lower);

        int32_t newUp = upup & 0xFFFFFFFF + (loup >> 32);
        int32_t newLo = loup & 0xFFFFFFFF + (lolo >> 32);*/
        double d = int32_t(bits >> 32) + double(uint32_t(bits)) / (1ULL << 32);
        double od = int32_t(other.bits >> 32) + double(uint32_t(other.bits)) / (1ULL << 32);
        return d * od * ((other.sign != sign) ? -1 : 1);

        //return Fixed64{ (uint64_t(newUp) << 32) | newLo, true };
    }

    inline bool operator == (const Fixed64& other) {
        return bits == other.bits;
    }

    inline bool operator != (const Fixed64& other) {
        return !operator==(other);
    }

    inline bool operator < (const Fixed64& other) {
        return bits < other.bits;
    }

    inline bool operator <= (const Fixed64& other) {
        return operator<(other) || operator==(other);
    }

    inline bool operator > (const Fixed64& other) {
        return bits > other.bits;
    }

    inline bool operator >= (const Fixed64& other) {
        return operator>(other) || operator==(other);
    }
};

struct Fixed32
{
    int32_t bits;

    Fixed32(const Fixed32&) = default;
    ~Fixed32() = default;


    inline Fixed32(int32_t bits, bool dummy) :
        bits{ bits }
    {
    }

    inline Fixed32(double x)
    {
        int integerPart = ::floor(x);
        double fractionalPart = x - integerPart;
        /*if (x < 0) {
            integerPart--;
            fractionalPart = 1.0 - fractionalPart;
        }*/
        bits = int32_t(integerPart) << 16;
        bits |= uint32_t(fractionalPart * (1ULL << 16)) & 0xFFFF;
    }

    inline Fixed32 operator + (const Fixed32& other) {
        return Fixed32{ bits + other.bits, true };
    }
    
    inline Fixed32& operator +=(const Fixed32& other) {
        bits += other.bits;
        return *this;
    }

    inline Fixed32 operator - (const Fixed32& other) {
        return Fixed32{ bits - other.bits, true };
    }

    inline Fixed32 operator * (const Fixed32& other) {
        int64_t prod = int64_t(bits) * int64_t(other.bits);
        return Fixed32{ int32_t(prod >> 16), true };
        //return Fixed32{ (uint64_t(newUp) << 32) | newLo, true };
    }

    inline bool operator == (const Fixed32& other) {
        return bits == other.bits;
    }

    inline bool operator != (const Fixed32& other) {
        return !operator==(other);
    }

    inline bool operator < (const Fixed32& other) {
        return bits < other.bits;
    }

    inline bool operator <= (const Fixed32& other) {
        return operator<(other) || operator==(other);
    }

    inline bool operator > (const Fixed32& other) {
        return bits > other.bits;
    }

    inline bool operator >= (const Fixed32& other) {
        return operator>(other) || operator==(other);
    }
};

#endif // MANDEL_FIXED128_H
