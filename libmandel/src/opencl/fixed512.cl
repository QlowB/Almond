

/*uint16 add(uint16 a, uint16 b) {
    uint mask = 0x00FFFFFF;
}*/


ulong2 add(ulong2 a, ulong2 b) {
    ulong2 result;
    uchar carry = 0;
    for (int i = 1; i >= 0; i--) {
        uchar tmpcr = carry;
        ulong temp = a[i] + b[i];
        carry = (hadd(a[i], b[i]) >> 63) | (hadd(temp, (ulong) carry) >> 63);
        temp += tmpcr;
        result[i] = temp;
    }
    return result;
}

ulong2 sub(ulong2 a, ulong2 b) {
    ulong lowerSubbed = a[1] - b[1];
    ulong upperSubbed = a[0] - b[1] - ((lowerSubbed > a[1]) ? 1 : 0);
    return (ulong2)(upperSubbed, lowerSubbed);
}

/*
ulong4 mulPart(ulong2 a, ulong b) {
    ulong4 result;
    ulong carry = 0;
    for (int i = 1; i >= 0; i--) {
        ulong lower = a[i] * b[i];
        ulong upper = mul_hi(a[i], b[i]);
        ulong c = lower + carry;
        ulong cc = hadd(lower, carry) >> 63;
        carry = upper + cc;
        result[i] = c;
    }
}*/

ulong2 mulu64(ulong a, ulong b) {
    return (ulong2)(mul_hi(a, b), a * b);
}

ulong2 mul(ulong2 a, ulong2 b) {
    //uint4 avec = (uint4)(a[0] >> 32, a[0], a[1] >> 32, a[1]);
    //uint4 bvec = (uint4)(b[0] >> 32, b[0], b[1] >> 32, b[1]);

    ulong2 uu = mulu64(a[0], b[0]);
    ulong2 ul = mulu64(a[0], b[1]);
    ulong2 lu = mulu64(a[1], b[0]);
    ulong2 ll = mulu64(a[1], b[1]);

    ulong4 res = (ulong4)(0, 0, 0, 0);
    res[3] = ll[1];
    res[2] += lu[1];
    res[2] += ul[1];
    if (res[2] < ul[1])
        res[1]++;
    res[2] += ll[0];
    if (res[2] < ll[0])
        res[1]++;
    res[1] += uu[1];
    if (res[1] < uu[1])
        res[0]++;
    res[1] += ul[0];
    if (res[1] < ul[0])
        res[0]++;
    res[1] += lu[0];
    if (res[1] < lu[0])
        res[0]++;
    res[0] += uu[0];

    uint4 num = (uint4) (res[0] & 0xFFFFFFFF, ((long) res[1]) >> 32, res[1] & 0xFFFFFFFF, ((long) res[2]) >> 32);
    return (ulong2)((num[0] << 32) + num[1], (num[2] << 32) + num[3]);
}


__kernel void iterate(__global float* A, const int width,
                      ulong x1, ulong x2, ulong y1, ulong y2,
                      ulong pw1, ulong pw2, ulong ph1, ulong ph2, int max, int smooth) {
    int index = get_global_id(0);
    ulong2 px = (ulong2) ((index % width) << 32, 0);
    ulong2 py = (ulong2) ((index / width) << 32, 0);

    ulong2 xl = (ulong2)(x1, x2);
    ulong2 yt = (ulong2)(y1, y2);
    ulong2 pixelScaleX = (ulong2)(pw1, pw2);
    ulong2 pixelScaleY = (ulong2)(ph1, ph2);

    ulong2 a = add(mul(pixelScaleX, px), xl); // pixelScaleX * px + xl
    ulong2 b = add(mul(pixelScaleY, py), yt); // pixelScaleY * py + yt
    ulong2 ca = a;
    ulong2 cb = b;

    int n = 0;
    while (n < max - 1) {
        ulong2 aa = mul(a, a);
        ulong2 bb = mul(b, b);
        ulong2 ab = mul(a, b);
        if (aa.s0 + aa.s1 + bb.s0 + bb.s1 > 16) break;
        a = add(sub(aa, bb), ca);
        b = add(add(ab, ab), cb);
        n++;
    }

    // N + 1 - log (log  |Z(N)|) / log 2
    if (n >= max - 1)
        A[index] = max;
    else {
        if (smooth != 0) {
            float aapprox = (float) a.s0 * 2.3283064e-10f;
            float bapprox = (float) b.s0 * 2.3283064e-10f;
            A[index] = ((float) n) + 1 - log(log(aapprox * aapprox + bapprox * bapprox) / 2) / log(2.0f);
        }
        else
            A[index] = ((float)n);
    }
    //               A[index] = ((float)n) + 1 - (a * a + b * b - 16) / (256 - 16);
    //           A[get_global_id(0)] = 5;
}
