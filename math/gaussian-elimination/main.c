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

void prettyPrintProblem(double coefficients[3][3], double constants[3], int n) {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (coefficients[i][j] >= 0) {
        if (j != 0) {
          printf("+");
        }
        printf("%g%c", coefficients[i][j], 'z' + j - n + 1);
      } else {
        printf("-%g%c", -coefficients[i][j], 'z' + j - n + 1);
      }
    }
    printf("=%g\n", constants[i]);
  }
}

void prettyPrintSolution(double solution[3], int n) {
  for (int i = 0; i < n; i++) {
    printf("%c=%g\n", 'z' + i - n + 1, solution[i]);
  }
}

void testSolution(double coefficients[3][3], double constants[3],
                  double solution[3], int n) {
  for (int i = 0; i < n; i++) {
    double sum = 0;
    for (int j = 0; j < n; j++) {
      sum += coefficients[i][j] * solution[j];
    }
    printf("Equation %d: %g\n", i, sum - constants[i]);
  }
}

int main() {
  double coefficients[3][3] = {{2, 3, -1}, {4, 1, 2}, {-3, 2, 1}};
  double constants[3] = {1, -2, 3};

  prettyPrintProblem(coefficients, constants, 3);
  printf("\n");

  double solution[3];
  gaussianElimination(coefficients, constants, solution, 3);

  prettyPrintSolution(solution, 3);
  printf("\n");

  testSolution(coefficients, constants, solution, 3);
}
