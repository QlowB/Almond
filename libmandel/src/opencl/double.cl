#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void iterate(__global float* A, const int width, double xl, double yt, double pixelScaleX, double pixelScaleY, int maxIter, int smooth) {
   int index = get_global_id(0);
   int x = index % width;
   int y = index / width;
   double a = x * pixelScaleX + xl;
   double b = y * pixelScaleY + yt;
   double ca = a;
   double cb = b;

   int n = 0;
   while (n < maxIter - 1) {
       double aa = a * a;
       double bb = b * b;
       double ab = a * b;
       if (aa + bb > 16) break;
       a = aa - bb + ca;
       b = ab + ab + cb;
       n++;
   }
// N + 1 - log (log  |Z(N)|) / log 2
   if (n >= maxIter - 1)
       A[index] = maxIter;
   else {
       if (smooth != 0)
           A[index] = ((float)n) + 1 - log(log((float)(a * a + b * b)) / 2) / log(2.0f);
       else
           A[index] = ((float)n);
   }
}

