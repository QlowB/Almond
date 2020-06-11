#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#pragma OPENCL FP_CONTRACT OFF


typedef struct hex_double {
    double x[6];
} HexDouble;

inline double2 twoSum(double a, double b) {
    double s = a + b;
    double bb = s - a;
    double e = (a - (s - bb)) + (b - bb);
    return (double2)(s, e);
}

inline double two_sum(double a, double b, double* err) {
    double s = a + b;
    double bb = s - a;
    *err = (a - (s - bb)) + (b - bb);
    return s;
}

inline double2 quickTwoSum(double a, double b) {
    double s = a + b;
    double e = b - (s - a);
    return (double2)(s, e);
}


inline double2 twoProd(double a, double b) {
    double p = a * b;
    double e = fma(a, b, -p);
    return (double2)(p, e);
}


inline double3 threeSum(double a, double b, double c)
{
    double2 x = twoSum(a, b);
    double2 y = twoSum(x.s0, c);
    double2 z = twoSum(x.s1, y.s1);
    return (double3)(y.s0, z.s0, z.s1);
}


inline double4 fourSum(double a, double b, double c, double d)
{
    double2 x = twoSum(a, b);
    double2 y = twoSum(x.s0, c);
    double2 z = twoSum(y.s0, d);
    double3 r = threeSum(x.s1, y.s1, z.s1);
    return (double4)(z.s0, r.s0, r.s1, r.s2);
}


inline double2 fiveTwoSum(double a, double b, double c, double d, double e)
{
    double2 x = twoSum(a, b);
    double2 y = twoSum(x.s0, c);
    double2 z = twoSum(y.s0, d);
    double2 w = twoSum(z.s0, e);
    return (double2)(w.s0, x.s1 + y.s1 + z.s1 + w.s1);
}


inline double3 sixThreeSum(double a, double b, double c, double d, double e, double f)
{
    double3 av = threeSum(a, b, c);
    double3 bv = threeSum(d, e, f);
    
    double2 r1t1 = twoSum(av.s0, bv.s0);
    double2 t2t3 = twoSum(av.s1, bv.s1);
    double t4 = av.s2 + bv.s2;

    double2 r2t5 = twoSum(r1t1.s1, t2t3.s0);
    double r3 = t4 + t2t3.s1 + r2t5.s1;
    return (double3)(r1t1.s0, r2t5.s0, r3);
}


inline double3 nineThreeSum(double a, double b, double c, double d, double e,
                            double f, double g, double h, double i)
{
    double3 av = threeSum(a, b, c);
    double3 bv = threeSum(d, e, f);
    double3 cv = threeSum(g, h, i);
    
    double3 r1t1t2 = threeSum(av.s0, bv.s0, cv.s0);
    double4 r2t3t4t5 = fourSum(av.s1, bv.s1, cv.s1, r1t1t2.s1);
    double r3 = av.s2 + bv.s2 + cv.s2 + r1t1t2.s2 + r2t3t4t5.s1 + r2t3t4t5.s2 + r2t3t4t5.s3;
    return (double3)(r1t1t2.s0, r2t3t4t5.s0, r3);
}


inline HexDouble add(const __private HexDouble* a, const __private HexDouble* b)
{
    double2 ap = twoSum(a->x[0], b->x[0]);
    double2 bp = twoSum(a->x[1], b->x[1]);
    double2 cp = twoSum(a->x[2], b->x[2]);
    double2 dp = twoSum(a->x[3], b->x[3]);
    double2 ep = twoSum(a->x[4], b->x[4]);

    double t0 = ap.s0;
    double2 t1p1 = twoSum(ap.s1, bp.s0);
    double3 t2p2p3 = threeSum(bp.s1, cp.s0, t1p1.s1);
    double4 t3p456 = fourSum(cp.s1, dp.s0, t2p2p3.s1, t2p2p3.s2);
    double2 t4p7 = fiveTwoSum(dp.s1, ep.s0, t3p456.s1, t3p456.s2, t3p456.s3);
    double t5 = a->x[5] + b->x[5] + ep.s1 + t4p7.s1;

    double2 r0 = quickTwoSum(t0, t1p1.s0);
    double2 r1 = quickTwoSum(r0.s1, t2p2p3.s0);
    double2 r2 = quickTwoSum(r1.s1, t3p456.s0);
    double2 r3 = quickTwoSum(r2.s1, t4p7.s0);
    double2 r45 = quickTwoSum(r3.s1, t5);

    return (HexDouble){ r0.s0, r1.s0, r2.s0, r3.s0, r45.s0, r45.s1 };
}


inline HexDouble mul(const __private HexDouble* a, const __private HexDouble* b)
{
    double2 p0 = twoProd(a->x[0], b->x[0]);

    double2 q0 = twoProd(a->x[0], b->x[1]);
    double2 q1 = twoProd(a->x[1], b->x[0]);

    double2 r0 = twoProd(a->x[2], b->x[0]);
    double2 r1 = twoProd(a->x[1], b->x[1]);
    double2 r2 = twoProd(a->x[0], b->x[2]);

    double2 s0 = twoProd(a->x[3], b->x[0]);
    double2 s1 = twoProd(a->x[2], b->x[1]);
    double2 s2 = twoProd(a->x[1], b->x[2]);
    double2 s3 = twoProd(a->x[0], b->x[3]);

    double2 t0 = twoProd(a->x[4], b->x[0]);
    double2 t1 = twoProd(a->x[3], b->x[1]);
    double2 t2 = twoProd(a->x[2], b->x[2]);
    double2 t3 = twoProd(a->x[1], b->x[3]);
    double2 t4 = twoProd(a->x[0], b->x[4]);

    double v1 = p0.s0;
    double3 t2L34 = threeSum(p0.s1, q0.s0, q1.s0);
    double3 t3L45 = sixThreeSum(q0.s1, q1.s1, r0.s0, r1.s0, r2.s0, t2L34.s1);
    double3 t4L56 = nineThreeSum(r0.s1, r1.s1, r2.s1, s0.s0, s1.s0, s2.s0, s3.s0, t2L34.s2, t3L45.s1);
    double3 x123 = nineThreeSum(s0.s1, s1.s1, s2.s1, s3.s1, t0.s0, t1.s0, t2.s0, t3.s0, t4.s0);
    double3 t5L67 = sixThreeSum(x123.s0, x123.s1, x123.s2, t3L45.s2, t4L56.s1, 0.0);

    double v6 = t4L56.s2 + t5L67.s1 + t5L67.s2 + t0.s1 + t1.s1 + t2.s1 + t3.s1 + t4.s1 +
        a->x[5] * b->x[0] + a->x[4] * b->x[1] + a->x[3] * b->x[2] + a->x[2] * b->x[3] + a->x[1] * b->x[4] + a->x[0] * b->x[5];

    double2 re0 = quickTwoSum(v1, t2L34.s0);
    double2 re1 = quickTwoSum(re0.s1, t3L45.s0);
    double2 re2 = quickTwoSum(re1.s1, t4L56.s0);
    double2 re3 = quickTwoSum(re2.s1, t5L67.s0);
    double2 re45 = quickTwoSum(re3.s1, v6);
    return (HexDouble){ re0.s0, re1.s0, re2.s0, re3.s0, re45.s0, re45.s1 };
}


inline HexDouble twice(const __private HexDouble* a)
{
    return (HexDouble) {
        a->x[0] * 2,
        a->x[1] * 2,
        a->x[2] * 2,
        a->x[3] * 2,
        a->x[4] * 2,
        a->x[5] * 2,
    };
}


HexDouble loadHd(const __constant double* v) {
    return (HexDouble) {
        v[0],
        v[1],
        v[2],
        v[3],
        v[4],
        v[5],
    };
}


__kernel void iterate(__global float* A, const int width,
                      __constant double* x, __constant double* y,
                      __constant double* pw, __constant double* ph, int maxIter, int smooth, int julia,
                      __constant double* jx, __constant double* jy) {
    int index = get_global_id(0);
    int px = index % width;
    int py = index / width;

    HexDouble xl = loadHd(x);
    HexDouble yt = loadHd(y);
    HexDouble pixelScaleX = loadHd(pw);
    HexDouble pixelScaleY = loadHd(ph);

    HexDouble pxh = { px, 0, 0, 0, 0, 0 };
    HexDouble pyh = { py, 0, 0, 0, 0, 0 };

    HexDouble caOff = mul(&pixelScaleX, &pxh);
    HexDouble cbOff = mul(&pixelScaleY, &pyh);
    HexDouble ca = add(&caOff, &xl); // pixelScaleX * px + xl
    HexDouble cb = add(&cbOff, &yt); // pixelScaleY * py + yt
    HexDouble a = ca;
    HexDouble b = cb;
    if (julia != 0) {
        ca = loadHd(jx);
        cb = loadHd(jy);
    }

    int n = 0;
    while (n < maxIter - 1) {
        HexDouble aa = mul(&a, &a); /* TODO add squaring function */
        HexDouble bb = mul(&b, &b);
        HexDouble ab = mul(&a, &b);
        HexDouble minusbb = { -bb.x[0], -bb.x[1], -bb.x[2], -bb.x[3], -bb.x[4], -bb.x[5] };
        HexDouble aambb = add(&aa, &minusbb);
        HexDouble abab = twice(&ab);
        a = add(&aambb, &ca);
        b = add(&abab, &cb);
        if (aa.x[0] + bb.x[0] > 16) break;
        n++;
    }

    // N + 1 - log (log  |Z(N)|) / log 2
    if (n >= maxIter - 1)
        A[index] = maxIter;
    else {
        if (smooth != 0)
            A[index] = ((float) n) + 1 - log2(log(a.x[0] * a.x[0] + b.x[0] * b.x[0]) * 0.5);
        else
            A[index] = ((float)n);
    }
    //               A[index] = ((float)n) + 1 - (a * a + b * b - 16) / (256 - 16);
    //           A[get_global_id(0)] = 5;
}

