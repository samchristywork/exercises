#include <assert.h>
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

#ifdef TEST
int compareComplex(Complex a, Complex b, double tol) {
  return (fabs(a.real - b.real) < tol) && (fabs(a.imag - b.imag) < tol);
}

void test_fft() {
  const double epsilon = 1e-6;

  // Test case 1: Simple impulse signal
  {
    Complex input[] = {{1, 0}, {0, 0}, {0, 0}, {0, 0}};
    Complex expected[] = {{1, 0}, {1, 0}, {1, 0}, {1, 0}};

    fft(input, 4);
    for (int i = 0; i < 4; i++) {
      assert(compareComplex(input[i], expected[i], epsilon));
    }
  }

  // Test case 2: Constant signal
  {
    Complex input[] = {{1, 0}, {1, 0}, {1, 0}, {1, 0}};
    Complex expected[] = {{4, 0}, {0, 0}, {0, 0}, {0, 0}};

    fft(input, 4);
    for (int i = 0; i < 4; i++) {
      assert(compareComplex(input[i], expected[i], epsilon));
    }
  }

  // Test case 3: Different non-zero values
  {
    Complex input[] = {{1, 0}, {2, 0}, {3, 0}, {4, 0}};
    Complex expected[] = {{10, 0}, {-2, 2}, {-2, 0}, {-2, -2}};

    fft(input, 4);
    for (int i = 0; i < 4; i++) {
      assert(compareComplex(input[i], expected[i], epsilon));
    }
  }

  printf("All tests passed!\n");
}

int main() { test_fft(); }
#else
int main() {
}
#endif
