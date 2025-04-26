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

Number generate_number(int n, int d) {
  Number number;
  number.length = n;
  number.digits = allocate_string(n + 1);
  for (int i = 0; i < n; i++) {
    number.digits[i] = '0'+ d;
  }
  number.digits[n] = '\0';
  return number;
}

int main() {
  {
    Number a = create_number("12345678901234567890");
    Number b = create_number("98765432109876543210");

    Number result = add(a, b);

    printf("Result: %s\n", result.digits);

    free_number(a);
    free_number(b);
    free_number(result);
  }

  {
    Number a = generate_number(999, 9);
    Number b = create_number("1");

    Number result = add(a, b);

    printf("Result: %s\n", result.digits);

    free_number(a);
    free_number(b);
    free_number(result);
  }

  return 0;
}
