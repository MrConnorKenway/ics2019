#include "common.h"
#include "syscall.h"

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0]) {
    case SYS_exit: _halt(a[1]);
    case SYS_yield: _yield(); c->GPRx = 0; break;
    case SYS_write: {
      int fd = a[1];
      size_t count = a[3];
      if (fd == 1 || fd == 2) {
        uint8_t *buf = (uint8_t *) a[2];
        for (size_t i = 0; i < count; ++i) {
          _putc(((uint8_t *) buf)[i]);
        }
        c->GPRx = count;
      } else {
        panic("Unsupported fd %d", fd);
      }
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
