__kernel void iterate(__global float* A, const int width, float xl, float yt, float pixelScaleX, float pixelScaleY, int max, int smooth) {
   int index = get_global_id(0);
   int x = index % width;
   int y = index / width;
   float a = x * pixelScaleX + xl;
   float b = y * pixelScaleY + yt;
   float ca = a;
   float cb = b;

   int n = 0;
   while (n < max - 1) {
       float aa = a * a;
       float bb = b * b;
       float ab = a * b;
       if (aa + bb > 16) break;
       a = aa - bb + ca;
       b = ab + ab + cb;
       n++;
   }
   if (n >= max - 1) {
       A[index] = max;
   }
   else {
       if (smooth != 0)
           A[index] = ((float)n) + 1 - log(log(a * a + b * b) / 2) / log(2.0f);
       else
           A[index] = ((float)n);
   }
}


__kernel void iterate_vec4(__global float* A, const int width, float xl, float yt, float pixelScaleX, float pixelScaleY, int max, int smooth) {
   int index = get_global_id(0) * 4;
   int x = index % width;
   int y = index / width;
   float4 a = (float4) (x * pixelScaleX + xl, (x + 1) * pixelScaleX + xl, (x + 2) * pixelScaleX + xl, (x + 3) * pixelScaleX + xl);
   float4 b = (float4) (y * pixelScaleY + yt);
   float4 ca = a;
   float4 cb = b;
   float4 resa = a;
   float4 resb = b;
   int4 count = (int4)(0);

   int n = 0;
   while (n < max) {
       float4 ab = a * b;
       float4 cmpVal = fma(a, a, b * b);
       int4 cmp = isless(cmpVal, (float4)(16.0f));
       if (!any(cmp)) break;
       a = fma(a, a, -fma(b, b, -ca));
       b = fma(2, ab, cb);
       if (smooth) {
           resa = as_float4(as_int4(a) & cmp | (as_int4(resa) & ~cmp));
           resb = as_float4(as_int4(b) & cmp | (as_int4(resb) & ~cmp));
       }
       count += cmp & (int4)(1);
       n++;
   }
   for (int i = 0; i < 4 && i + x < width; i++) {
       if (smooth != 0)
           A[index + i] = ((float) count[i]) + 1 - log(log(fma(resa[i], resa[i], resb[i] * resb[i])) / 2) / log(2.0f);
      else
          A[index + i] = ((float) count[i]);
   }
}


