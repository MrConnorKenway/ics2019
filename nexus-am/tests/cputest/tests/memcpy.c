#include "trap.h"

int main() {
  char s1[] = "hello", s2[] = "world";
  memcpy(s2, s1, strlen(s1));
  nemu_assert(strcmp(s2, "hello") == 0);
  return 0;
}
