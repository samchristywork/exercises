#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  KEYWORD,
  IDENTIFIER,
  CONSTANT,
  STRING_LITERAL,
  CHARACTER_CONSTANT,
  SINGLE_LINE_COMMENT,
  MULTI_LINE_COMMENT,
  PUNCTUATOR,
  UNKNOWN
} TokenType;

typedef struct {
  TokenType type;
  int start;
  int end;
} Token;

const char *keywords[] = {"int", "for", "if", "else", "while", "return"};
const size_t num_keywords = sizeof(keywords) / sizeof(keywords[0]);

const char *punctuators = "<>(){}[];=<>,+-*/:!&|.?%";

void printToken(Token *token, const char *source) {
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
  case SINGLE_LINE_COMMENT:
    type_str = "SINGLE_LINE_COMMENT";
    break;
  case MULTI_LINE_COMMENT:
    type_str = "MULTI_LINE_COMMENT";
    break;
  case PUNCTUATOR:
    type_str = "PUNCTUATOR";
    break;
  default:
    break;
  }
  printf("%-20s%.*s\n", type_str, token->end - token->start,
         source + token->start);
}

int isKeyword(const char *value) {
  for (size_t i = 0; i < num_keywords; i++) {
    if (strcmp(value, keywords[i]) == 0) {
      return true;
    }
  }
  return false;
}

void tokenize(const char *source) {
  int length = strlen(source);
  Token currentToken;
  int index = 0;
  int start = 0;

  while (index < length) {
    char ch = source[index];

    if (isspace(ch)) {
      index++;
      continue;
    }

    start = index;
    if (isalpha(ch) || ch == '_') {
      do {
        index++;
      } while (isalnum(source[index]) || source[index] == '_');

      currentToken.start = start;
      currentToken.end = index;
      char value[100];
      snprintf(value, index - start + 1, "%s", source + start);
      currentToken.type = isKeyword(value) ? KEYWORD : IDENTIFIER;

    } else if (isdigit(ch)) {
      do {
        index++;
      } while (isdigit(source[index]));

      currentToken.start = start;
      currentToken.end = index;
      currentToken.type = CONSTANT;

    } else if (ch == '"') {
      index++;
      while (source[index] != '"' && index < length) {
        if (source[index] == '\\') {
          index++;
        }
        index++;
      }
      if (source[index] == '"') {
        index++;
      }

      currentToken.start = start;
      currentToken.end = index;
      currentToken.type = STRING_LITERAL;

    } else if (ch == '\'') {
      index++;
      while (source[index] != '\'' && index < length) {
        if (source[index] == '\\') {
          index++;
        }
        index++;
      }
      if (source[index] == '\'') {
        index++;
      }

      currentToken.start = start;
      currentToken.end = index;
      currentToken.type = CHARACTER_CONSTANT;

    } else if (ch == '/' && source[index + 1] == '/') {
      index += 2;
      while (source[index] != '\n' && index < length) {
        index++;
      }

      currentToken.start = start;
      currentToken.end = index;
      currentToken.type = SINGLE_LINE_COMMENT;

    } else if (ch == '/' && source[index + 1] == '*') {
      index += 2;
      while (index < length) {
        index++;
        if (source[index] == '*' && source[index + 1] == '/') {
          index += 2;
          break;
        }
      }

      currentToken.start = start;
      currentToken.end = index;
      currentToken.type = MULTI_LINE_COMMENT;

    } else if (strchr(punctuators, ch) != NULL) {
      index++;
      currentToken.start = start;
      currentToken.end = index;
      currentToken.type = PUNCTUATOR;
      // TODO: Handle compound punctuators separately
      if ((ch == '+' && source[index] == '+') || (source[index] == '=')) {
        index++;
        currentToken.end = index;
      }

    } else {
      index++;
      currentToken.start = start;
      currentToken.end = index;
      currentToken.type = UNKNOWN;
    }
    printToken(&currentToken, source);
  }
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

  FILE *fp = fopen(fileName, "r");
  if (!fp) {
    fprintf(stderr, "Could not open file %s\n", fileName);
    return 1;
  }

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char *source = malloc(fsize + 1);
  fread(source, 1, fsize, fp);
  source[fsize] = '\0';

  fclose(fp);

  tokenize(source);

  free(source);
}
