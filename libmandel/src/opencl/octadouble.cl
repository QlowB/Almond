#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#pragma OPENCL FP_CONTRACT OFF


typedef double8 OctaDouble;

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


inline OctaDouble add(const __private OctaDouble* a, const __private OctaDouble* b)
{
    double2 ap = twoSum(a->s0, b->s0);
    double2 bp = twoSum(a->s1, b->s1);
    double2 cp = twoSum(a->s2, b->s2);
    double2 dp = twoSum(a->s3, b->s3);
    double2 ep = twoSum(a->s4, b->s4);
    double2 fp = twoSum(a->s5, b->s5);
    double2 gp = twoSum(a->s6, b->s6);

    double t0 = ap.s0;
    double2 t1p1 = twoSum(ap.s1, bp.s0);
    double3 t2p2p3 = threeSum(bp.s1, cp.s0, t1p1.s1);
    double4 t3p456 = fourSum(cp.s1, dp.s0, t2p2p3.s1, t2p2p3.s2);
    double3 t4p78 = sixThreeSum(dp.s1, ep.s0, t3p456.s1, t3p456.s2, t3p456.s3, 0.0);
    double3 t5p910 = sixThreeSum(ep.s1, fp.s0, t4p78.s1, t4p78.s2, 0.0, 0.0);
    double2 t6p11 = fiveTwoSum(fp.s1, gp.s0, t5p910.s1, t5p910.s2, 0.0);
    double t7 = a->s7 + b->s7 + gp.s1 + t6p11.s1;

    double2 r0 = quickTwoSum(t0, t1p1.s0);
    double2 r1 = quickTwoSum(r0.s1, t2p2p3.s0);
    double2 r2 = quickTwoSum(r1.s1, t3p456.s0);
    double2 r3 = quickTwoSum(r2.s1, t4p78.s0);
    double2 r4 = quickTwoSum(r3.s1, t5p910.s0);
    double2 r5 = quickTwoSum(r4.s1, t6p11.s0);
    double2 r67 = quickTwoSum(r5.s1, t7);

    return (OctaDouble)(r0.s0, r1.s0, r2.s0, r3.s0, r4.s0, r5.s0, r67.s0, r67.s1);
}


inline OctaDouble mul(const __private OctaDouble* a, const __private OctaDouble* b)
{
    double2 p0 = twoProd(a->s0, b->s0);

    double2 q0 = twoProd(a->s0, b->s1);
    double2 q1 = twoProd(a->s1, b->s0);

    double2 r0 = twoProd(a->s2, b->s0);
    double2 r1 = twoProd(a->s1, b->s1);
    double2 r2 = twoProd(a->s0, b->s2);

    double2 s0 = twoProd(a->s3, b->s0);
    double2 s1 = twoProd(a->s2, b->s1);
    double2 s2 = twoProd(a->s1, b->s2);
    double2 s3 = twoProd(a->s0, b->s3);

    double2 t0 = twoProd(a->s4, b->s0);
    double2 t1 = twoProd(a->s3, b->s1);
    double2 t2 = twoProd(a->s2, b->s2);
    double2 t3 = twoProd(a->s1, b->s3);
    double2 t4 = twoProd(a->s0, b->s4);

    double2 u0 = twoProd(a->s5, b->s0);
    double2 u1 = twoProd(a->s4, b->s1);
    double2 u2 = twoProd(a->s3, b->s2);
    double2 u3 = twoProd(a->s2, b->s3);
    double2 u4 = twoProd(a->s1, b->s4);
    double2 u5 = twoProd(a->s0, b->s5);

    double2 v0 = twoProd(a->s6, b->s0);
    double2 v1 = twoProd(a->s5, b->s1);
    double2 v2 = twoProd(a->s4, b->s2);
    double2 v3 = twoProd(a->s3, b->s3);
    double2 v4 = twoProd(a->s2, b->s4);
    double2 v5 = twoProd(a->s1, b->s5);
    double2 v6 = twoProd(a->s0, b->s6);

    double a1 = p0.s0;
    double3 a2t12 = threeSum(p0.s1, q0.s0, q1.s0);
    double3 a3t34 = sixThreeSum(q0.s1, q1.s1, r0.s0, r1.s0, r2.s0, a2t12.s1);
    double3 a4t56 = nineThreeSum(r0.s1, r1.s1, r2.s1, s0.s0, s1.s0, s2.s0, s3.s0, a3t34.s1, a2t12.s2);
    double3 x123 = nineThreeSum(s0.s1, s1.s1, s2.s1, s3.s1, t0.s0, t1.s0, t2.s0, t3.s0, t4.s0);
    double3 a5t78 = sixThreeSum(x123.s0, x123.s1, x123.s2, a4t56.s1, a3t34.s2, 0.0);
    double3 y123 = nineThreeSum(t0.s1, t1.s1, t2.s1, t3.s1, t4.s1, u0.s0, u1.s0, u2.s0, u3.s0);
    double3 a6t9a = nineThreeSum(y123.s0, y123.s1, y123.s2, u4.s0, u5.s0, a5t78.s1, a4t56.s2, 0.0, 0.0);
    double3 z123 = nineThreeSum(u0.s1, u1.s1, u2.s1, u3.s1, u4.s1, u5.s1, v0.s0, v1.s0, v2.s0);
    double3 a7tbc = nineThreeSum(v3.s0, v4.s0, v5.s0, v6.s0, z123.s0, z123.s1, z123.s2, a6t9a.s1, a6t9a.s2);
    double a8 = v0.s1 + v1.s1 + v2.s1 + v3.s1 + v4.s1 + v5.s1 + v6.s1 + a7tbc.s1 + a7tbc.s2 +
        a->s7 * b->s0 + 
        a->s6 * b->s1 + 
        a->s5 * b->s2 + 
        a->s4 * b->s3 + 
        a->s3 * b->s4 + 
        a->s2 * b->s5 + 
        a->s1 * b->s6 + 
        a->s0 * b->s7;


    double2 re0 = quickTwoSum(a1, a2t12.s0);
    double2 re1 = quickTwoSum(re0.s1, a3t34.s0);
    double2 re2 = quickTwoSum(re1.s1, a4t56.s0);
    double2 re3 = quickTwoSum(re2.s1, a5t78.s0);
    double2 re4 = quickTwoSum(re3.s1, a6t9a.s0);
    double2 re5 = quickTwoSum(re4.s1, a7tbc.s0);
    double2 re67 = quickTwoSum(re5.s1, a8);
    return (OctaDouble)(re0.s0, re1.s0, re2.s0, re3.s0, re4.s0, re5.s0, re67.s0, re67.s1);

    double r[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    for (int i = 0; i < 8; i++) {
        for (int j = 0; i + j < 8; j++) {
            double2 se = twoProd((*a)[i], (*b)[j]);
            for (int k = 0; k < 8; k++) {
                double3 x123 = threeSum(r[k], se.s0, se.s1);
                r[k] = x123.s0;
                se.s0 = x123.s1;
                se.s1 = x123.s2;
            }
        }
    }

    return (OctaDouble) (r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7]);
    /*double2 p0 = twoProd(a->s0, b->s0);

    double2 q0 = twoProd(a->s0, b->s1);
    double2 q1 = twoProd(a->s1, b->s0);

    double2 r0 = twoProd(a->s2, b->s0);
    double2 r1 = twoProd(a->s1, b->s1);
    double2 r2 = twoProd(a->s0, b->s2);

    double2 s0 = twoProd(a->s3, b->s0);
    double2 s1 = twoProd(a->s2, b->s1);
    double2 s2 = twoProd(a->s1, b->s2);
    double2 s3 = twoProd(a->s0, b->s3);

    double2 t0 = twoProd(a->s4, b->s0);
    double2 t1 = twoProd(a->s3, b->s1);
    double2 t2 = twoProd(a->s2, b->s2);
    double2 t3 = twoProd(a->s1, b->s3);
    double2 t4 = twoProd(a->s0, b->s4);

    double2 u0 = twoProd(a->s5, b->s0);
    double2 u1 = twoProd(a->s4, b->s1);
    double2 u2 = twoProd(a->s3, b->s2);
    double2 u3 = twoProd(a->s2, b->s3);
    double2 u4 = twoProd(a->s1, b->s4);
    double2 u5 = twoProd(a->s0, b->s5);

    double v1 = p0.s0;
    double3 t2L34 = threeSum(p0.s1, q0.s0, q1.s0);
    double3 t3L45 = sixThreeSum(q0.s1, q1.s1, r0.s0, r1.s0, r2.s0, t2L34.s1);
    double3 t4L56 = nineThreeSum(r0.s1, r1.s1, r2.s1, s0.s0, s1.s0, s2.s0, s3.s0, t2L34.s2, t3L45.s1);
    double3 x123 = nineThreeSum(s0.s1, s1.s1, s2.s1, s3.s1, t0.s0, t1.s0, t2.s0, t3.s0, t4.s0);
    double3 t5L67 = sixThreeSum(x123.s0, x123.s1, x123.s2, t3L45.s2, t4L56.s1, t4L56.s2);
    double3 x456 = nineThreeSum(t0.s1, t1.s1, t2.s1, t3.s1, t4.s1, u0.s0, u1.s0, u2.s0, u3.s0);
    double3 t6L89 = nineThreeSum(x456.s0, x456.s1, x456.s2, t5L67.s1, t5L67.s2, u4.s0, u5.s0, 0.0, 0.0);

    double v7 = t6L89.s1 + t6L89.s2 + 
        a->s6 * b->s0 + a->s5 * b->s1 + a->s4 * b->s2 + a->s3 * b->s3 + a->s2 * b->s4 + a->s1 * b->s5 + a->s0 * b->s6;

    double2 re0 = quickTwoSum(v1, t2L34.s0);
    double2 re1 = quickTwoSum(re0.s1, t3L45.s0);
    double2 re2 = quickTwoSum(re1.s1, t4L56.s0);
    double2 re3 = quickTwoSum(re2.s1, t5L67.s0);
    double2 re4 = quickTwoSum(re3.s1, t6L89.s0);
    double2 re56 = quickTwoSum(re4.s1, v7);
    return (OctaDouble)(re0.s0, re1.s0, re2.s0, re3.s0, re4.s0, re56.s0, re56.s1, 0.0);*/
}


inline OctaDouble twice(const __private OctaDouble* a)
{
    return (OctaDouble) (
        a->s0 * 2,
        a->s1 * 2,
        a->s2 * 2,
        a->s3 * 2,
        a->s4 * 2,
        a->s5 * 2,
        a->s6 * 2,
        a->s7 * 2
    );
}


OctaDouble loadHd(const __constant double* v) {
    return (OctaDouble) (
        v[0],
        v[1],
        v[2],
        v[3],
        v[4],
        v[5],
        v[6],
        v[7]
    );
}


__kernel void iterate(__global float* A, const int width,
                      __constant double* x, __constant double* y,
                      __constant double* pw, __constant double* ph, int maxIter, int smooth, int julia,
                      __constant double* jx, __constant double* jy) {
    int index = get_global_id(0);
    int px = index % width;
    int py = index / width;

    OctaDouble xl = loadHd(x);
    OctaDouble yt = loadHd(y);
    OctaDouble pixelScaleX = loadHd(pw);
    OctaDouble pixelScaleY = loadHd(ph);

    OctaDouble pxh = { px, 0, 0, 0, 0, 0, 0, 0 };
    OctaDouble pyh = { py, 0, 0, 0, 0, 0, 0, 0 };

    OctaDouble caOff = mul(&pixelScaleX, &pxh);
    OctaDouble cbOff = mul(&pixelScaleY, &pyh);
    OctaDouble ca = add(&caOff, &xl); // pixelScaleX * px + xl
    OctaDouble cb = add(&cbOff, &yt); // pixelScaleY * py + yt
    OctaDouble a = ca;
    OctaDouble b = cb;
    if (julia != 0) {
        ca = loadHd(jx);
        cb = loadHd(jy);
    }

    int n = 0;
    while (n < maxIter - 1) {
        OctaDouble aa = mul(&a, &a); /* TODO add squaring function */
        OctaDouble bb = mul(&b, &b);
        OctaDouble ab = mul(&a, &b);
        OctaDouble minusbb = (OctaDouble)(-bb.s0, -bb.s1, -bb.s2, -bb.s3, -bb.s4, -bb.s5, -bb.s6, -bb.s7);
        OctaDouble aambb = add(&aa, &minusbb);
        OctaDouble abab = twice(&ab);
        a = add(&aambb, &ca);
        b = add(&abab, &cb);
        if (aa.s0 + bb.s0 > 16) break;
        n++;
    }

    // N + 1 - log (log  |Z(N)|) / log 2
    if (n >= maxIter - 1)
        A[index] = maxIter;
    else {
        if (smooth != 0)
            A[index] = ((float) n) + 1 - log2(log(a.s0 * a.s0 + b.s0 * b.s0) * 0.5);
        else
            A[index] = ((float)n);
    }
    //               A[index] = ((float)n) + 1 - (a * a + b * b - 16) / (256 - 16);
    //           A[get_global_id(0)] = 5;
}

