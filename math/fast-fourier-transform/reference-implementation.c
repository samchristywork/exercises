#include <fftw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  // Capacity is hard-coded in the reference implementation
  fftw_complex signal[44100];
  fftw_complex result[44100];

  int n = 0;
  while (1) {
    if (scanf("%lf %lf", &signal[n][0], &signal[n][1]) != 2) {
      break;
    }
    n++;

    if (n >= 44100) {
      fprintf(stderr, "Error: too many samples\n");
      exit(EXIT_FAILURE);
    }
  }

  fftw_plan p =
      fftw_plan_dft_1d(n, signal, result, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(p);

  for (int i = 0; i < n; i++) {
    printf("%f %f\n", result[i][0], result[i][1]);
  }
}
