#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void generate_sine_wave(const char *filename, double frequency,
                        double amplitude, int sample_rate) {
  FILE *file = fopen(filename, "w");
  if (file == NULL) {
    perror("Error opening file");
    return;
  }

  for (int i = 0; i < sample_rate; i++) {
    double t = (double)i / sample_rate;
    double sample = amplitude * sin(2 * M_PI * frequency * t);
    fprintf(file, "%f\n", sample);
  }

  fclose(file);
}

void generate_square_wave(const char *filename, double frequency,
                          double amplitude, int sample_rate) {
  FILE *file = fopen(filename, "w");
  if (file == NULL) {
    perror("Error opening file");
    return;
  }

  for (int i = 0; i < sample_rate; i++) {
    double t = (double)i / sample_rate;
    double sample =
        (sin(2 * M_PI * frequency * t) >= 0) ? amplitude : -amplitude;
    fprintf(file, "%f\n", sample);
  }

  fclose(file);
}

void generate_dc_signal(const char *filename, double amplitude,
                        int sample_rate) {
  FILE *file = fopen(filename, "w");
  if (file == NULL) {
    perror("Error opening file");
    return;
  }

  for (int i = 0; i < sample_rate; i++) {
    fprintf(file, "%f\n", amplitude);
  }

  fclose(file);
}

void generate_white_noise(const char *filename, double amplitude,
                          int sample_rate) {
  FILE *file = fopen(filename, "w");
  if (file == NULL) {
    perror("Error opening file");
    return;
  }

  for (int i = 0; i < sample_rate; i++) {
    double sample = ((double)rand() / RAND_MAX) * 2 * amplitude - amplitude;
    fprintf(file, "%f\n", sample);
  }

  fclose(file);
}

int main() {
  generate_sine_wave("sine.txt", 440, 1.0, 44100);
  generate_square_wave("square.txt", 440, 1.0, 44100);
  generate_dc_signal("dc.txt", 1.0, 44100);
  generate_white_noise("noise.txt", 1.0, 44100);
}
