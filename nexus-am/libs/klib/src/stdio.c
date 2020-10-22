#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static void printnum(void (*putch)(int, void *),
                     void *putbuf,
                     unsigned long long num,
                     unsigned base,
                     int width,
                     int padc) {
  if (num >= base) {
    printnum(putch, putbuf, num / base, base, width - 1, padc);
  } else {
    while (--width > 0) {
      putch(padc, putbuf);
    }
  }

  putch("0123456789abcdef"[num % base], putbuf);
}

void vprintfmt(void (*putch)(int, void *), void *putbuf, const char *fmt, va_list ap) {
  char c;
  const char *ptr;
  unsigned long long num;
  char padc;
  int base, width, precision;

  while (1) {
    while ((c = *fmt++) != '%') {
      if (c == 0) {
        return;
      }
      putch(c, putbuf);
    }

    padc = ' ';
    width = -1;
    precision = -1;

    reswitch:
    switch (c = *fmt++) {
      // flag to pad on the right
      case '-': {
        padc = '-';
        goto reswitch;
      }

      // flag to pad with 0's instead of spaces
      case '0': {
        padc = '0';
        goto reswitch;
      }

      // width field
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': {
        for (precision = 0;; ++fmt) {
          precision = precision * 10 + c - '0';
          c = *fmt;
          if (c < '0' || c > '9') {
            break;
          }
        }
        if (width < 0) {
          width = precision;
          precision = -1;
        }
        goto reswitch;
      }

      case 'c': {
        putch(va_arg(ap, int), putbuf);
        break;
      }

      case 's': {
        if ((ptr = va_arg(ap, char*)) == NULL) {
          ptr = "(null)";
        }
        if (width > 0 && padc != '-') {
          for (width -= strnlen(ptr, precision); width > 0; --width) {
            putch(padc, putbuf);
          }
        }
        for (; (c = *ptr++) != 0 && (precision < 0 || --precision >= 0); --width) {
          putch(c, putbuf);
        }
        for (; width > 0; --width) {
          putch(' ', putbuf);
        }
        break;
      }

      case 'd': {
        num = va_arg(ap, int);
        if ((long long) num < 0) {
          putch('-', putbuf);
          num = -(long long) num;
        }
        base = 10;
        goto number;
      }

      case 'p': {
        putch('0', putbuf);
        putch('x', putbuf);
        num = (unsigned long long) (uintptr_t) va_arg(ap, void *);
        base = 16;
        goto number;
      }

      case 'x': {
        num = va_arg(ap, unsigned int);
        base = 16;
        goto number;
      }

      number:
        printnum(putch, putbuf, num, base, width, padc);
        break;
    }
  }
}

void putchar(int c, int *cnt) {
  _putc(c);
  ++cnt;
}

int printf(const char *fmt, ...) {
  va_list ap;
  int rc;

  va_start(ap, fmt);
  vprintfmt((void *) putchar, &rc, fmt, ap);
  va_end(ap);

  return rc;
}

struct sprintbuf {
  char *buf;
  char *ebuf;
  int cnt;
};

void sprintputch(int c, struct sprintbuf *b) {
  ++b->cnt;
  if (b->ebuf == NULL || b->buf < b->ebuf) {
    *b->buf++ = c;
  }
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  assert(out != NULL);

  struct sprintbuf b = {out, NULL, 0};
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

int vsnprintf(char *buf, size_t n, const char *fmt, va_list ap) {
  struct sprintbuf b = {buf, buf + n - 1, 0};

  if (buf == NULL || n < 1) return -1;

  // print the string to the buffer
  vprintfmt((void *)sprintputch, &b, fmt, ap);

  // null terminate the buffer
  *b.buf = '\0';

  return b.cnt;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  int rc;

  va_start(ap, fmt);
  rc = vsnprintf(out, n, fmt, ap);
  va_end(ap);

  return rc;
}

#endif
