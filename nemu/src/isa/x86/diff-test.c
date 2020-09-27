#include "nemu.h"
#include "monitor/diff-test.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  for (int i = R_EAX; i <= R_EDI; ++i) {
    if (ref_r->gpr[i]._32 != reg_l(i)) {
      return false;
    }
  }

  if ((ref_r->eflags & flag_mask) != (cpu.eflags & flag_mask)) {
    return false;
  }

  return ref_r->pc == cpu.pc;
}

void isa_difftest_attach(void) {
}
