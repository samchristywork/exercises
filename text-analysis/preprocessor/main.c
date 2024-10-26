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

typedef struct Token {
  char *start;
  size_t length;
  TokenType type;
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

void printTokenBody(Token token) {
  for (int i=0; i<token.length; i++) {
    printf("%c", token.start[i]);
  }
  printf("\n");
}

int main() {
  char *buffer = readStdin();

  Token tokens[999];
  int tokenCount = 0;

  for (int i=0; i<strlen(buffer); i++) {
    if (strncmp(buffer + i, "#define", 7) == 0) {
      tokens[tokenCount].start = buffer + i;
      while (buffer[i] != '\n') {
        i++;
      }
      tokens[tokenCount].length = buffer + i - tokens[tokenCount].start;
      tokens[tokenCount].type = TOKEN_DEFINE;
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
      while (buffer[i] != '#') {
        i++;
      }
      i--;
      tokens[tokenCount].length = buffer + i - tokens[tokenCount].start;
      tokens[tokenCount].type = TOKEN_TEXT;
      tokenCount++;
    }
  }

  for (int i=0; i<tokenCount; i++) {
    Token token = tokens[i];
    switch (token.type) {
      case TOKEN_DEFINE:
        printf("TOKEN_DEFINE: ");
        break;
      case TOKEN_IF:
        printf("TOKEN_IF: ");
        break;
      case TOKEN_ELSE:
        printf("TOKEN_ELSE: ");
        break;
      case TOKEN_ENDIF:
        printf("TOKEN_ENDIF: ");
        break;
      case TOKEN_TEXT:
        printf("TOKEN_TEXT: ");
        break;
    }
  }

  free(buffer);
}
