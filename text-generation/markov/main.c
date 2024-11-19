#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char *read_file(FILE *f) {
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *data = malloc(size + 1);
  fread(data, 1, size, f);
  data[size] = '\0';

  return data;
}

bool notAlpha(char *token) {
  for (int i = 0; i < strlen(token); i++) {
    if (!isalpha(token[i])) {
      return true;
    }
  }
  return false;
}

int main() {
  srand(time(NULL));

  FILE *f = fopen("input.txt", "r");
  char *data = read_file(f);
  fclose(f);
}
