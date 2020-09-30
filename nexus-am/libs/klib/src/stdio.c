#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  return 0;
}

static void printnum(void (*putch)(int, void *), void *putbuf, unsigned long long num, unsigned base) {
  if (num >= base) {
    printnum(putch, putbuf, num / base, base);
  }

  putch("0123456789abcdef"[num % base], putbuf);
}

void vprintfmt(void (*putch)(int, void *), void *putbuf, const char *fmt, va_list ap) {
  char c;
  const char *ptr;
  unsigned long long num;

  while (1) {
    while ((c = *fmt++) != '%') {
      if (c == 0) {
        return;
      }
      putch(c, putbuf);
    }

    switch (c = *fmt++) {
      case 's': {
        if ((ptr = va_arg(ap, char*)) == NULL) {
          ptr = "(null)";
        }
        for (; (c = *ptr++) != 0;) {
          putch(c, putbuf);
        }
        break;
      }

      case 'd': {
        num = va_arg(ap, int);
        if (num < 0) {
          putch('-', putbuf);
          num = -(long long) num;
        }
        printnum(putch, putbuf, num, 10);
        break;
      }
    }
  }
}

struct sprintbuf {
  char *buf;
  int cnt;
};

void sprintputch(int c, struct sprintbuf *b) {
  ++b->cnt;
  *b->buf++ = c;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  assert(out != NULL);

  struct sprintbuf b = {out, 0};
  vprintfmt((void *) sprintputch, &b, fmt, ap);
  *b.buf = 0;
  return b.cnt;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  int rc;

  va_start(ap, fmt);
  rc = vsprintf(out, fmt, ap);
  va_end(ap);

  return rc;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  return 0;
}

#endif
