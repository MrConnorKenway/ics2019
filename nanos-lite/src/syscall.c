#include "syscall.h"

#include "common.h"
#include "fs.h"
#include "proc.h"

int mm_brk(uintptr_t, intptr_t);
void context_uload(PCB *, const char *);

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0]) {
    case SYS_exit: _halt(a[1]);
    case SYS_yield: _yield(); c->GPRx = 0; break;
    case SYS_open: {
      c->GPRx = fs_open((const char *)a[1], a[2], a[3]);
      break;
    }
    case SYS_read: {
      c->GPRx = fs_read(a[1], (void *)a[2], a[3]);
      break;
    }
    case SYS_write: {
      c->GPRx = fs_write(a[1], (void *)a[2], a[3]);
      break;
    }
    case SYS_close: {
      c->GPRx = fs_close(a[1]);
      break;
    }
    case SYS_lseek: {
      c->GPRx = fs_lseek(a[1], a[2], a[3]);
      break;
    }
    case SYS_brk: {
      c->GPRx = mm_brk(a[1], a[2]);
      break;
    }
    case SYS_execve: {
      context_uload(current, (const char *)a[1]);
      return current->cp;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
