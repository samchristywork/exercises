#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER };

void process_line(char *line, int width, int justify) {
  int len = strlen(line);
  int startOfLine = 0;

  for (int i = 0; i < len; i++) {
    if (i - startOfLine >= width) {
      int spaces = 0;
      while (i > startOfLine && line[i] != ' ') {
        i--;
        spaces++;
      }

      if (justify == ALIGN_RIGHT) {
        for (int j = 0; j < width - (i - startOfLine); j++) {
          putchar(' ');
        }
      } else if (justify == ALIGN_CENTER) {
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
    if (justify == ALIGN_LEFT) {
      fwrite(line + startOfLine, len - startOfLine, 1, stdout);
    } else if (justify == ALIGN_RIGHT) {
      for (int j = 0; j < width - (len - startOfLine); j++) {
        putchar(' ');
      }
      fwrite(line + startOfLine, len - startOfLine, 1, stdout);
    } else if (justify == ALIGN_CENTER) {
      int padding = (width - (len - startOfLine)) / 2;
      for (int j = 0; j < padding; j++) {
        putchar(' ');
      }
      fwrite(line + startOfLine, len - startOfLine, 1, stdout);
    }
    printf("\n");
  }
}

int main() {
  int width = 80;
  char *line = NULL;
  size_t length = 0;

  while (getline(&line, &length, stdin) != -1) {
    process_line(line, width, ALIGN_CENTER);
  }

  free(line);
}
