#include <libpng/png.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Input value is assumed to be between 0 and 1
#define map(x, range) ((x) * ((range.max) - (range.min)) + (range.min))

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

typedef struct {
  double min;
  double max;
} Range;

typedef struct {
  int width;
  int height;
} Dimension;

typedef struct {
  double x;
  double y;
} Vec2;

typedef struct {
  int quantization;
  bool mirror;
  Range range;
  double scale;
} PixelProperties;

typedef Color (*StyleFunc)(Vec2, PixelProperties);

typedef struct {
  Dimension dimensions;
  bool channel;
  int quantization;
  Range range;
  bool mirror;
  bool invert;
  double scale;
  StyleFunc func;
} ImageProperties;

double dot(double g[], Vec2 v) { return g[0] * v.x + g[1] * v.y; }

double noise(Vec2 pos) {
  const double F2 = 0.5 * (sqrt(3.0) - 1.0);
  const double G2 = (3.0 - sqrt(3.0)) / 6.0;

  double s = (pos.x + pos.y) * F2;
  int i = floor(pos.x + s);
  int j = floor(pos.y + s);
  double t = (i + j) * G2;

  Vec2 v0 = {pos.x - (i - t), pos.y - (j - t)};

  int i1, j1;
  if (v0.x > v0.y) {
    i1 = 1;
    j1 = 0;
  } else {
    i1 = 0;
    j1 = 1;
  }

  Vec2 v1 = {v0.x - i1 + G2, v0.y - j1 + G2};
  Vec2 v2 = {v0.x - 1.0 + 2.0 * G2, v0.y - 1.0 + 2.0 * G2};

  int ii = i & 255;
  int jj = j & 255;
  int gi0 = perm[ii + perm[jj]] % 12;
  int gi1 = perm[ii + i1 + perm[jj + j1]] % 12;
  int gi2 = perm[ii + 1 + perm[jj + 1]] % 12;

  double n0, n1, n2;

  double t0 = 0.5 - v0.x * v0.x - v0.y * v0.y;
  if (t0 < 0) {
    n0 = 0.0;
  } else {
    t0 *= t0;
    n0 = t0 * t0 * dot(grad3[gi0], v0);
  }

  double t1 = 0.5 - v1.x * v1.x - v1.y * v1.y;
  if (t1 < 0) {
    n1 = 0.0;
  } else {
    t1 *= t1;
    n1 = t1 * t1 * dot(grad3[gi1], v1);
  }

  double t2 = 0.5 - v2.x * v2.x - v2.y * v2.y;
  if (t2 < 0) {
    n2 = 0.0;
  } else {
    t2 *= t2;
    n2 = t2 * t2 * dot(grad3[gi2], v2);
  }

  return 70.0 * (n0 + n1 + n2);
}

Color mixOffsetNoise(Vec2 pos, PixelProperties p, StyleFunc func) {
  Color c1 = func((Vec2){pos.x * p.scale, pos.y * p.scale}, p);
  Color c2 = func((Vec2){pos.x * p.scale + 1e5, pos.y * p.scale}, p);
  Color c3 = func((Vec2){pos.x * p.scale, pos.y * p.scale + 1e5}, p);

  return (Color){c1.r, c2.g, c3.b};
}

void writePPMImage(FILE *f, ImageProperties properties) {
  PixelProperties pixelProperties = {
      .quantization = properties.quantization,
      .mirror = properties.mirror,
      .range = properties.range,
      .scale = properties.scale,
  };

  fprintf(f, "P3\n%d %d\n255\n", properties.dimensions.width,
          properties.dimensions.height);
  for (int y = 0; y < properties.dimensions.height; y++) {
    for (int x = 0; x < properties.dimensions.width; x++) {
      Color c;
      if (properties.channel) {
        c = mixOffsetNoise((Vec2){x, y}, pixelProperties, properties.func);
      } else {
        c = properties.func((Vec2){x * properties.scale, y * properties.scale},
                            pixelProperties);
      }

      int r = c.r;
      int g = c.g;
      int b = c.b;

      if (properties.invert) {
        r = 255 - r;
        g = 255 - g;
        b = 255 - b;
      }

      fprintf(f, "%d %d %d ", r, g, b);
    }
  }
}

void writePNGImage(FILE *f, ImageProperties properties) {
  PixelProperties pixelProperties = {
      .quantization = properties.quantization,
      .mirror = properties.mirror,
      .range = properties.range,
      .scale = properties.scale,
  };

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
  png_set_IHDR(png_ptr, info_ptr, properties.dimensions.width,
               properties.dimensions.height, 8, PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png_ptr, info_ptr);

  png_bytep row = (png_bytep)malloc(3 * properties.dimensions.width);
  for (int y = 0; y < properties.dimensions.height; y++) {
    for (int x = 0; x < properties.dimensions.width; x++) {
      Color c;
      if (properties.channel) {
        c = mixOffsetNoise((Vec2){x, y}, pixelProperties, properties.func);
      } else {
        c = properties.func((Vec2){x * properties.scale, y * properties.scale},
                            pixelProperties);
      }

      int r = c.r;
      int g = c.g;
      int b = c.b;

      if (properties.invert) {
        r = 255 - r;
        g = 255 - g;
        b = 255 - b;
      }

      row[3 * x + 0] = r;
      row[3 * x + 1] = g;
      row[3 * x + 2] = b;
    }
    png_write_row(png_ptr, row);
  }

  png_write_end(png_ptr, NULL);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  free(row);
}

Color linear(Vec2 pos, PixelProperties p) {
  double f = noise(pos) * 0.5 + 0.5;
  if (p.mirror) {
    f = f < 0.5 ? 0.5 - f : f - 0.5;
  }
  f = map(f, p.range);
  int n = f * p.quantization;
  n = n * 255 / p.quantization;
  return (Color){n, n, n};
}

Color fbm(Vec2 pos, PixelProperties p) {
  double n = 0;
  double amplitude = 1;
  double frequency = 1;

  for (int i = 0; i < 8; i++) {
    n += noise((Vec2){pos.x * frequency, pos.y * frequency}) * amplitude;
    amplitude *= 0.5;
    frequency *= 2;
  }

  n = n * 0.5 + 0.5;
  if (p.mirror) {
    n = n < 0.5 ? 0.5 - n : n - 0.5;
  }
  n = map(n, p.range);

  int m = n * p.quantization;
  m = m * 255 / p.quantization;
  m = MAX(MIN(m, 255), 0);

  return (Color){m, m, m};
}

Color step(Vec2 pos, PixelProperties p) {
  double n = noise(pos);
  n = map(n, p.range);
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
  FILE *f = stdout;
  enum { PPM, PNG } type = PPM;
  ImageProperties properties = {
      .dimensions = {512, 512},
      .channel = false,
      .quantization = 256,
      .range = {0, 1},
      .mirror = false,
      .invert = false,
      .scale = 0.01,
      .func = linear,
  };

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == 'w' && i + 1 < argc) {
        properties.dimensions.width = atoi(argv[++i]);
      } else if (argv[i][1] == 'h' && i + 1 < argc) {
        properties.dimensions.height = atoi(argv[++i]);
      } else if (argv[i][1] == 's' && i + 1 < argc) {
        properties.scale = atof(argv[++i]);
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
          properties.func = linear;
        } else if (strcmp(argv[i], "fbm") == 0) {
          properties.func = fbm;
        } else if (strcmp(argv[i], "step") == 0) {
          properties.func = step;
        } else {
          usage(argv[0]);
        }
      } else if (argv[i][1] == 'u' && i + 1 < argc) {
        properties.range.max = atof(argv[++i]);
      } else if (argv[i][1] == 'l' && i + 1 < argc) {
        properties.range.min = atof(argv[++i]);
      } else if (argv[i][1] == 'c') {
        properties.channel = true;
      } else if (argv[i][1] == 'm') {
        properties.mirror = true;
      } else if (argv[i][1] == 'i') {
        properties.invert = true;
      } else if (argv[i][1] == 'q' && i + 1 < argc) {
        properties.quantization = atoi(argv[++i]);
        if (properties.quantization <= 0) {
          fprintf(stderr,
                  "Error: quantization factor must be greater than 0\n");
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
    writePPMImage(f, properties);
    break;
  case PNG:
    writePNGImage(f, properties);
    break;
  default:
    usage(argv[0]);
  }
}
