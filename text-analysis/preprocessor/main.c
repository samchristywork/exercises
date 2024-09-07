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
