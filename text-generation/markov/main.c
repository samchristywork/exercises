#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INITIAL_CAPACITY 1000
#define ITERATIONS 10
#define TEXT_LENGTH 100

void exit_with_message(const char *message) {
  fprintf(stderr, "%s\n", message);
  exit(EXIT_FAILURE);
}

void *allocate_memory(size_t size) {
  void *ptr = malloc(size);
  if (!ptr) {
    exit_with_message("Memory allocation failed");
  }
  return ptr;
}

FILE *open_file(const char *filename, const char *mode) {
  FILE *file = fopen(filename, mode);
  if (!file) {
    exit_with_message("File opening failed");
  }
  return file;
}

char *read_file(const char *filename) {
  FILE *f = open_file(filename, "r");

  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  rewind(f);

  char *data = allocate_memory(size + 1);
  fread(data, 1, size, f);
  data[size] = '\0';

  fclose(f);
  return data;
}

char **expand_capacity(char **tokens, int *capacity) {
  *capacity *= 2;
  tokens = realloc(tokens, (*capacity) * sizeof(char *));
  if (!tokens) {
    exit_with_message("Memory reallocation failed");
  }
  return tokens;
}

char **tokenize(char *data, int *nTokens) {
  int capacity = INITIAL_CAPACITY;
  char **tokens = allocate_memory(capacity * sizeof(char *));
  char *token = strtok(data, " \n");
  int count = 0;

  while (token) {
    if (count >= capacity) {
      tokens = expand_capacity(tokens, &capacity);
    }

    tokens[count] = allocate_memory(strlen(token) + 1);
    strcpy(tokens[count], token);
    for (int i = 0; tokens[count][i]; i++) {
      tokens[count][i] = tolower(tokens[count][i]);
    }
    count++;
    token = strtok(NULL, " \n");
  }

  *nTokens = count;
  return tokens;
}

char *predict_next_word(char **tokens, int nTokens, const char *word) {
  char **nextWords = allocate_memory(nTokens * sizeof(char *));
  int nextWordsCount = 0;

  for (int i = 0; i < nTokens - 1; i++) {
    if (strcmp(tokens[i], word) == 0) {
      nextWords[nextWordsCount++] = tokens[i + 1];
    }
  }

  char *result = nextWordsCount > 0 ? nextWords[rand() % nextWordsCount] : NULL;
  free(nextWords);
  return result;
}

void generate_text(const char *startWord, int length, char **tokens,
                   int nTokens) {
  const char *currentWord = startWord;
  printf("%s ", currentWord);

  for (int i = 0; i < length; i++) {
    currentWord = predict_next_word(tokens, nTokens, currentWord);
    if (!currentWord) {
      break;
    }
    printf("%s ", currentWord);
  }
  printf("\n");
}

void free_tokens(char **tokens, int nTokens) {
  for (int i = 0; i < nTokens; i++) {
    free(tokens[i]);
  }
  free(tokens);
}

void usage(char *name) {
  fprintf(stderr, "Usage: %s [options] <filename>\n", name);
  fprintf(stderr, "\nOptions:\n");
  fprintf(stderr, "  -h, --help    Show this help message\n");
  fprintf(stderr,
          "  -l, --length <n>  Length of generated text (default: %d)\n",
          TEXT_LENGTH);
  fprintf(stderr,
          "  -i, --iterations <n>   Number of iterations (default: %d)\n",
          ITERATIONS);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  srand((unsigned)time(NULL));

  if (argc < 2) {
    usage(argv[0]);
  }

  int textLength = TEXT_LENGTH;
  int iterations = ITERATIONS;
  char *filename = NULL;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      usage(argv[0]);
    } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--length") == 0) {
      if (i + 1 < argc) {
        textLength = atoi(argv[++i]);
      } else {
        fprintf(stderr, "Error: Missing argument for -l/--length\n");
        exit(EXIT_FAILURE);
      }
    } else if (strcmp(argv[i], "-i") == 0 ||
               strcmp(argv[i], "--iterations") == 0) {
      if (i + 1 < argc) {
        iterations = atoi(argv[++i]);
      } else {
        fprintf(stderr, "Error: Missing argument for -i/--iterations\n");
        exit(EXIT_FAILURE);
      }
    } else {
      filename = argv[i];
    }
  }

  char *data = read_file(filename);
  int nTokens = 0;
  char **tokens = tokenize(data, &nTokens);
  free(data);

  for (int i = 0; i < iterations; i++) {
    char *startWord = tokens[rand() % nTokens];
    generate_text(startWord, textLength, tokens, nTokens);
  }

  free_tokens(tokens, nTokens);
}
