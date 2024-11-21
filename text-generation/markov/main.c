#include <ctype.h>
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

char **tokenize(char *data, int *nTokens) {
  char **tokens = malloc(sizeof(char *) * 10000);
  char *token = strtok(data, " \n");
  int count = 0;

  while (token != NULL) {
    tokens[count] = token;
    for (int i = 0; i < strlen(token); i++) {
      token[i] = tolower(token[i]);
    }
    count++;
    token = strtok(NULL, " \n");
  }
  *nTokens = count;
  return tokens;
}

int main() {
  srand(time(NULL));

  FILE *f = fopen("input.txt", "r");
  char *data = read_file(f);
  fclose(f);

  int nTokens = 0;
  char **tokens = tokenize(data, &nTokens);
}
