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
  CHARACTER_CONSTANT,
  PUNCTUATOR,
  UNKNOWN
} TokenType;

typedef struct {
  TokenType type;
  char value[MAX_TOKEN_LEN];
} Token;

const char *keywords[] = {"int", "for", "if", "else", "while", "return"};
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
  case CHARACTER_CONSTANT:
    type_str = "CHARACTER_CONSTANT";
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

    } else if (ch == '#') {
      while ((ch = fgetc(fp)) != EOF && ch != '\n') {
      }
      continue;

    } else if (isdigit(ch)) {
      do {
        currentToken.value[index++] = ch;
      } while (isdigit(ch = fgetc(fp)));

      ungetc(ch, fp);
      currentToken.value[index] = '\0';
      currentToken.type = CONSTANT;

    } else if (ch == '"') {
      currentToken.value[index++] = ch;
      while ((ch = fgetc(fp)) != EOF && ch != '"') {
        currentToken.value[index++] = ch;
      }
      if (ch == '"') {
        currentToken.value[index++] = ch;
      }
      currentToken.value[index] = '\0';
      currentToken.type = STRING_LITERAL;

    } else if (ch == '\'') {
      currentToken.value[index++] = ch;
      while ((ch = fgetc(fp)) != EOF && ch != '\'') {
        currentToken.value[index++] = ch;
      }
      if (ch == '\'') {
        currentToken.value[index++] = ch;
      }
      currentToken.value[index] = '\0';
      currentToken.type = CHARACTER_CONSTANT;

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

void printUsage(const char *programName) {
  printf("Usage: %s <filename>\n", programName);
  printf(
      "Tokenizes the given C source file and prints tokens to the console.\n");
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printUsage(argv[0]);
    return 1;
  }

  const char *fileName = argv[1];
  tokenize(fileName);
}
