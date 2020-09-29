#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t n;
  for (n = 0; *s != 0; ++s) {
    ++n;
  }
  return n;
}

char *strcpy(char *dst, const char *src) {
  char *ret = dst;
  while ((*dst++ = *src++) != 0);
  return ret;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i;
  char *ret;

  ret = dst;
  for (i = 0; i < n; i++) {
    *dst++ = *src;
    // If strlen(src) < size, null-pad 'dst' out to 'size' chars
    if (*src != '\0') {
      ++src;
    }
  }
  return ret;
}

char *strcat(char *dst, const char *src) {
  size_t len = strlen(dst);
  strcpy(dst + len, src);
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
    ++s1;
    ++s2;
  }
  return (int) ((unsigned char) *s1 - (unsigned char) *s2);
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while (n > 0 && *s1 && *s1 == *s2) {
    --n;
    ++s1;
    ++s2;
  }
  if (n == 0) {
    return 0;
  } else {
    return (int) ((unsigned char) *s1 - (unsigned char) *s2);
  }
}

void *memset(void *v, int c, size_t n) {
  char *p;
  int m;

  p = v;
  m = n;
  while (--m >= 0) {
    *p++ = c;
  }

  return v;
}

void *memcpy(void *out, const void *in, size_t n) {
  const char *s;
  char *d;

  s = in;
  d = out;
  if (s < d && s + n > d) {
    s += n;
    d += n;
    while (n-- > 0)
      *--d = *--s;
  } else {
    while (n-- > 0) {
      *d++ = *s++;
    }
  }

  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const uint8_t *v1 = (const uint8_t *) s1;
  const uint8_t *v2 = (const uint8_t *) s2;

  while (n-- > 0) {
    if (*v1 < *v2) {
      return -1;
    } else if (*v1 > *v2) {
      return 1;
    }
    ++v1;
    ++v2;
  }

  return 0;
}

#endif
