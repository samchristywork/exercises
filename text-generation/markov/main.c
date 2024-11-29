#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INITIAL_CAPACITY 1000
#define WORDS_TO_GENERATE 10
#define TEXT_LENGTH 100

void handle_memory_error(void *ptr, const char *message) {
  if (ptr == NULL) {
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
  }
}

char *read_file(const char *filename) {
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    fprintf(stderr, "Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *data = malloc(size + 1);
  handle_memory_error(data, "Memory allocation failed");

  fread(data, 1, size, f);
  data[size] = '\0';

  fclose(f);
  return data;
}

char **tokenize(const char *data, int *nTokens) {
  int capacity = INITIAL_CAPACITY;
  char **tokens = malloc(capacity * sizeof(char *));
  handle_memory_error(tokens, "Memory allocation failed");

  char *dataCopy = strdup(data);
  handle_memory_error(dataCopy, "Memory allocation failed for data copy");

  char *token = strtok(dataCopy, " \n");
  int count = 0;

  while (token != NULL) {
    if (count >= capacity) {
      capacity *= 2;
      tokens = realloc(tokens, capacity * sizeof(char *));
      handle_memory_error(tokens, "Memory reallocation failed");
    }

    tokens[count] = strdup(token);
    handle_memory_error(tokens[count], "Memory allocation failed for token");

    for (int i = 0; tokens[count][i]; i++) {
      tokens[count][i] = tolower(tokens[count][i]);
    }

    count++;
    token = strtok(NULL, " \n");
  }
  free(dataCopy);

  *nTokens = count;
  return tokens;
}

char *predict(char **tokens, int nTokens, const char *word) {
  char **nextWords = malloc(nTokens * sizeof(char *));
  handle_memory_error(nextWords, "Memory allocation failed");

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

void print_text(const char *startWord, int length, char **tokens, int nTokens) {
  const char *currentWord = startWord;
  printf("%s ", currentWord);

  for (int i = 0; i < length; i++) {
    currentWord = predict(tokens, nTokens, currentWord);
    if (currentWord == NULL) {
      break;
    }
    printf("%s ", currentWord);
  }
  printf("\n\n");
}

void free_tokens(char **tokens, int nTokens) {
  for (int i = 0; i < nTokens; i++) {
    free(tokens[i]);
  }
  free(tokens);
}

int main() {
  srand((unsigned)time(NULL));

  char *data = read_file("input.txt");
  int nTokens = 0;
  char **tokens = tokenize(data, &nTokens);
  free(data);

  for (int i = 0; i < WORDS_TO_GENERATE; i++) {
    print_text("markov", TEXT_LENGTH, tokens, nTokens);
  }

  free_tokens(tokens, nTokens);
}
