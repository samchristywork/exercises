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
  if (data == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }
  fread(data, 1, size, f);
  data[size] = '\0';

  return data;
}

char **tokenize(char *data, int *nTokens) {
  int capacity = 1000;
  char **tokens = malloc(sizeof(char *) * capacity);
  if (tokens == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }

  char *token = strtok(data, " \n");
  int count = 0;

  while (token != NULL) {
    if (count >= capacity) {
      capacity *= 2;
      tokens = realloc(tokens, sizeof(char *) * capacity);
      if (tokens == NULL) {
        fprintf(stderr, "Memory reallocation failed\n");
        exit(1);
      }
    }

    tokens[count] = token;
    for (int i = 0; token[i]; i++) {
      token[i] = tolower(token[i]);
    }
    count++;
    token = strtok(NULL, " \n");
  }

  *nTokens = count;
  return tokens;
}

char *predict(char **tokens, int nTokens, char *word) {
  char **nextWords = malloc(sizeof(char *) * nTokens);
  if (nextWords == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }

  int nextWordsCount = 0;

  for (int i = 0; i < nTokens - 1; i++) {
    if (strcmp(tokens[i], word) == 0) {
      nextWords[nextWordsCount] = tokens[i + 1];
      nextWordsCount++;
    }
  }

  char *result = NULL;
  if (nextWordsCount > 0) {
    int randomIndex = rand() % nextWordsCount;
    result = nextWords[randomIndex];
  }

  free(nextWords);
  return result;
}

int main() {
  srand(time(NULL));

  FILE *f = fopen("input.txt", "r");
  if (f == NULL) {
    fprintf(stderr, "Could not open file input.txt\n");
    return 1;
  }

  char *data = read_file(f);
  fclose(f);

  int nTokens = 0;
  char **tokens = tokenize(data, &nTokens);

  char *word = "markov";

  printf("%s ", word);
  for (int i = 0; i < 100; i++) {
    char *nextWord = predict(tokens, nTokens, word);
    if (nextWord == NULL) {
      break;
    }
    printf("%s ", nextWord);
    word = nextWord;
  }
  printf("\n");

  free(tokens);
  free(data);
}
