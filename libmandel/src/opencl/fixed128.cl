

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


__kernel void iterate(__global float* A, const int width,
                      ulong x, ulong y, ulong pw, ulong ph, int max, int smooth) {
    int index = get_global_id(0);
    long px = (index % width);
    long py = (index / width);

    long xl = x;
    long yt = y;
    long pixelScaleX = pw;
    long pixelScaleY = ph;

    long a = xl + pixelScaleX * px; // pixelScaleX * px + xl
    long b = yt + pixelScaleY * py; // pixelScaleY * py + yt
    long ca = a;
    long cb = b;

    int n = 0;
    while (n < max - 1) {
        long aa = mul(a, a);
        long bb = mul(b, b);
        long ab = mul(a, b);
        if (aa + bb > (16LL << 48)) break;
        a = aa - bb + ca;
        b = ab + ab + cb;
        n++;
    }

    // N + 1 - log (log  |Z(N)|) / log 2
    if (n >= max - 1)
        A[index] = max;
    else {
        if (smooth != 0) {
            float aapprox = ((float) a) * (1.0f / (1LL << 48)); // 3.5527137e-15f;
            float bapprox = ((float) b) * (1.0f / (1LL << 48)); // 3.5527137e-15f;
            A[index] = ((float) n) + 1 - log(log(aapprox * aapprox + bapprox * bapprox) / 2) / log(2.0f);
        }
        else
            A[index] = ((float)n);
    }
}
