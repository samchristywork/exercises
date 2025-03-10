#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER };

void print_border_top(int width) {
  width += 2;

  printf("┌");
  for (int i = 0; i < width; i++) {
    printf("─");
  }
  printf("┐");
  putchar('\n');
}

void print_border_bottom(int width) {
  width += 2;

  printf("└");
  for (int i = 0; i < width; i++) {
    printf("─");
  }
  printf("┘");
  putchar('\n');
}

void process_line(char *line, int width, int align, bool border) {
  int len = strlen(line);
  int startOfLine = 0;

  line[strcspn(line, "\n")] = '\0';

  for (int i = 0; i < len; i++) {
    if (i - startOfLine >= width) {
      int spaces = 0;
      int endOfLine = i;

      while (i > startOfLine && line[i] != ' ') {
        i--;
        spaces++;
      }

      if (i == startOfLine) {
        i = endOfLine;
      } else {
        while (line[i] == ' ') {
          i++;
        }
      }

      if (border) {
        printf("│");
        putchar(' ');
      }

      if (align == ALIGN_RIGHT) {
        for (int j = 0; j < width - (i - startOfLine - 1); j++) {
          putchar(' ');
        }
      } else if (align == ALIGN_CENTER) {
        int padding = (width - (i - startOfLine)) / 2;
        for (int j = 0; j < padding; j++) {
          putchar(' ');
        }
      }

      fwrite(line + startOfLine, i - startOfLine, 1, stdout);

      if (align == ALIGN_LEFT) {
        for (int j = 0; j < width - (i - startOfLine - 1); j++) {
          putchar(' ');
        }
      } else if (align == ALIGN_CENTER) {
        int padding = (width - (i - startOfLine)) / 2;
        int extraPadding = width - (i - startOfLine) - 2 * padding;
        for (int j = 0; j < padding + extraPadding + 1; j++) {
          putchar(' ');
        }
      }

      if (border) {
        printf("│");
      }

      printf("\n");

      while (line[i] == ' ') {
        i++;
      }

      startOfLine = i;
    }
  }

  if (startOfLine < len) {
    if (border) {
      printf("│");
      putchar(' ');
    }

    len--;
    width--;
    if (align == ALIGN_LEFT) {
      fwrite(line + startOfLine, len - startOfLine, 1, stdout);
      for (int j = 0; j < width - (len - startOfLine - 2); j++) {
        putchar(' ');
      }
    } else if (align == ALIGN_RIGHT) {
      for (int j = 0; j < width - (len - startOfLine - 1); j++) {
        putchar(' ');
      }
      fwrite(line + startOfLine, len - startOfLine, 1, stdout);
      putchar(' ');
    } else if (align == ALIGN_CENTER) {
      int padding = (width - (len - startOfLine)) / 2;
      for (int j = 0; j < padding + 2; j++) {
        putchar(' ');
      }
      fwrite(line + startOfLine, len - startOfLine, 1, stdout);
      for (int j = 0; j < padding; j++) {
        putchar(' ');
      }
      if ((width - (len - startOfLine)) % 2 != 0) {
        putchar(' ');
      }
    }

    if (border) {
      printf("│");
    }
  }

  printf("\n");
}

void usage(const char *name) {
  fprintf(stderr,
          "Usage: %s [OPTIONS]\n"
          "\n"
          "Options:\n"
          "  -l, --left          Align text to the left (default)\n"
          "  -r, --right         Align text to the right\n"
          "  -c, --center        Center align text\n"
          "  -w, --width WIDTH   Set the width of the output (default: 80)\n"
          "  -b, --border        Add a border around the text\n"
          "  -h, --help          Show this help message\n",
          name);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  int width = 80;
  char *line = NULL;
  size_t length = 0;
  int align = ALIGN_LEFT;
  bool border = false;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--left") == 0) {
      align = ALIGN_LEFT;
    } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--right") == 0) {
      align = ALIGN_RIGHT;
    } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--center") == 0) {
      align = ALIGN_CENTER;
    } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--width") == 0) {
      if (i + 1 < argc) {
        width = atoi(argv[++i]);
        if (width <= 0) {
          usage(argv[0]);
        }
      } else {
        usage(argv[0]);
      }
    } else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--border") == 0) {
      border = true;
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      usage(argv[0]);
    } else {
      fprintf(stderr, "Unknown option: %s\n", argv[i]);
      usage(argv[0]);
    }
  }

  if (border) {
    print_border_top(width);
  }

  while (getline(&line, &length, stdin) != -1) {
    process_line(line, width, align, border);
  }

  if (border) {
    print_border_bottom(width);
  }

  free(line);
}
