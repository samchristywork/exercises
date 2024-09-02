#include <stdbool.h>
#include <stdio.h>

bool matchChar(char pattern, char text) {
  return pattern == '.' || pattern == text;
}

bool matchHere(const char *pattern, const char *text) {
  if (*pattern == '\0')
    return true;

  if (*(pattern + 1) == '*') {
    do {
      if (matchHere(pattern + 2, text)) {
        return true;
      }
    } while (*text != '\0' && matchChar(*pattern, *text++));
    return false;
  }

  if (*text != '\0' && matchChar(*pattern, *text)) {
    return matchHere(pattern + 1, text + 1);
  }

  return false;
}

bool match(const char *pattern, const char *text) {
  if (*pattern == '^') {
    return matchHere(pattern + 1, text);
  }

  do {
    if (matchHere(pattern, text)) {
      return true;
    }
  } while (*text++ != '\0');

  return false;
}

#ifdef TEST
#include <regex.h>

void testPattern(const char *pattern, const char *text) {
  regex_t re;
  regcomp(&re, pattern, REG_EXTENDED);
  bool expected = regexec(&re, text, 0, NULL, 0) == 0;
  bool actual = match(pattern, text);
  if (expected != actual) {
    printf("pattern: %s, text: %s, expected: %s, actual: %s\n", pattern, text,
           expected ? "true" : "false", actual ? "true" : "false");
  }
}

int main() {
  testPattern("a", "This is a sentence.");
  testPattern("a.*e", "This is a sentence.");
  testPattern("^T", "This is a sentence.");
  testPattern("o$", "Hello");
}
#else
int main() {
}
#endif
