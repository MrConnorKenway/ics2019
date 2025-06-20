#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void context_kload(PCB *, void *);
void context_uload(PCB *, const char *);

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite for the %dth time!", j);
    j ++;
    _yield();
  }
}

void init_proc() {
  switch_boot_pcb();
  context_uload(&pcb[0], "/bin/hello");
  context_uload(&pcb[1], "/bin/init");
}

_Context* schedule(_Context *prev) {
  current->cp = prev;
  current = (current == &pcb[0]) ? &pcb[1] : &pcb[0];
  return current->cp;
}
