#include <libpng/png.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

double dot(double g[], double x, double y) { return g[0] * x + g[1] * y; }

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
  if (t0 < 0)
    n0 = 0.0;
  else {
    t0 *= t0;
    n0 = t0 * t0 * dot(grad3[gi0], x0, y0);
  }

  double t1 = 0.5 - x1 * x1 - y1 * y1;
  if (t1 < 0)
    n1 = 0.0;
  else {
    t1 *= t1;
    n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
  }

  double t2 = 0.5 - x2 * x2 - y2 * y2;
  if (t2 < 0)
    n2 = 0.0;
  else {
    t2 *= t2;
    n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
  }

  return 70.0 * (n0 + n1 + n2);
}

void writePPMImage(FILE *f, int width, int height, float scale) {
  fprintf(f, "P3\n%d %d\n255\n", width, height);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      double value = noise(x * scale, y * scale);
      int s = (int)(value * 128 + 128);
      fprintf(f, "%d %d %d ", s, s, s);
    }
  }
}

void writePNGImage(FILE *f, int width, int height, float scale) {
  png_structp png_ptr =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    fprintf(stderr, "Error: png_create_write_struct\n");
    exit(1);
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr, NULL);
    fprintf(stderr, "Error: png_create_info_struct\n");
    exit(1);
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fprintf(stderr, "Error: setjmp\n");
    exit(1);
  }

  png_init_io(png_ptr, f);
  png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png_ptr, info_ptr);

  png_bytep row = (png_bytep)malloc(3 * width);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      double value = noise(x * scale, y * scale);
      int s = (int)(value * 128 + 128);
      row[3 * x + 0] = s;
      row[3 * x + 1] = s;
      row[3 * x + 2] = s;
    }
    png_write_row(png_ptr, row);
  }

  png_write_end(png_ptr, NULL);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  free(row);
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
         "  -h            Show this help message\n"
         "\n"
         "Supported file types:\n"
         "  PPM           Portable Pixel Map\n"
         "  PNG           Portable Network Graphics\n",
         name);
  exit(1);
}

int main(int argc, char *argv[]) {
  int width = 512;
  int height = 512;
  float scale = 0.01;
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
        if (strcmp(argv[++i], "PPM") == 0) {
          type = PPM;
        } else if (strcmp(argv[i], "PNG") == 0) {
          type = PNG;
        } else {
          usage(argv[0]);
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
    writePPMImage(stdout, width, height, scale);
    break;
  case PNG:
    writePNGImage(stdout, width, height, scale);
    break;
  default:
    usage(argv[0]);
  }
}
