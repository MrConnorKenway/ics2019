#include "rtl/rtl.h"

void raise_intr(uint32_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  rtl_push(&cpu.eflags);
  rtl_push(&cpu.cs);
  rtl_li(&s0, ret_addr);
  rtl_push(&s0);
  rtl_addi(&s0, &cpu.IDTR.limit, 1);
  rtl_div_qi(&s0, &s0, 256);
  rtl_mul_loi(&s0, &s0, NO);
  rtl_add(&s0, &s0, &cpu.IDTR.base);
  rtl_lm(&s1, &s0, 2);
  rtl_addi(&s0, &s0, 6);
  rtl_lm(&s2, &s0, 2);
  rtl_shli(&s2, &s2, 16);
  rtl_or(&s0, &s1, &s2);
  rtl_j(s0);
}

bool isa_query_intr(void) {
  return false;
}
