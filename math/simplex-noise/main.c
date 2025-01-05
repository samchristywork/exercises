#include <stdio.h>

double noise(double x, double y) {
  return 0;
}

void drawImage(int width, int height) {
  printf("P3\n%d %d\n255\n", width, height);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      double value = noise(x / 100.0, y / 100.0);
      if (value < 0) {
        printf("255 255 255\n");
      } else {
        printf("0 0 0\n");
      }
    }
    printf("\n");
  }
}

int main() {
  drawImage(512, 512);
}
