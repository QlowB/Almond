// citation: Guillaume da Graçca, David Defour. Implementation of float-float operators on graphics hardware.
// Real Numbers and Computers 7, Jul 2006, Nancy, France. pp.23-32. ffhal-00021443
// https://hal.archives-ouvertes.fr/hal-00021443/document

float2 twoSum(float a, float b) {
    float s = a + b;
    float v = s - a;
    float r = (a - (s - v)) + (b - v);
    return (float2)(s, r);
}

float2 split(float a) {
    float c = (4096 + 1) * a;
    float abig = c - a;
    float ahi = c - abig;
    float alo = a - ahi;
    return (float2)(ahi, alo);
}

float2 twoProd(float a, float b) {
    float x = a * b;
    float2 aex = split(a);
    float2 bex = split(b);
    float errx = x - (aex.s0 * bex.s0);
    float erry = errx - (aex.s1 * bex.s0);
    float errz = erry - (aex.s0 * bex.s1);
    float y = (aex.s1 * bex.s1) - errz;
    return (float2)(x, y);
}

float2 add(float2 a, float2 b) {
    float r = a.s0 + b.s0;
    float s;
    if (fabs(a.s0) >= fabs(b.s0)) {
        s = (((a.s0 - r) + b.s0) + b.s1) + a.s1;
    }
    else {
        s = (((b.s0 - r) + a.s0) + a.s1) + b.s1;
    }
    return twoSum(r, s);
}

float2 mul(float2 a, float2 b) {
    float2 t = twoProd(a.s0, b.s0);
    t.s1 += ((a.s0 * b.s1) + (a.s1 * b.s0));
    return twoSum(t.s0, t.s1);
}

float2 mulFloat(float2 a, float b) {
    float2 t = twoProd(a.s0, b);
    t.s1 += (a.s1 * b);
    return twoSum(t.s0, t.s1);
}

__kernel void iterate(__global float* A, const int width,
                      float x1, float x2, float y1, float y2,
                      float pw1, float pw2, float ph1, float ph2, int max, int smooth) {
    int index = get_global_id(0);
    int px = index % width;
    int py = index / width;

    float2 xl = (float2)(x1, x2);
    float2 yt = (float2)(y1, y2);
    float2 pixelScaleX = (float2)(pw1, pw2);
    float2 pixelScaleY = (float2)(ph1, ph2);

    float2 a = add(mulFloat(pixelScaleX, (float) px), xl); // pixelScaleX * px + xl
    float2 b = add(mulFloat(pixelScaleY, (float) py), yt); // pixelScaleY * py + yt
    float2 ca = a;
    float2 cb = b;

    int n = 0;
    while (n < max - 1) {
        float2 aa = mul(a, a);
        float2 bb = mul(b, b);
        float2 ab = mul(a, b);
        if (aa.s0 + bb.s0 > 16) break;
        float2 minusbb = (float2)(-bb.s0, -bb.s1);
        a = add(add(aa, minusbb), ca);
        b = add(add(ab, ab), cb);
        n++;
    }

    // N + 1 - log (log  |Z(N)|) / log 2
    if (n >= max - 1)
        A[index] = max;
    else {
        if (smooth != 0)
            A[index] = ((float) n) + 1 - log(log(a.s0 * a.s0 + b.s0 * b.s0) / 2) / log(2.0f);
        else
            A[index] = ((float)n);
    }
}