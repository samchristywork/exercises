#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int length;
  char *digits;
} Number;

char *allocate_string(int length) {
  char *str = (char *)malloc(length * sizeof(char));
  if (str == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }
  return str;
}

Number create_number(const char *str) {
  Number number;
  number.length = strlen(str);
  number.digits = allocate_string(number.length + 1);
  strcpy(number.digits, str);
  return number;
}

void free_number(Number number) {
  free(number.digits);
  number.digits = NULL;
  number.length = 0;
}

int main() {
}
