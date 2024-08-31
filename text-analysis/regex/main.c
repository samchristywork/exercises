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

  }
}

int main() {
}
