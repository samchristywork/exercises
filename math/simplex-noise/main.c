#include <libpng/png.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

int perm[512];
int p[] = {151, 160, 137, 91,  90,  15,  131, 13,  201, 95,  96,  53,  194, 233,
           7,   225, 140, 36,  103, 30,  69,  142, 8,   99,  37,  240, 21,  10,
           23,  190, 6,   148, 247, 120, 234, 75,  0,   26,  197, 62,  94,  252,
           219, 203, 117, 35,  11,  32,  57,  177, 33,  88,  237, 149, 56,  87,
           174, 20,  125, 136, 171, 168, 68,  175, 74,  165, 71,  134, 139, 48,
           27,  166, 77,  146, 158, 231, 83,  111, 229, 122, 60,  211, 133, 230,
           220, 105, 92,  41,  55,  46,  245, 40,  244, 102, 143, 54,  65,  25,
           63,  161, 1,   216, 80,  73,  209, 76,  132, 187, 208, 89,  18,  169,
           200, 196, 135, 130, 116, 188, 159, 86,  164, 100, 109, 198, 173, 186,
           3,   64,  52,  217, 226, 250, 124, 123, 5,   202, 38,  147, 118, 126,
           255, 82,  85,  212, 207, 206, 59,  227, 47,  16,  58,  17,  182, 189,
           28,  42,  223, 183, 170, 213, 119, 248, 152, 2,   44,  154, 163, 70,
           221, 153, 101, 155, 167, 43,  172, 9,   129, 22,  39,  253, 19,  98,
           108, 110, 79,  113, 224, 232, 178, 185, 112, 104, 218, 246, 97,  228,
           251, 34,  242, 193, 238, 210, 144, 12,  191, 179, 162, 241, 81,  51,
           145, 235, 249, 14,  239, 107, 49,  192, 214, 31,  181, 199, 106, 157,
           184, 84,  204, 176, 115, 121, 50,  45,  127, 4,   150, 254, 138, 236,
           205, 93,  222, 114, 67,  29,  24,  72,  243, 141, 128, 195, 78,  66,
           215, 61,  156, 180};

double grad3[][3] = {{1, 1, 0}, {-1, 1, 0}, {1, -1, 0}, {-1, -1, 0},
                     {1, 0, 1}, {-1, 0, 1}, {1, 0, -1}, {-1, 0, -1},
                     {0, 1, 1}, {0, -1, 1}, {0, 1, -1}, {0, -1, -1}};

typedef struct {
  unsigned char r, g, b;
} Color;

double dot(double g[], double x, double y) { return g[0] * x + g[1] * y; }

// Input value is assumed to be between 0 and 1
double map(double x, double min, double max) { return x * (max - min) + min; }

double noise(double x, double y) {
  const double F2 = 0.5 * (sqrt(3.0) - 1.0);
  const double G2 = (3.0 - sqrt(3.0)) / 6.0;

  double s = (x + y) * F2;
  int i = floor(x + s);
  int j = floor(y + s);
  double t = (i + j) * G2;
  double X0 = i - t;
  double Y0 = j - t;
  double x0 = x - X0;
  double y0 = y - Y0;

  int i1, j1;
  if (x0 > y0) {
    i1 = 1;
    j1 = 0;
  } else {
    i1 = 0;
    j1 = 1;
  }

  double x1 = x0 - i1 + G2;
  double y1 = y0 - j1 + G2;
  double x2 = x0 - 1.0 + 2.0 * G2;
  double y2 = y0 - 1.0 + 2.0 * G2;

  int ii = i & 255;
  int jj = j & 255;
  int gi0 = perm[ii + perm[jj]] % 12;
  int gi1 = perm[ii + i1 + perm[jj + j1]] % 12;
  int gi2 = perm[ii + 1 + perm[jj + 1]] % 12;

  double n0, n1, n2;
  double t0 = 0.5 - x0 * x0 - y0 * y0;
  if (t0 < 0) {
    n0 = 0.0;
  } else {
    t0 *= t0;
    n0 = t0 * t0 * dot(grad3[gi0], x0, y0);
  }

  double t1 = 0.5 - x1 * x1 - y1 * y1;
  if (t1 < 0) {
    n1 = 0.0;
  } else {
    t1 *= t1;
    n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
  }

  double t2 = 0.5 - x2 * x2 - y2 * y2;
  if (t2 < 0) {
    n2 = 0.0;
  } else {
    t2 *= t2;
    n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
  }

  return 70.0 * (n0 + n1 + n2);
}

void writePPMImage(FILE *f, int width, int height, float scale,
                   Color (*func)(double, double, int, bool, double, double),
                   bool channel, int quant, double lower, double upper,
                   bool mirror, bool invert) {
  fprintf(f, "P3\n%d %d\n255\n", width, height);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (channel) {
        Color c1 =
            func((x + 0) * scale, (y + 0) * scale, quant, mirror, lower, upper);
        Color c2 = func((x + 1e10) * scale, (y + 0) * scale, quant, mirror,
                        lower, upper);
        Color c3 = func((x + 0) * scale, (y + 1e10) * scale, quant, mirror,
                        lower, upper);

        int r = c1.r;
        int g = c2.g;
        int b = c3.b;

        if (invert) {
          r = 255 - r;
          g = 255 - g;
          b = 255 - b;
        }

        fprintf(f, "%d %d %d ", r, g, b);
      } else {
        Color c = func(x * scale, y * scale, quant, mirror, lower, upper);

        int r = c.r;
        int g = c.g;
        int b = c.b;

        if (invert) {
          r = 255 - r;
          g = 255 - g;
          b = 255 - b;
        }

        fprintf(f, "%d %d %d ", r, g, b);
      }
    }
  }
}

void writePNGImage(FILE *f, int width, int height, float scale,
                   Color (*func)(double, double, int, bool, double, double),
                   bool channel, int quant, double lower, double upper,
                   bool mirror, bool invert) {
  png_structp png_ptr =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    fprintf(stderr, "Error: png_create_write_struct\n");
    exit(EXIT_FAILURE);
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr, NULL);
    fprintf(stderr, "Error: png_create_info_struct\n");
    exit(EXIT_FAILURE);
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fprintf(stderr, "Error: setjmp\n");
    exit(EXIT_FAILURE);
  }

  png_init_io(png_ptr, f);
  png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png_ptr, info_ptr);

  png_bytep row = (png_bytep)malloc(3 * width);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (channel) {
        Color c1 =
            func((x + 0) * scale, (y + 0) * scale, quant, mirror, lower, upper);
        Color c2 = func((x + 1e10) * scale, (y + 0) * scale, quant, mirror,
                        lower, upper);
        Color c3 = func((x + 0) * scale, (y + 1e10) * scale, quant, mirror,
                        lower, upper);

        int r = c1.r;
        int g = c2.g;
        int b = c3.b;

        if (invert) {
          r = 255 - r;
          g = 255 - g;
          b = 255 - b;
        }

        row[3 * x + 0] = r;
        row[3 * x + 1] = g;
        row[3 * x + 2] = b;
      } else {
        Color c = func(x * scale, y * scale, quant, mirror, lower, upper);

        int r = c.r;
        int g = c.g;
        int b = c.b;

        if (invert) {
          r = 255 - r;
          g = 255 - g;
          b = 255 - b;
        }

        row[3 * x + 0] = r;
        row[3 * x + 1] = g;
        row[3 * x + 2] = b;
      }
    }
    png_write_row(png_ptr, row);
  }

  png_write_end(png_ptr, NULL);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  free(row);
}

Color linear(double x, double y, int quantization, bool mirror, double lower,
             double upper) {
  double f = noise(x, y) * 0.5 + 0.5;
  if (mirror) {
    f = f < 0.5 ? 0.5 - f : f - 0.5;
  }
  f = map(f, lower, upper);
  int n = f * quantization;
  n = n * 255 / quantization;
  return (Color){n, n, n};
}

Color fbm(double x, double y, int quantization, bool mirror, double lower,
          double upper) {
  double n = 0;
  double amplitude = 1;
  double frequency = 1;

  for (int i = 0; i < 8; i++) {
    n += noise(x * frequency, y * frequency) * amplitude;
    amplitude *= 0.5;
    frequency *= 2;
  }

  n = n * 0.5 + 0.5;
  if (mirror) {
    n = n < 0.5 ? 0.5 - n : n - 0.5;
  }
  n = map(n, lower, upper);

  int m = n * quantization;
  m = m * 255 / quantization;
  m = max(min(m, 255), 0);

  return (Color){m, m, m};
}

Color step(double x, double y, int quantization, bool mirror, double lower,
           double upper) {
  double n = noise(x, y);
  n = map(n, lower, upper);
  return (Color){n > 0 ? 255 : 0, n > 0 ? 255 : 0, n > 0 ? 255 : 0};
}

void usage(char *name) {
  printf("Usage: %s [options]\n"
         "\n"
         "Options:\n"
         "  -w <width>    Width of the image (default: 512)\n"
         "  -h <height>   Height of the image (default: 512)\n"
         "  -s <scale>    Scale of the noise (default: 0.01)\n"
         "  -t <type>     File type (default PPM)\n"
         "  -f <file>     Output file (default stdout)\n"
         "  -r <style>    Style of the noise (default linear)\n"
         "  -l <f>        Value lower bound (default 0)\n"
         "  -u <f>        Value upper bound (default 1)\n"
         "  -c            Use a different seed for each channel\n"
         "  -m            Mirror the noise in the value domain\n"
         "  -i            Invert the noise\n"
         "  -q <n>        Color quantization modifier (default 256)\n"
         "  -h            Show this help message\n"
         "\n"
         "Supported file types:\n"
         "  ppm           Portable Pixel Map\n"
         "  png           Portable Network Graphics\n"
         "\n"
         "Supported styles:\n"
         "  linear        Linear interpolation\n"
         "  fbm           Fractional Brownian motion\n"
         "  step          Step interpolation\n",
         name);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  int width = 512;
  int height = 512;
  float scale = 0.01;
  FILE *f = stdout;
  bool channel = false;
  int quantization = 256;
  double lower = 0;
  double upper = 1;
  bool mirror = false;
  bool invert = false;
  Color (*func)(double, double, int, bool, double, double) = linear;
  enum { PPM, PNG } type = PPM;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == 'w' && i + 1 < argc) {
        width = atoi(argv[++i]);
      } else if (argv[i][1] == 'h' && i + 1 < argc) {
        height = atoi(argv[++i]);
      } else if (argv[i][1] == 's' && i + 1 < argc) {
        scale = atof(argv[++i]);
      } else if (argv[i][1] == 't' && i + 1 < argc) {
        if (strcmp(argv[++i], "ppm") == 0) {
          type = PPM;
        } else if (strcmp(argv[i], "png") == 0) {
          type = PNG;
        } else {
          usage(argv[0]);
        }
      } else if (argv[i][1] == 'f' && i + 1 < argc) {
        f = fopen(argv[++i], "wb");
        if (!f) {
          fprintf(stderr, "Error: fopen\n");
          exit(EXIT_FAILURE);
        }
      } else if (argv[i][1] == 'r' && i + 1 < argc) {
        if (strcmp(argv[++i], "linear") == 0) {
          func = linear;
        } else if (strcmp(argv[i], "fbm") == 0) {
          func = fbm;
        } else if (strcmp(argv[i], "step") == 0) {
          func = step;
        } else {
          usage(argv[0]);
        }
      } else if (argv[i][1] == 'u' && i + 1 < argc) {
        upper = atof(argv[++i]);
      } else if (argv[i][1] == 'l' && i + 1 < argc) {
        lower = atof(argv[++i]);
      } else if (argv[i][1] == 'c') {
        channel = true;
      } else if (argv[i][1] == 'm') {
        mirror = true;
      } else if (argv[i][1] == 'i') {
        invert = true;
      } else if (argv[i][1] == 'q' && i + 1 < argc) {
        quantization = atoi(argv[++i]);
        if (quantization <= 0) {
          fprintf(stderr, "Error: quantization must be greater than 0\n");
          exit(EXIT_FAILURE);
        }
      } else if (argv[i][1] == 'h') {
        usage(argv[0]);
      } else {
        usage(argv[0]);
      }
    }
  }

  for (int i = 0; i < 256; i++) {
    perm[i] = p[i];
    perm[i + 256] = p[i];
  }

  switch (type) {
  case PPM:
    writePPMImage(f, width, height, scale, func, channel, quantization, lower,
                  upper, mirror, invert);
    break;
  case PNG:
    writePNGImage(f, width, height, scale, func, channel, quantization, lower,
                  upper, mirror, invert);
    break;
  default:
    usage(argv[0]);
  }
}
