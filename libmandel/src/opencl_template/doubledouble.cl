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

inline double2 mul(double2 a, double2 b) {
    double2 p = twoProd(a.s0, b.s0);
    p.s1 += (a.s0 * b.s1 + a.s1 * b.s0);
    return quickTwoSum(p.s0, p.s1);
}

inline double2 sq(double2 a) {
    double2 p = twoProd(a.s0, a.s0);
    double e = 2 * a.s0 * a.s1;
    p.s1 += e;
    return quickTwoSum(p.s0, p.s1);
}

inline double2 add(double2 a, double2 b) {
    double2 se = twoSum(a.s0, b.s0);
    se.s1 += a.s1 + b.s1;
    return quickTwoSum(se.s0, se.s1);
}

inline double2 mulDouble(double2 a, double b) {
    double2 p = twoProd(a.s0, b);
    p.s1 += a.s1 * b;
    return quickTwoSum(p.s0, p.s1);
}

__kernel void iterate(__global float* A, const int width,
                      double x1, double x2, double y1, double y2,
                      double pw1, double pw2, double ph1, double ph2, int max, int smooth,
                      int julia, double jx1, double jx2, double jy1, double jy2) {
    int index = get_global_id(0);
    int px = index % width;
    int py = index / width;

    double2 xl = (double2)(x1, x2);
    double2 yt = (double2)(y1, y2);
    double2 pixelScaleX = (double2)(pw1, pw2);
    double2 pixelScaleY = (double2)(ph1, ph2);
    double2 a = add(mulDouble(pixelScaleX, (double) px), xl); // pixelScaleX * px + xl
    double2 b = add(mulDouble(pixelScaleY, (double) py), yt); // pixelScaleY * py + yt
    double2 ca = julia != 0 ? ((double2) (jx1, jx2)) : a;
    double2 cb = julia != 0 ? ((double2) (jy1, jy2)) : b;


    int n = 0;
    while (n < max - 1) {
        double2 aa = mul(a, a);
        double2 bb = mul(b, b);
        double2 ab = mul(a, b);
        double2 minusbb = (double2)(-bb.s0, -bb.s1);
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
            A[index] = ((float) n) + 1 - log(log(a.s0 * a.s0 + b.s0 * b.s0) / 2) / log(2.0f);
        else
            A[index] = ((float) n);
    }
    //               A[index] = ((float)n) + 1 - (a * a + b * b - 16) / (256 - 16);
    //           A[get_global_id(0)] = 5;
}