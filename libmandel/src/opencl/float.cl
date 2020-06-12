
__kernel void iterate(__global float* A, const int width, float xl, float yt, float pixelScaleX, float pixelScaleY, int maxIter, int smooth, int julia, float juliaX, float juliaY) {
   int index = get_global_id(0);
   int x = index % width;
   int y = index / width;
   float a = x * pixelScaleX + xl;
   float b = y * pixelScaleY + yt;
   float ca = julia != 0 ? juliaX : a;
   float cb = julia != 0 ? juliaY : b;

   int n = 0;
   while (n < maxIter - 1) {
       float aa = a * a;
       float bb = b * b;
       float ab = a * b;
       a = aa - bb + ca;
       b = ab + ab + cb;
       if (aa + bb > 16) break;
       n++;
   }
   if (n >= maxIter - 1) {
       A[index] = maxIter;
   }
   else {
       if (smooth != 0)
           A[index] = ((float)n) + 1 - log(log(a * a + b * b) / 2) / log(2.0f);
       else
           A[index] = ((float)n);
   }
}


__kernel void iterate2(__global float* A, const int width, float xl, float yt, float pixelScaleX, float pixelScaleY, int maxIter, int smooth, int julia, float juliaX, float juliaY) {
   int index = get_global_id(0);
   int index1 = index * 2;
   int index2 = index * 2 + 1;
   int x1 = index1 % width;
   int y1 = index1 / width;
   int x2 = index2 % width;
   int y2 = index2 / width;

   float a1 = x1 * pixelScaleX + xl;
   float b1 = y1 * pixelScaleY + yt;
   float ca1 = julia != 0 ? juliaX : a1;
   float cb1 = julia != 0 ? juliaY : b1;

   float a2 = x2 * pixelScaleX + xl;
   float b2 = y2 * pixelScaleY + yt;
   float ca2 = julia != 0 ? juliaX : a2;
   float cb2 = julia != 0 ? juliaY : b2;

   int n = 0;
   int n1 = 0;
   int n2 = 0;
   while (n < maxIter - 1) {
       float aa1 = a1 * a1;
       float aa2 = a2 * a2;
       float bb1 = b1 * b1;
       float bb2 = b2 * b2;
       float ab1 = a1 * b1;
       float ab2 = a2 * b2;

       if (aa1 + bb1 <= 16) {
           a1 = aa1 - bb1 + ca1;
           b1 = ab1 + ab1 + cb1;
           n1++;
       }
       if (aa2 + bb2 <= 16) {
           a2 = aa2 - bb2 + ca2;
           b2 = ab2 + ab2 + cb2;
           n2++;
       }
       if (aa1 + bb1 > 16 && aa2 + bb2 > 16) {
           break;
       }
       n++;
   }
   if (n1 >= maxIter - 1) {
       A[index1] = maxIter;
   }
   else {
       if (smooth != 0) {
           A[index1] = ((float)n1) + 1 - log2(log(a1 * a1 + b1 * b1) / 2);
       } else {
           A[index1] = ((float)n1);
       }
   }

   if (n2 >= maxIter - 1) {
       A[index2] = maxIter;
   }
   else {
       if (smooth != 0) {
           A[index2] = ((float)n2) + 1 - log2(log(a2 * a2 + b2 * b2) / 2);
       } else {
           A[index2] = ((float)n2);
       }
   }
}



__kernel void iterate_vec4(__global float* A, const int width, float xl, float yt, float pixelScaleX, float pixelScaleY, int maxIter, int smooth, int julia, float juliaX, float juliaY) {
   int index = get_global_id(0) * 4;
   int x = index % width;
   int y = index / width;
   float4 a = (float4) (x * pixelScaleX + xl, (x + 1) * pixelScaleX + xl, (x + 2) * pixelScaleX + xl, (x + 3) * pixelScaleX + xl);
   float4 b = (float4) (y * pixelScaleY + yt);
   float4 ca = julia ? ((float4)(juliaX)) : a;
   float4 cb = julia ? ((float4)(juliaY)) : b;
   float4 resa = (float4)(0);
   float4 resb = (float4)(0);
   float4 count = (float4)(0);

   int n = 0;
   if (smooth) {
       int4 cmp = isless((float4)(16.0f), (float4)(16.0f));
       while (n < maxIter) {
           float4 ab = a * b;
           float4 cmpVal = fma(a, a, b * b);
           a = fma(a, a, -fma(b, b, -ca));
           b = fma(2, ab, cb);
           resa = as_float4((as_int4(a) & cmp) | (as_int4(resa) & ~cmp));
           resb = as_float4((as_int4(b) & cmp) | (as_int4(resb) & ~cmp));
           cmp = isless(cmpVal, (float4)(16.0f));
           if (!any(cmp)) break;
           count += as_float4(cmp & as_int4((float4)(1)));
           n++;
       }
   }
    else {
       while (n < maxIter) {
           float4 ab = a * b;
           float4 cmpVal = fma(a, a, b * b);
           a = fma(a, a, -fma(b, b, -ca));
           b = fma(2, ab, cb);
           int4 cmp = isless(cmpVal, (float4)(16.0f));
           if (!any(cmp)) break;
           count += as_float4(cmp & as_int4((float4)(1)));
           n++;
       }
    }

    float4 res;
    if (smooth != 0) {
        if (count.s0 >= 0)
            res = ((float4) count) + ((float4)(1.0f, 1.0f, 1.0f, 1.0f)) - log2(log(fma(resa, resa, resb * resb)) / 2);
    }


    for (int i = 0; i < 4 && i + x < width; i++) {
    if (smooth != 0) {
        if (count[i] >= 0)
            A[index + i] = ((float) count[i]) + 1 - log(log(fma(resa[i], resa[i], resb[i] * resb[i])) / 2) / log(2.0f);
    }
    else
        A[index + i] = ((float) count[i]);
   }
}

