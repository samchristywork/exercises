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

int main() {
}
