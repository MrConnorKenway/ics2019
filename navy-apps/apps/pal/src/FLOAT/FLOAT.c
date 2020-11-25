#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  long long A = a, B = b;
  return (A * B) >> 16;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  return (a / b) << 16;
}

typedef struct {
  unsigned fraction: 23;
  unsigned exponent: 8;
  int sign: 1;
} float_format;

FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */
  float_format *format = (float_format *) (&a);
  int biased_exponent = format->exponent - 127;
  int fraction = (1 << 23) | format->fraction;
  assert(7 - biased_exponent >= 0);
  FLOAT res = fraction >> (7 - biased_exponent);
  if (format->sign) {
    res = -res;
  }
  return res;
}

FLOAT Fabs(FLOAT a) {
  return (a >= 0) ? a : -a;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}
