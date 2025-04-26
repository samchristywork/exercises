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

Number add(Number a, Number b) {
  int maxLength = (a.length > b.length) ? a.length : b.length;
  Number result;
  result.length = maxLength + 1;
  result.digits = allocate_string(result.length + 1);
  result.digits[result.length] = '\0';

  int carry = 0;

  for (int i = 0; i < maxLength; i++) {
    int digitA = (i < a.length) ? a.digits[a.length - 1 - i] - '0' : 0;
    int digitB = (i < b.length) ? b.digits[b.length - 1 - i] - '0' : 0;

    int sum = digitA + digitB + carry;
    result.digits[result.length - 2 - i] = (sum % 10) + '0';
    carry = sum / 10;
  }

  if (carry > 0) {
    result.digits[0] = carry + '0';
  } else {
    result.length--;
    for (int i = 0; i < result.length; i++) {
      result.digits[i] = result.digits[i + 1];
    }
    result.digits = (char*)realloc(result.digits, result.length + 1);
  }

  return result;
}

int main() {
}
