#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#pragma OPENCL FP_CONTRACT OFF

inline double2 twoSum(double a, double b) {
    double s = a + b;
    double bb = s - a;
    double e = (a - (s - bb)) + (b - bb);
    return (double2)(s, e);
}

inline double two_sum(double a, double b, double* err) {
    double s = a + b;
    double bb = s - a;
    *err = (a - (s - bb)) + (b - bb);
    return s;
}

inline double2 quickTwoSum(double a, double b) {
    double s = a + b;
    double e = b - (s - a);
    return (double2)(s, e);
}

inline double quick_two_sum(double a, double b, double* err) {
  double s = a + b;
  *err = b - (s - a);
  return s;
}

inline double2 twoProd(double a, double b) {
    double p = a * b;
    double e = fma(a, b, -p);
    return (double2)(p, e);
}

inline double two_prod(double a, double b, double* err) {
    double p = a * b;
    *err = fma(a, b, -p);
    return p;
}

inline void threeSum(double* a, double* b, double* c) {
    double2 t = twoSum(*a, *b);
    double2 at3 = twoSum(*c, t.s0);
    double2 bc = twoSum(t.s1, at3.s1);
    *a = at3.s0;
    *b = bc.s0;
    *c = bc.s1;
}

inline void threeSum2(double* a, double* b, double* c) {
    double2 t = twoSum(*a, *b);
    double2 at3 = twoSum(*c, t.s0);
    *a = at3.s0;
    *b = t.s1 + at3.s1;
}

inline void renorm(double *c0, double *c1, 
                   double *c2, double *c3, double *c4) {
  double s0, s1, s2 = 0.0, s3 = 0.0;

  s0 = quick_two_sum(*c3, *c4, c4);
  s0 = quick_two_sum(*c2, s0, c3);
  s0 = quick_two_sum(*c1, s0, c2);
  *c0 = quick_two_sum(*c0, s0, c1);

  s0 = *c0;
  s1 = *c1;

  if (s1 != 0.0) {
    s1 = quick_two_sum(s1, *c2, &s2);
    if (s2 != 0.0) {
      s2 = quick_two_sum(s2, *c3, &s3);
      if (s3 != 0.0)
        s3 += *c4;
      else
        s2 = quick_two_sum(s2, *c4, &s3);
    } else {
      s1 = quick_two_sum(s1, *c3, &s2);
      if (s2 != 0.0)
        s2 = quick_two_sum(s2, *c4, &s3);
      else
        s1 = quick_two_sum(s1, *c4, &s2);
    }
  } else {
    s0 = quick_two_sum(s0, *c2, &s1);
    if (s1 != 0.0) {
      s1 = quick_two_sum(s1, *c3, &s2);
      if (s2 != 0.0)
        s2 = quick_two_sum(s2, *c4, &s3);
      else
        s1 = quick_two_sum(s1, *c4, &s2);
    } else {
      s0 = quick_two_sum(s0, *c3, &s1);
      if (s1 != 0.0)
        s1 = quick_two_sum(s1, *c4, &s2);
      else
        s0 = quick_two_sum(s0, *c4, &s1);
    }
  }

  *c0 = s0;
  *c1 = s1;
  *c2 = s2;
  *c3 = s3;
}


inline double4 add(double4 a, double4 b) {
    double s0, s1, s2, s3;
    double t0, t1, t2, t3;
    
    s0 = two_sum(a[0], b[0], &t0);
    s1 = two_sum(a[1], b[1], &t1);
    s2 = two_sum(a[2], b[2], &t2);
    s3 = two_sum(a[3], b[3], &t3);

    s1 = two_sum(s1, t0, &t0);
    threeSum(&s2, &t0, &t1);
    threeSum2(&s3, &t0, &t2);
    t0 = t0 + t1 + t3;

    renorm(&s0, &s1, &s2, &s3, &t0);
    return (double4)(s0, s1, s2, s3);
}


inline double4 mul(double4 a, double4 b) {
    double p0, p1, p2, p3, p4, p5;
    double q0, q1, q2, q3, q4, q5;
    double t0, t1;
    double s0, s1, s2;

    p0 = two_prod(a[0], b[0], &q0);

    p1 = two_prod(a[0], b[1], &q1);
    p2 = two_prod(a[1], b[0], &q2);

    p3 = two_prod(a[0], b[2], &q3);
    p4 = two_prod(a[1], b[1], &q4);
    p5 = two_prod(a[2], b[0], &q5);

    /* Start Accumulation */
    threeSum(&p1, &p2, &q0);

    /* Six-Three Sum  of p2, q1, q2, p3, p4, p5. */
    threeSum(&p2, &q1, &q2);
    threeSum(&p3, &p4, &p5);
    /* compute (s0, s1, s2) = (p2, q1, q2) + (p3, p4, p5). */
    s0 = two_sum(p2, p3, &t0);
    s1 = two_sum(q1, p4, &t1);
    s2 = q2 + p5;
    s1 = two_sum(s1, t0, &t0);
    s2 += (t0 + t1);

    /* O(eps^3) order terms */
    s1 += a[0]*b[3] + a[1]*b[2] + a[2]*b[1] + a[3]*b[0] + q0 + q3 + q4 + q5;
    renorm(&p0, &p1, &s0, &s1, &s2);
    return (double4)(p0, p1, s0, s1);
}

inline double4 sq(double4 a) {
    double p0, p1, p2, p3, p4, p5;
    double q0, q1, q2, q3;
    double s0, s1;
    double t0, t1;
    
    p0 = two_prod(a[0], a[0], &q0);
    p1 = two_prod(2.0 * a[0], a[1], &q1);
    p2 = two_prod(2.0 * a[0], a[2], &q2);
    p3 = two_prod(a[1], a[1], &q3);

    p1 = two_sum(q0, p1, &q0);

    q0 = two_sum(q0, q1, &q1);
    p2 = two_sum(p2, p3, &p3);

    s0 = two_sum(q0, p2, &t0);
    s1 = two_sum(q1, p3, &t1);

    s1 = two_sum(s1, t0, &t0);
    t0 += t1;

    s1 = quick_two_sum(s1, t0, &t0);
    p2 = quick_two_sum(s0, s1, &t1);
    p3 = quick_two_sum(t1, t0, &q0);

    p4 = 2.0 * a[0] * a[3];
    p5 = 2.0 * a[1] * a[2];

    p4 = two_sum(p4, p5, &p5);
    q2 = two_sum(q2, q3, &q3);

    t0 = two_sum(p4, q2, &t1);
    t1 = t1 + p5 + q3;

    p3 = two_sum(p3, t0, &p4);
    p4 = p4 + q0 + t1;

    renorm(&p0, &p1, &p2, &p3, &p4);
    return (double4)(p0, p1, p2, p3);
}


__kernel void iterate(__global float* A, const int width,
                      double x1, double x2, double x3, double x4, double y1, double y2, double y3, double y4,
                      double pw1, double pw2, double pw3, double pw4, double ph1, double ph2, double ph3, double ph4, int max, int smooth, int julia,
                      double jx1, double jx2, double jx3, double jx4, double jy1, double jy2, double jy3, double jy4) {
    int index = get_global_id(0);
    int px = index % width;
    int py = index / width;

    double4 xl = (double4)(x1, x2, x3, x4);
    double4 yt = (double4)(y1, y2, y3, y4);
    double4 pixelScaleX = (double4)(pw1, pw2, pw3, pw4);
    double4 pixelScaleY = (double4)(ph1, ph2, ph3, ph4);

    double4 c_re = add(mul(pixelScaleX, (double4) (px, 0, 0, 0)), xl); // pixelScaleX * px + xl
    double4 c_im = add(mul(pixelScaleY, (double4) (py, 0, 0, 0)), yt); // pixelScaleY * py + yt
