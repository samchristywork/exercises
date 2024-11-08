#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

char *replaceAll(const char *str, const char *from, const char *to) {
  size_t fromLen = strlen(from);
  size_t toLen = strlen(to);

  if (fromLen == 0) {
    return strdup(str);
  }

  size_t resultSize = strlen(str) + 1;
  char *result = malloc(resultSize);
  if (!result) return NULL;
  strcpy(result, "");

  const char *p = str;
  while ((p = strstr(p, from)) != NULL) {
    resultSize += toLen - fromLen;
    result = realloc(result, resultSize);
    if (!result) return NULL;

    strncat(result, str, p - str);
    strcat(result, to);
    str = p + fromLen;
    p = str;
  }
  strcat(result, str);

  return result;
}

char *applyDefines(char *tokenBody, size_t length, Token *tokens, int tokenCount) {
  size_t maxLength = length;
  for (int i = 0; i < tokenCount; i++) {
    if (tokens[i].type == TOKEN_DEFINE) {
      maxLength += strlen(tokens[i].defineDetails.value) - (tokens[i].defineDetails.key ? strlen(tokens[i].defineDetails.key) : 0);
    }
  }

  char *result = malloc(maxLength + 1);
  if (!result) {
    fprintf(stderr, "Memory allocation failed\n");
    return NULL;
  }
  result[0] = '\0';

  char *copy = strndup(tokenBody, length);
  char *cursor = copy;

  while (*cursor != '\0') {
    char *tokenStart = cursor;
    while (*cursor != ' ' && *cursor != '\t' && *cursor != '\n' && *cursor != '(' && *cursor != '\0') {
      cursor++;
    }

    char *key = strndup(tokenStart, cursor - tokenStart);
    bool substituted = false;

    for (int i = 0; i < tokenCount; i++) {
      if (tokens[i].type == TOKEN_DEFINE && strcmp(tokens[i].defineDetails.key, key) == 0) {
        if (tokens[i].defineDetails.parameters == NULL) {
          strcat(result, tokens[i].defineDetails.value);
          substituted = true;
        } else {
          if (*cursor == '(') {
            cursor++;
            char *argsStart = cursor;
            int brackets = 1;
            while (brackets > 0 && *cursor != '\0') {
              if (*cursor == '(') {
                brackets++;
              } else if (*cursor == ')') {
                brackets--;
              }
              cursor++;
            }

            if (brackets == 0) {
              char *args = strndup(argsStart, (cursor - 1) - argsStart);
              char *params = strdup(tokens[i].defineDetails.parameters);

              char *arg, *argPtrs[10];
              int argCount = 0;
              char *param = strtok(params, ",");
              char *argPiece = strtok(args, ",");

              while (argPiece) {
                argPtrs[argCount++] = strdup(argPiece);
                argPiece = strtok(NULL, ",");
              }

              char *valueCopy = strdup(tokens[i].defineDetails.value);
              for (int j = 0; j < argCount; ++j) {
                if (param) {
                  char *substitutedValue = replaceAll(valueCopy, param, argPtrs[j]);
                  free(valueCopy);
                  valueCopy = substitutedValue;
                  param = strtok(NULL, ",");
                }
              }
              strcat(result, valueCopy);

              free(valueCopy);
              free(args);
              free(params);
              for (int j = 0; j < argCount; ++j) {
                free(argPtrs[j]);
              }
              substituted = true;
            }
          }
        }
        free(key);
        if (substituted) {
          break;
        }
      }
    }

    if (!substituted) {
      strncat(result, key, cursor - key);
      strcat(result, " ");
    }

    if (*cursor != '\0') {
      cursor++;
    }
  }

  free(copy);

  size_t res_len = strlen(result);
  if (res_len > 0 && result[res_len - 1] == ' ') {
    result[res_len - 1] = '\0';
  }

  return result;
}

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

bool evaluateIfCondition(char *tokenBody, size_t length, Token *tokens, int tokenCount) {
  char *tokensCopy = strndup(tokenBody, length);
  char *token = strtok(tokensCopy, " \t\n");

  char *left = NULL;
  char *right = NULL;
  char *op = NULL;

  while (token) {
    if (left == NULL) {
      left = token;
    } else if (op == NULL) {
      op = token;
    } else if (right == NULL) {
      right = token;
    }

    token = strtok(NULL, " \t\n");
  }

  if (left == NULL || op == NULL || right == NULL) {
    fprintf(stderr, "Invalid if condition\n");
    return false;
  }

  int leftValue = 0;
  int rightValue = 0;

  if (strncmp("<", op, 1) == 0) {
    leftValue = atoi(left);
    rightValue = atoi(right);
    return leftValue < rightValue;
  } else if (strncmp(">", op, 1) == 0) {
    leftValue = atoi(left);
    rightValue = atoi(right);
    return leftValue > rightValue;
  } else if (strncmp("==", op, 2) == 0) {
    return strcmp(left, right) == 0;
  } else if (strncmp("!=", op, 2) == 0) {
    return strcmp(left, right) != 0;
  } else {
    fprintf(stderr, "Invalid operator\n");
    return false;
  }

  return false;
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
    char *tokenBody = strndup(token.start, token.length);
    char *substitutedBody = applyDefines(tokenBody, token.length, tokens, tokenCount);

    switch (token.type) {
      case TOKEN_DEFINE:
        printDefineDetails(&token.defineDetails);
        break;
      case TOKEN_IF:
        if (evaluateIfCondition(substitutedBody+4, strlen(substitutedBody)-4, tokens, tokenCount)) {
          i++;
          char *tokenBody = strndup(tokens[i].start, tokens[i].length);
          char *substitutedBody = applyDefines(tokenBody, tokens[i].length, tokens, tokenCount);
          printf("%s\n", substitutedBody);
          free(tokenBody);
          free(substitutedBody);
          while (tokens[i].type != TOKEN_ENDIF) {
            i++;
          }
        } else {
          i++;
          i++;
          printf("%s\n", substitutedBody);
        }
        break;
      case TOKEN_ELSE:
        break;
      case TOKEN_ENDIF:
        break;
      case TOKEN_TEXT:
        printf("%s\n", substitutedBody);
        break;
    }

    free(tokenBody);
    free(substitutedBody);
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
