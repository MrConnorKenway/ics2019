#include "memory.h"
#include "proc.h"

static void *pf = NULL;

void* new_page(size_t nr_page) {
  void *p = pf;
  pf += PGSIZE * nr_page;
  assert(pf < (void *)_heap.end);
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

int _map(_AddressSpace *, void *, void *, int);

/* The brk() system call handler. */
int mm_brk(uintptr_t brk, intptr_t increment) {
  uintptr_t va_end = PGROUNDUP(brk + increment);
  for (uintptr_t va = (current->max_brk == 0) ? PGROUNDUP(brk)
                                              : current->max_brk;
       va < va_end; va += PGSIZE) {
    _map(&current->as, (void *)va, new_page(1), 0);
  }
  current->max_brk = va_end;
  return 0;
}

void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);

  _vme_init(new_page, free_page);
}
