#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  rtl_push(&id_dest->val);

  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&id_dest->val);
  operand_write(id_dest, &id_dest->val);

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  TODO();

  print_asm("pusha");
}

make_EHelper(popa) {
  TODO();

  print_asm("popa");
}

make_EHelper(leave) {
  rtl_mv(&cpu.esp, &cpu.ebp);
  rtl_pop(&cpu.ebp);

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decinfo.isa.is_operand_size_16) {
    rtl_lr(&s0, R_EAX, 2);
    rtl_sext(&s0, &s0, 2);
    rtl_sari(&s0, &s0, 16);
    rtl_sr(R_EDX, &s0, 2);
  } else {
    rtl_lr(&s0, R_EAX, 4);
    rtl_sari(&s0, &s0, 31);
    rtl_sr(R_EDX, &s0, 4);
  }

  print_asm(decinfo.isa.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decinfo.isa.is_operand_size_16) {
    rtl_lr(&s0, R_EAX, 1);
    rtl_sext(&s0, &s0, 1);
    rtl_sr(R_EAX, &s0, 2);
  }
  else {
    rtl_lr(&s0, R_EAX, 2);
    rtl_sext(&s0, &s0, 2);
    rtl_sr(R_EAX, &s0, 4);
  }

  print_asm(decinfo.isa.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decinfo.isa.is_operand_size_16 ? 2 : 4;
#ifdef DEBUG
  snprintf(id_dest->str, OP_STR_SIZE, "%%%s", reg_name(id_dest->reg, id_dest->width));
#endif
  rtl_sext(&s0, &id_src->val, id_src->width);
  operand_write(id_dest, &s0);
  if (id_src->width == 1) {
    print_asm_template2(movsb);
  } else {
    print_asm_template2(movsw);
  }
}

make_EHelper(movzx) {
  id_dest->width = decinfo.isa.is_operand_size_16 ? 2 : 4;
#ifdef DEBUG
  snprintf(id_dest->str, OP_STR_SIZE, "%%%s", reg_name(id_dest->reg, id_dest->width));
#endif
  operand_write(id_dest, &id_src->val);
  if (id_src->width == 1) {
    print_asm_template2(movzb);
  } else {
    print_asm_template2(movzw);
  }
}

make_EHelper(lea) {
  operand_write(id_dest, &id_src->addr);
  print_asm_template2(lea);
}
