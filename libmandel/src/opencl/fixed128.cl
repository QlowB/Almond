ulong2 add(ulong2 a, ulong2 b) {
    ulong lower = a[1] + b[1];
    ulong upper = a[0] + b[0];
    if (lower < a[1])
        upper += 1;
    return (ulong2)(upper, lower);
}

ulong2 sub(ulong2 a, ulong2 b) {
    ulong lower = a[1] - b[1];
    ulong upper = a[0] - b[0];
    if (lower > a[1])
        upper += 1;
    return (ulong2)(upper, lower);
}

long2 mul(long2 a, long2 b) {
    long a0 = mul_hi(a[0], b[0]);
    long b0 = a[0] * b[0];
    long b1 = mul_hi(a[0], b[1]);
    long b2 = mul_hi(a[1], b[0]);
    long c0 = a[1] * b[0];
    long c1 = a[0] * b[1];
    long c2 = mul_hi(a[1], b[1]);
    long c3 = mul_hi(a[1], b[1]);

    long carry = 0;
    long r1 = b0 + b1;
    if (r1 < b0)
        carry++;
    long r2 = r1 + b2;
    if (r2 < r1)
        carry++;

    a0 += carry;

    long newUpper = (a0 << 16) + ((r2 >> 48) & 0xFFFF);
}

long2 mulInteger(long2 a, long b) {
    long lo = a[1] * b;
    long carry = mul_hi(a[1], b);
    long hi = mad(a[0], b, carry);
    return (long2)(hi, lo);
}


__kernel void iterate(__global float* A, const int width,
                      ulong x1, ulong x2, ulong y1, ulong y2,
                      ulong pw1, ulong pw2, ulong ph1 ulong ph2,
                      int max, int smooth, int julia,
                      ulong jx1, ulong jx2, ulong jy1, ulong jy2) {
    int index = get_global_id(0);
    int px = (index % width);
    int py = (index / width);

    long2 xl = (long2)(x1, x2);
    long2 yt = (long2)(y1, y2);
    long2 pixelScaleX = (long2)(pw1, pw2);
    long2 pixelScaleY = (long2)(ph1, ph2);

    long2 a = add(xl, mulInteger(pixelScaleX, px)); // pixelScaleX * px + xl
    long2 b = add(yt, mulInteger(pixelScaleY, py))y; // pixelScaleY * py + yt
    long2 ca = a;
    long2 cb = b;
    if(julia != 0) {
        a = (long2)(jx1, jx2);
        b = (long2)(jy1, jy2);
    }

    int n = 0;
    while (n < max - 1) {
        long2 aa = mul(a, a);
        long2 bb = mul(b, b);
        long2 ab = mul(a, b);
        a = add(sub(aa, bb), ca);
        b = add(add(ab, ab), cb);
        if (aa[0] + bb[0] > (16LL << 48)) break;
        n++;
    }

    // N + 1 - log (log  |Z(N)|) / log 2
    if (n >= max - 1)
        A[index] = max;
    else {
        if (smooth != 0) {
            float aapprox = ((float) a[0]) * (1.0f / (1LL << 48)); // 3.5527137e-15f;
            float bapprox = ((float) b[0]) * (1.0f / (1LL << 48)); // 3.5527137e-15f;
            A[index] = ((float) n) + 1 - log(log(aapprox * aapprox + bapprox * bapprox) / 2) / log(2.0f);
        }
        else
            A[index] = ((float)n);
    }
}