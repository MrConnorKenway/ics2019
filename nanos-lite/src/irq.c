#include "common.h"
#include "syscall.h"

_Context* do_syscall(_Context *);
_Context* schedule(_Context *);

static _Context* do_event(_Event e, _Context* c) {
  switch (e.event) {
    case _EVENT_SYSCALL: {
      return do_syscall(c);
    }
    case _EVENT_IRQ_TIMER: {
      Log("Time slice is up");
      return schedule(c);
    }
    case _EVENT_YIELD: {
      return schedule(c);
    }
    default: panic("Unhandled event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  _cte_init(do_event);
}
