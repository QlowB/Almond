#pragma OPENCL EXTENSION cl_khr_fp64 : enable

inline double2 twoSum(double a, double b) {
    double s = a + b;
    double bb = s - a;
    double e = (a - (s - bb)) + (b - bb);
    return (double2)(s, e);
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

inline double2 threeTwoSum(double a, double b, double c)
{
    double2 t = twoSum(a, b);
    double2 rt = twoSum(t.s0, c);
    return (double2)(rt.s0, t.s1 + rt.s1);
}

inline double3 mul(double3 a, double3 b) {
    double2 p1 = twoProd(a.s0, b.s0);
    double2 p2 = twoProd(a.s0, b.s1);
    double2 p3 = twoProd(a.s1, b.s0);

    double2 t12 = threeTwoSum(p1.s1, p2.s0, p3.s0);
    double t3 = t12.s1 + p2.s1 + p3.s1 + a.s2 * b.s0 + a.s0 * b.s2 + a.s1 * b.s1;
    double2 re0q = quickTwoSum(p1.s0, t12.s0);
    double2 rer = quickTwoSum(re0q.s1, t3);
    return (double3)(re0q.s0, rer.s0, rer.s1);
}

inline double3 sq(double3 a) {
    // TODO: improve
    return mul(a, a);
}

inline double3 add(double3 a, double3 b) {
    double2 rt = twoSum(a.s0, b.s0);
    double2 t12 = twoSum(a.s1, b.s1);
    double2 t34 = twoSum(rt.s1, t12.s0);
    double r2 = t12.s1 + t34.s1 + a.s2 + b.s2;

    double2 re1t4 = quickTwoSum(rt.s0, t34.s0);
    double2 re23 = twoSum(re1t4.s1, r2);
    return (double3)(re1t4.s0, re23.s0, re23.s1);
}

inline double3 mulDouble(double3 a, double b) {
    // TODO: improve
    return mul(a, (double3)(b, 0.0, 0.0));
}

__kernel void iterate(__global float* A, const int width,
                      double x1, double x2, double x3, double y1, double y2, double y3,
                      double pw1, double pw2, double pw3, double ph1, double ph2, double ph3, int max, int smooth,
                      int julia, double jx1, double jx2, double jx3, double jy1, double jy2, double jy3) {
    int index = get_global_id(0);
    int px = index % width;
    int py = index / width;

    double3 xl = (double3)(x1, x2, x3);
    double3 yt = (double3)(y1, y2, y3);
    double3 pixelScaleX = (double3)(pw1, pw2, pw3);
    double3 pixelScaleY = (double3)(ph1, ph2, ph3);
    double3 a = add(mulDouble(pixelScaleX, (double) px), xl); // pixelScaleX * px + xl
    double3 b = add(mulDouble(pixelScaleY, (double) py), yt); // pixelScaleY * py + yt
    double3 ca = julia != 0 ? ((double3) (jx1, jx2, jx3)) : a;
    double3 cb = julia != 0 ? ((double3) (jy1, jy2, jy3)) : b;


    int n = 0;
    while (n < max - 1) {
        double3 aa = mul(a, a);
        double3 bb = mul(b, b);
        double3 ab = mul(a, b);
        double3 minusbb = (double3)(-bb.s0, -bb.s1, -bb.s2);
        a = add(add(aa, minusbb), ca);
        b = add(add(ab, ab), cb);
        if (aa.s0 + bb.s0 > 16) break;
        n++;
    }

    // N + 1 - log (log  |Z(N)|) / log 2
    if (n >= max - 1)
        A[index] = max;
    else {
        if (smooth != 0)
            A[index] = ((float) n) + 1.0f - log2(log(a.s0 * a.s0 + b.s0 * b.s0) * 0.5f);
        else
            A[index] = ((float) n);
    }
    //               A[index] = ((float)n) + 1 - (a * a + b * b - 16) / (256 - 16);
    //           A[get_global_id(0)] = 5;
}