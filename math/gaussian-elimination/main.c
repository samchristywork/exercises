#include <math.h>
#include <stdio.h>

void gaussianElimination(double coefficients[3][3], double constants[3],
                         double solution[3], int n) {
  // Forward elimination
  for (int i = 0; i < n; i++) {
    // Find the maximum element for pivot
    for (int k = i + 1; k < n; k++) {
      if (fabs(coefficients[k][i]) > fabs(coefficients[i][i])) {
        // Swap the rows
        for (int j = 0; j < n; j++) {
          double temp = coefficients[i][j];
          coefficients[i][j] = coefficients[k][j];
          coefficients[k][j] = temp;
        }
        double temp = constants[i];
        constants[i] = constants[k];
        constants[k] = temp;
      }
    }

    // Make the elements below the pivot element equal to zero
    for (int k = i + 1; k < n; k++) {
      double factor = coefficients[k][i] / coefficients[i][i];
      for (int j = i; j < n; j++) {
        coefficients[k][j] -= factor * coefficients[i][j];
      }
      constants[k] -= factor * constants[i];
    }
  }

  // Back substitution
  for (int i = n - 1; i >= 0; i--) {
    solution[i] = constants[i];
    for (int j = i + 1; j < n; j++) {
      solution[i] -= coefficients[i][j] * solution[j];
    }
    solution[i] /= coefficients[i][i];
  }
}

int main() {
  double coefficients[3][3] = {{2, 3, -1}, {4, 1, 2}, {-3, 2, 1}};
  double constants[3] = {1, -2, 3};
  double solution[3];

  gaussianElimination(coefficients, constants, solution, 3);

  for (int i = 0; i < 3; i++) {
    printf("[%d] = %.2f\n", i, solution[i]);
  }
}
