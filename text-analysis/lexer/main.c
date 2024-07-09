#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAX_TOKEN_LEN 100

typedef enum {
  KEYWORD,
  IDENTIFIER,
  CONSTANT,
  STRING_LITERAL,
  PUNCTUATOR,
  UNKNOWN
} TokenType;

typedef struct {
  TokenType type;
  char value[MAX_TOKEN_LEN];
} Token;

const char *keywords[] = {"int",   "for",    "if",     "else",
                          "while", "return"};
const size_t num_keywords = sizeof(keywords) / sizeof(keywords[0]);

const char *punctuators = "<>(){};=<>++";

void printToken(Token *token) {
  const char *type_str = "UNKNOWN";
  switch (token->type) {
  case KEYWORD:
    type_str = "KEYWORD";
    break;
  case IDENTIFIER:
    type_str = "IDENTIFIER";
    break;
  case CONSTANT:
    type_str = "CONSTANT";
    break;
  case STRING_LITERAL:
    type_str = "STRING_LITERAL";
    break;
  case PUNCTUATOR:
    type_str = "PUNCTUATOR";
    break;
  default:
    break;
  }
  printf("%s\t%s\n", type_str, token->value);
}

int isKeyword(const char *value) {
  for (size_t i = 0; i < num_keywords; i++) {
    if (strcmp(value, keywords[i]) == 0) {
      return true;
    }
  }
  return false;
}

void tokenize(const char *fileName) {
  FILE *fp = fopen(fileName, "r");
  if (!fp) {
    fprintf(stderr, "Could not open file %s\n", fileName);
    return;
  }

  char ch;
  Token currentToken;
  int index = 0;

  while ((ch = fgetc(fp)) != EOF) {
    if (isspace(ch)) {
      continue;
    }

    index = 0;
    if (isalpha(ch) || ch == '_') {
      do {
        currentToken.value[index++] = ch;
      } while (isalnum(ch = fgetc(fp)) || ch == '_');

      ungetc(ch, fp);
      currentToken.value[index] = '\0';
      currentToken.type = isKeyword(currentToken.value) ? KEYWORD : IDENTIFIER;

    } else if (isdigit(ch)) {
      do {
        currentToken.value[index++] = ch;
      } while (isdigit(ch = fgetc(fp)));

      ungetc(ch, fp);
      currentToken.value[index] = '\0';
      currentToken.type = CONSTANT;

    } else if (strchr(punctuators, ch) != NULL) {
      currentToken.value[0] = ch;
      currentToken.value[1] = '\0';
      currentToken.type = PUNCTUATOR;

      if ((ch == '+' && (ch = fgetc(fp)) == '+') || (ch = fgetc(fp)) == '=') {
        strcat(currentToken.value, "=");
      } else {
        ungetc(ch, fp);
      }

    } else {
      currentToken.value[0] = ch;
      currentToken.value[1] = '\0';
      currentToken.type = UNKNOWN;
    }
    printToken(&currentToken);
  }
  fclose(fp);
}

int main() {
  const char *fileName = "sample.c";
  tokenize(fileName);
}
