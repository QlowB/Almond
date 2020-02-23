

long mul(long a, long b) {
    long upper = mul_hi(a, b);
    long lower = a * b;
    return (upper << 32) + ((lower >> 32) & 0xFFFFFFFF);
}


__kernel void iterate(__global float* A, const int width,
                      ulong x, ulong y, ulong pw, ulong ph, int max, int smooth) {
    int index = get_global_id(0);
    long px = (index % width) << 48;
    long py = (index / width) << 48;

    long xl = x;
    long yt = y;
    long pixelScaleX = pw;
    long pixelScaleY = ph;

    long a = (mul(pixelScaleX, px) + xl); // pixelScaleX * px + xl
    long b = (mul(pixelScaleY, py) + yt); // pixelScaleY * py + yt
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
            float aapprox = ((float) a) * 3.5527137e-15f;
            float bapprox = ((float) b) * 3.5527137e-15f;
            A[index] = ((float) n) + 1 - log(log(aapprox * aapprox + bapprox * bapprox) / 2) / log(2.0f);
        }
        else
            A[index] = ((float)n);
    }
    //               A[index] = ((float)n) + 1 - (a * a + b * b - 16) / (256 - 16);
    //           A[get_global_id(0)] = 5;
}
