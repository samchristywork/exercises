#include <stdio.h>
#include <string.h>

void process_line(char *line, int width) {
  int len = strlen(line);
  int startOfLine = 0;

  for (int i = 0; i < len; i++) {
    if (i - startOfLine >= width) {
      while (i > startOfLine && line[i] != ' ') {
        i--;
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
    fwrite(line + startOfLine, len - startOfLine, 1, stdout);
  }
}

int main() {
  int width = 80;
  char line[1024];

  while (fgets(line, sizeof(line), stdin)) {
    process_line(line, width);
    printf("\n");
  }
}
