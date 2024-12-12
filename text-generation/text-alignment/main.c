#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER };

void process_line(char *line, int width, int align) {
  int len = strlen(line);
  int startOfLine = 0;

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

      if (align == ALIGN_RIGHT) {
        for (int j = 0; j < width - (i - startOfLine); j++) {
          putchar(' ');
        }
      } else if (align == ALIGN_CENTER) {
        int padding = (width - (i - startOfLine)) / 2;
        for (int j = 0; j < padding; j++) {
          putchar(' ');
        }
      }

      fwrite(line + startOfLine, i - startOfLine, 1, stdout);
      printf("\n");

      while (line[i] == ' ') {
        i++;
      }

      startOfLine = i;
    }
  }

  if (startOfLine < len) {
    if (align == ALIGN_LEFT) {
      fwrite(line + startOfLine, len - startOfLine, 1, stdout);
    } else if (align == ALIGN_RIGHT) {
      for (int j = 0; j < width - (len - startOfLine); j++) {
        putchar(' ');
      }
      fwrite(line + startOfLine, len - startOfLine, 1, stdout);
    } else if (align == ALIGN_CENTER) {
      int padding = (width - (len - startOfLine)) / 2;
      for (int j = 0; j < padding; j++) {
        putchar(' ');
      }
      fwrite(line + startOfLine, len - startOfLine, 1, stdout);
    }
    printf("\n");
  }
}

void usage(const char *name) {
  fprintf(stderr, "Usage: %s [OPTIONS]\n"
      "\n"
      "Options:\n"
      "  -l, --left          Align text to the left (default)\n"
      "  -r, --right         Align text to the right\n"
      "  -c, --center        Center align text\n"
      "\n", name);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  int width = 80;
  char *line = NULL;
  size_t length = 0;
  int align = ALIGN_LEFT;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--left") == 0) {
      align = ALIGN_LEFT;
    } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--right") == 0) {
      align = ALIGN_RIGHT;
    } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--center") == 0) {
      align = ALIGN_CENTER;
    }
  }

  while (getline(&line, &length, stdin) != -1) {
    process_line(line, width, align);
  }

  free(line);
}
