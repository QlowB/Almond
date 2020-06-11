#pragma OPENCL FP_CONTRACT OFF

inline float2 twoSum(float a, float b) {
    float s = a + b;
    float bb = s - a;
    float e = (a - (s - bb)) + (b - bb);
    return (float2)(s, e);
}

inline float2 quickTwoSum(float a, float b) {
    float s = a + b;
    float e = b - (s - a);
    return (float2)(s, e);
}

inline float2 twoProd(float a, float b) {
    float p = a * b;
    float e = fma(a, b, -p);
    return (float2)(p, e);
}

inline float2 threeTwoSum(float a, float b, float c)
{
    float2 t = twoSum(a, b);
    float2 rt = twoSum(t.s0, c);
    return (float2)(rt.s0, t.s1 + rt.s1);
}

inline float3 mul(float3 a, float3 b) {
    float2 p1 = twoProd(a.s0, b.s0);
    float2 p2 = twoProd(a.s0, b.s1);
    float2 p3 = twoProd(a.s1, b.s0);

    float2 t12 = threeTwoSum(p1.s1, p2.s0, p3.s0);
    float t3 = t12.s1 + p2.s1 + p3.s1 + a.s2 * b.s0 + a.s0 * b.s2 + a.s1 * b.s1;
    float2 re0q = quickTwoSum(p1.s0, t12.s0);
    float2 rer = quickTwoSum(re0q.s1, t3);
    return (float3)(re0q.s0, rer.s0, rer.s1);
}

inline float3 sq(float3 a) {
    // TODO: improve
    return mul(a, a);
}

inline float3 add(float3 a, float3 b) {
    float2 rt = twoSum(a.s0, b.s0);
    float2 t12 = twoSum(a.s1, b.s1);
    float2 t34 = twoSum(rt.s1, t12.s0);
    float r2 = t12.s1 + t34.s1 + a.s2 + b.s2;

    float2 re1t4 = quickTwoSum(rt.s0, t34.s0);
    float2 re23 = twoSum(re1t4.s1, r2);
    return (float3)(re1t4.s0, re23.s0, re23.s1);
}

inline float3 mulSingle(float3 a, float b) {
    // TODO: improve
    return mul(a, (float3)(b, 0.0, 0.0));
}

__kernel void iterate(__global float* A, const int width,
                      float x1, float x2, float x3, float y1, float y2, float y3,
                      float pw1, float pw2, float pw3, float ph1, float ph2, float ph3, int max, int smooth,
                      int julia, float jx1, float jx2, float jx3, float jy1, float jy2, float jy3) {
    int index = get_global_id(0);
    int px = index % width;
    int py = index / width;

    float3 xl = (float3)(x1, x2, x3);
    float3 yt = (float3)(y1, y2, y3);
    float3 pixelScaleX = (float3)(pw1, pw2, pw3);
    float3 pixelScaleY = (float3)(ph1, ph2, ph3);
    float3 a = add(mulSingle(pixelScaleX, (float) px), xl); // pixelScaleX * px + xl
    float3 b = add(mulSingle(pixelScaleY, (float) py), yt); // pixelScaleY * py + yt
    float3 ca = julia != 0 ? ((float3) (jx1, jx2, jx3)) : a;
    float3 cb = julia != 0 ? ((float3) (jy1, jy2, jy3)) : b;


    int n = 0;
    while (n < max - 1) {
        float3 aa = mul(a, a);
        float3 bb = mul(b, b);
        float3 ab = mul(a, b);
        float3 minusbb = (float3)(-bb.s0, -bb.s1, -bb.s2);
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
