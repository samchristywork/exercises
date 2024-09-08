#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct symbol {
  char *name;
  char *value;
} symbol;

symbol symbols[100];

void addSymbol(char *name, char *value) {
  printf("\t\tAdding symbol: %s = %s\n", name, value);
  for (int i = 0; i < 100; i++) {
    if (symbols[i].name == NULL) {
      symbols[i].name = name;
      symbols[i].value = value;
      return;
    }
  }
}

void printSymbols() {
  for (int i = 0; i < 100; i++) {
    if (symbols[i].name == NULL) {
      return;
    }
    printf("%s = %s\n", symbols[i].name, symbols[i].value);
  }
}

char *substituteSymbols(char *text) {
  char *subText = malloc(1024);
  strcpy(subText, text);

  for (int i = 0; i < 100; i++) {
    if (symbols[i].name == NULL) {
      return subText;
    }

    char *pos = strstr(subText, symbols[i].name);
    if (pos != NULL) {
      char *newText = malloc(1024);
      strcpy(newText, subText);
      pos = strstr(newText, symbols[i].name);
      int posIndex = pos - newText;
      int nameLength = strlen(symbols[i].name);
      int valueLength = strlen(symbols[i].value);
      int newLength = strlen(newText) + valueLength - nameLength;
      char *temp = malloc(newLength);
      strncpy(temp, newText, posIndex);
      strcat(temp, symbols[i].value);
      strcat(temp, newText + posIndex + nameLength);
      strcpy(newText, temp);
      free(temp);
      free(subText);
      subText = newText;
    }
  }

  return subText;
}

char *getName(char *text) {
  char *name = malloc(1024);
  sscanf(text, "#define %s", name);
  return name;
}

char *getValue(char *text) {
  char *value = malloc(1024);
  char *p=text;
  while (*p != ' ') {
    p++;
  }
  p++;
  while (*p != ' ') {
    p++;
  }
  p++;
  strcpy(value, p);
  return value;
}

int main(int argc, char *argv[]) {
}
