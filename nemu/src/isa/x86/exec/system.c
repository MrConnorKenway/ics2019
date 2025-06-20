#include "cpu/exec.h"

make_EHelper(lidt) {
  rtl_lm(&s0, &id_dest->addr, 2);
  rtl_mv(&cpu.IDTR.limit, &s0);
  rtl_addi(&s0, &id_dest->addr, 2);
  rtl_lm(&s1, &s0, 4);
  switch (id_dest->width) {
    case 2: {
      rtl_andi(&s1, &s1, 0xffffff);
      break;
    }
    case 4: {
      break;
    }
    default:assert(0);
  }
  rtl_mv(&cpu.IDTR.base, &s1);

  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  switch (id_dest->reg) {
    case 0:
      rtl_mv((rtlreg_t*)&cpu.cr0, &id_src->val);
      break;

    case 3:
      rtl_mv((rtlreg_t*)&cpu.cr3, &id_src->val);
      break;

    default:assert(0);
  }

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  switch (id_src->reg) {
    case 0:
      rtl_mv(&id_src->val, (rtlreg_t*)&cpu.cr0);
      break;

    case 3:
      rtl_mv(&id_src->val, (rtlreg_t*)&cpu.cr3);
      break;

    default:assert(0);
  }
  operand_write(id_dest, &id_src->val);

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

  difftest_skip_ref();
}

make_EHelper(int) {
  raise_intr(id_dest->val, decinfo.seq_pc);

  print_asm("int %s", id_dest->str);

  difftest_skip_dut(1, 2);
}

make_EHelper(iret) {
  rtl_pop(&decinfo.jmp_pc);
  rtl_j(decinfo.jmp_pc);
  rtl_pop(&cpu.cs);
  rtl_pop(&cpu.eflags);

  print_asm("iret");
}

uint32_t pio_read_l(ioaddr_t);
uint32_t pio_read_w(ioaddr_t);
uint32_t pio_read_b(ioaddr_t);
void pio_write_l(ioaddr_t, uint32_t);
void pio_write_w(ioaddr_t, uint32_t);
void pio_write_b(ioaddr_t, uint32_t);

make_EHelper(in) {
  switch (id_dest->width) {
    case 4: {
      id_dest->val = pio_read_l(id_src->val);
      break;
    }
    case 1: {
      id_dest->val = pio_read_b(id_src->val);
      break;
    }
    case 2: {
      id_dest->val = pio_read_w(id_src->val);
      break;
    }
    default:assert(0);
  }

  operand_write(id_dest, &id_dest->val);

  print_asm_template2(in);
}

make_EHelper(out) {
  switch (id_src->width) {
    case 4: {
      pio_write_l(id_dest->val, id_src->val);
      break;
    }
    case 1: {
      pio_write_b(id_dest->val, id_src->val);
      break;
    }
    case 2: {
      pio_write_w(id_dest->val, id_src->val);
      break;
    }
    default:assert(0);
  }

  print_asm_template2(out);
}
