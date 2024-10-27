#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TOKEN_DEFINE,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_ENDIF,
  TOKEN_TEXT,
} TokenType;

typedef struct {
  char *key;
  char *parameters;
  char *value;
} DefineDetails;

typedef struct Token {
  char *start;
  size_t length;
  TokenType type;
  DefineDetails defineDetails;
} Token;

char *readStdin() {
  size_t bufferSize = 1024;
  size_t bytesRead = 0;
  char *buffer = malloc(bufferSize);

  if (buffer == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    return NULL;
  }

  while (1) {
    size_t result = fread(buffer + bytesRead, 1, bufferSize - bytesRead, stdin);
    bytesRead += result;

    if (result < bufferSize - bytesRead) {
      if (feof(stdin)) {
        break;
      } else if (ferror(stdin)) {
        fprintf(stderr, "Error reading from stdin\n");
        free(buffer);
        return NULL;
      }
    }

    if (bytesRead >= bufferSize) {
      bufferSize *= 2;
      char *newBuffer = realloc(buffer, bufferSize);
      if (newBuffer == NULL) {
        fprintf(stderr, "Memory reallocation failed\n");
        free(buffer);
        return NULL;
      }
      buffer = newBuffer;
    }
  }

  if (bytesRead < bufferSize) {
    buffer[bytesRead] = '\0';
  } else {
    char *newBuffer = realloc(buffer, bytesRead + 1);
    if (newBuffer == NULL) {
      fprintf(stderr, "Memory reallocation failed\n");
      free(buffer);
      return NULL;
    }
    buffer = newBuffer;
    buffer[bytesRead] = '\0';
  }

  return buffer;
}

void printTokenType(TokenType type) {
  switch (type) {
    case TOKEN_DEFINE:
      printf("TOKEN_DEFINE");
      break;
    case TOKEN_IF:
      printf("TOKEN_IF");
      break;
    case TOKEN_ELSE:
      printf("TOKEN_ELSE");
      break;
    case TOKEN_ENDIF:
      printf("TOKEN_ENDIF");
      break;
    case TOKEN_TEXT:
      printf("TOKEN_TEXT");
      break;
  }
}

void printDefineDetails(DefineDetails *details) {
  printf("Key: %s, ", details->key ? details->key : "N/A");
  printf("Parameters: %s, ", details->parameters ? details->parameters : "N/A");
  printf("Value: %s\n", details->value ? details->value : "N/A");
}

int main() {
  char *buffer = readStdin();

  Token tokens[999];
  int tokenCount = 0;

  for (size_t i = 0; i < strlen(buffer); i++) {
    if (strncmp(buffer + i, "#define", 7) == 0) {
      tokens[tokenCount].start = buffer + i;
      tokens[tokenCount].type = TOKEN_DEFINE;

      i += 7;

      while (buffer[i] == ' ' || buffer[i] == '\t') {
        i++;
      }

      char *keyStart = buffer + i;
      while (buffer[i] != ' ' && buffer[i] != '\t' && buffer[i] != '(' && buffer[i] != '\n') {
        i++;
      }
      tokens[tokenCount].defineDetails.key = strndup(keyStart, buffer + i - keyStart);

      if (buffer[i] == '(') {
        i++;
        char *paramStart = buffer + i;
        while (buffer[i] != ')') {
          i++;
        }
        tokens[tokenCount].defineDetails.parameters = strndup(paramStart, buffer + i - paramStart);
        i++;
      } else {
        tokens[tokenCount].defineDetails.parameters = NULL;
      }

      while (buffer[i] == ' ' || buffer[i] == '\t') {
        i++;
      }

      char *valueStart = buffer + i;
      while (buffer[i] != '\n') {
        i++;
      }
      tokens[tokenCount].defineDetails.value = strndup(valueStart, buffer + i - valueStart);

      tokens[tokenCount].length = buffer + i - tokens[tokenCount].start;
      tokenCount++;

    } else if (strncmp(buffer + i, "#if", 3) == 0) {
      tokens[tokenCount].start = buffer + i;
      while (buffer[i] != '\n') {
        i++;
      }
      tokens[tokenCount].length = buffer + i - tokens[tokenCount].start;
      tokens[tokenCount].type = TOKEN_IF;
      tokenCount++;
    } else if (strncmp(buffer + i, "#else", 5) == 0) {
      tokens[tokenCount].start = buffer + i;
      while (buffer[i] != '\n') {
        i++;
      }
      tokens[tokenCount].length = buffer + i - tokens[tokenCount].start;
      tokens[tokenCount].type = TOKEN_ELSE;
      tokenCount++;
    } else if (strncmp(buffer + i, "#endif", 6) == 0) {
      tokens[tokenCount].start = buffer + i;
      while (buffer[i] != '\n') {
        i++;
      }
      tokens[tokenCount].length = buffer + i - tokens[tokenCount].start;
      tokens[tokenCount].type = TOKEN_ENDIF;
      tokenCount++;
    } else {
      tokens[tokenCount].start = buffer + i;
      while (buffer[i] != '\n' && buffer[i] != '\0') {
        if (buffer[i] == '#') {
          break;
        }
        i++;
      }
      tokens[tokenCount].length = buffer + i - tokens[tokenCount].start;
      tokens[tokenCount].type = TOKEN_TEXT;
      tokenCount++;
    }
  }

  for (int i = 0; i < tokenCount; i++) {
    Token token = tokens[i];
    switch (token.type) {
      case TOKEN_DEFINE:
        printf("TOKEN_DEFINE: ");
        printDefineDetails(&token.defineDetails);
        break;
      case TOKEN_IF:
        printf("TOKEN_IF\n");
        break;
      case TOKEN_ELSE:
        printf("TOKEN_ELSE\n");
        break;
      case TOKEN_ENDIF:
        printf("TOKEN_ENDIF\n");
        break;
      case TOKEN_TEXT:
        printf("TOKEN_TEXT\n");
        break;
    }
  }

  for (int i = 0; i < tokenCount; i++) {
    if (tokens[i].type == TOKEN_DEFINE) {
      free(tokens[i].defineDetails.key);
      free(tokens[i].defineDetails.parameters);
      free(tokens[i].defineDetails.value);
    }
  }
  free(buffer);
}
