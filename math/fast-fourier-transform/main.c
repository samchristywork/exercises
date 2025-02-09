#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  double real;
  double imag;
} Complex;

void fft(Complex *X, int n) {
  if (n <= 1) {
    return;
  }

  Complex *even = (Complex *)malloc(n / 2 * sizeof(Complex));
  Complex *odd = (Complex *)malloc(n / 2 * sizeof(Complex));
  for (int i = 0; i < n / 2; i++) {
    even[i] = X[i * 2];
    odd[i] = X[i * 2 + 1];
  }

  fft(even, n / 2);
  fft(odd, n / 2);

  for (int k = 0; k < n / 2; k++) {
    double t = -2 * M_PI * k / n;
    Complex exp = {cos(t), sin(t)};
    Complex temp = {exp.real * odd[k].real - exp.imag * odd[k].imag,
                    exp.real * odd[k].imag + exp.imag * odd[k].real};

    X[k].real = even[k].real + temp.real;
    X[k].imag = even[k].imag + temp.imag;

    X[k + n / 2].real = even[k].real - temp.real;
    X[k + n / 2].imag = even[k].imag - temp.imag;
  }

  free(even);
  free(odd);
}
