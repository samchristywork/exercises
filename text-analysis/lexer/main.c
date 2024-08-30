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

const char *colorCodes[] = {"\033[0;31m", "\033[0;37m", "\033[0;33m",
                            "\033[0;32m", "\033[0;35m", "\033[0;34m",
                            "\033[0;34m", "\033[0;36m", "\033[0;38m"};

typedef struct {
  TokenType type;
  int start;
  int end;
} Token;

const char *keywords[] = {"auto",     "break",   "case",   "char",     "const",
                          "continue", "default", "do",     "double",   "else",
                          "enum",     "extern",  "float",  "for",      "goto",
                          "if",       "int",     "long",   "register", "return",
                          "short",    "signed",  "sizeof", "static",   "struct",
                          "switch",   "typedef", "union",  "unsigned", "void",
                          "volatile", "while"};

const size_t num_keywords = sizeof(keywords) / sizeof(keywords[0]);

const char *punctuators = "<>(){}[];=<>,+-*/:!&|.?%";

void printTokenData(Token *token, const char *source) {
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

void printHighlightedToken(Token *token, const char *source) {
  const char *color = colorCodes[token->type];
  printf("%s%.*s\033[0m", color, token->end - token->start,
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

Token *tokenize(const char *source, int *numTokens) {
  int tokensCapacity = 100;
  Token *tokens = malloc(tokensCapacity * sizeof(Token));

  int length = strlen(source);
  int index = 0;
  int start = 0;
  int currentIdx = 0;

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

      tokens[currentIdx].start = start;
      tokens[currentIdx].end = index;
      char value[100];
      snprintf(value, index - start + 1, "%s", source + start);
      tokens[currentIdx].type = isKeyword(value) ? KEYWORD : IDENTIFIER;

    } else if (isdigit(ch)) {
      do {
        index++;
      } while (isdigit(source[index]));

      tokens[currentIdx].start = start;
      tokens[currentIdx].end = index;
      tokens[currentIdx].type = CONSTANT;

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

      tokens[currentIdx].start = start;
      tokens[currentIdx].end = index;
      tokens[currentIdx].type = STRING_LITERAL;

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

      tokens[currentIdx].start = start;
      tokens[currentIdx].end = index;
      tokens[currentIdx].type = CHARACTER_CONSTANT;

    } else if (ch == '/' && source[index + 1] == '/') {
      index += 2;
      while (source[index] != '\n' && index < length) {
        index++;
      }

      tokens[currentIdx].start = start;
      tokens[currentIdx].end = index;
      tokens[currentIdx].type = SINGLE_LINE_COMMENT;

    } else if (ch == '/' && source[index + 1] == '*') {
      index += 2;
      while (index < length) {
        index++;
        if (source[index] == '*' && source[index + 1] == '/') {
          index += 2;
          break;
        }
      }

      tokens[currentIdx].start = start;
      tokens[currentIdx].end = index;
      tokens[currentIdx].type = MULTI_LINE_COMMENT;

    } else if (strchr(punctuators, ch) != NULL) {
      index++;
      tokens[currentIdx].start = start;
      tokens[currentIdx].end = index;
      tokens[currentIdx].type = PUNCTUATOR;
      // TODO: Handle compound punctuators separately
      if ((ch == '+' && source[index] == '+') || (source[index] == '=')) {
        index++;
        tokens[currentIdx].end = index;
      }

    } else {
      index++;
      tokens[currentIdx].start = start;
      tokens[currentIdx].end = index;
      tokens[currentIdx].type = UNKNOWN;
    }
    currentIdx++;
    *numTokens = currentIdx;
    if (currentIdx >= tokensCapacity) {
      tokensCapacity *= 2;
      tokens = realloc(tokens, tokensCapacity * sizeof(Token));
    }
  }

  return tokens;
}

void printUsage(const char *programName) {
  printf("Usage: %s [options] <filename>\n", programName);
  printf("Options:\n");
  printf("  -h, --help\t\tPrint this message\n");
  printf("  -i  --highlight\tPrint out highlighted source code\n");
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printUsage(argv[0]);
    return 1;
  }

  char *filename = NULL;
  bool highlight = false;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      printUsage(argv[0]);
      return 0;
    } else if (strcmp(argv[i], "-i") == 0 ||
               strcmp(argv[i], "--highlight") == 0) {
      highlight = true;
    } else {
      filename = argv[i];
    }
  }

  if (filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
      fprintf(stderr, "Could not open file %s\n", filename);
      return 1;
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *source = malloc(fsize + 1);
    fread(source, 1, fsize, fp);
    source[fsize] = '\0';

    fclose(fp);

    int numTokens = 0;
    Token *tokens = tokenize(source, &numTokens);

    if (highlight) {
      for (int i = 0; i < numTokens; i++) {
        if (i > 0) {
          Token *prev = &tokens[i - 1];
          Token *current = &tokens[i];
          fwrite(source + prev->end, 1, current->start - prev->end, stdout);
        }
        printHighlightedToken(&tokens[i], source);
      }
    } else {
      for (int i = 0; i < numTokens; i++) {
        printTokenData(&tokens[i], source);
      }
    }

    free(tokens);
    free(source);
  }
}
