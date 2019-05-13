#pragma once


template<typename T>
float iterate(const T& ca, const T& cb, int maxIterations)
{
    T a = ca;
    T b = cb;

    int n;
    for(n = 0; n < maxIterations; n++) {
        T aa = a * a;
        T bb = b * b;
        T abab = a * b; abab += abab;

        a = aa - bb + ca;
        b = abab + cb;
        if (aa + bb > T(16)) {
            break;
        }
    }

    return float(n);
}
