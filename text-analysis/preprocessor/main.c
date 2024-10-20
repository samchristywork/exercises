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
      symbols[i].name = strdup(name);
      symbols[i].value = strdup(value);
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
  char *subText = strdup(text);

  for (int i = 0; symbols[i].name != NULL; i++) {
    while (1) {
      char *pos = strstr(subText, symbols[i].name);
      if (pos == NULL) {
        break;
      }

      int posIndex = pos - subText;
      int nameLength = strlen(symbols[i].name);
      int valueLength = strlen(symbols[i].value);
      int newLength = strlen(subText) + valueLength - nameLength + 1;

      char *newText = malloc(newLength);
      strncpy(newText, subText, posIndex);
      newText[posIndex] = '\0'; // Ensure null termination
      strcat(newText, symbols[i].value);
      strcat(newText, subText + posIndex + nameLength);

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
  char *p = text;
  while (*p != ' ') p++;
  p++;
  while (*p != ' ') p++;
  p++;
  strcpy(value, p);
  return value;
}

void usage(char *name) {
  printf("Usage: %s file\n", name);
}

int main() {
  char text[1024];

  while (fgets(text, sizeof(text), stdin) != NULL) {
    text[strcspn(text, "\n")] = '\0';

    if (strlen(text) == 0) {
      continue;
    }

    char *subText = substituteSymbols(text);

    if (strncmp(subText, "#define", 7) == 0) {
      addSymbol(getName(subText), getValue(subText));
      free(subText);
      continue;
    }

    printf("%s\n", subText);

    free(subText);
  }

  printf("\nSymbols:\n");
  printSymbols();
}
