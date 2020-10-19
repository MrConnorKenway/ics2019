#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(movs) {
  rtl_lr(&s0, R_ESI, 4);
  rtl_lm(&id_src->val, &s0, id_dest->width);

  rtl_lr(&s1, R_EDI, 4);
  rtl_sm(&s1, &id_src->val, id_dest->width);

  if (cpu.DF == 0) {
    rtl_addi(&s0, &s0, id_dest->width);
    rtl_addi(&s1, &s1, id_dest->width);
  } else {
    rtl_subi(&s0, &s0, id_dest->width);
    rtl_subi(&s1, &s1, id_dest->width);
  }

  rtl_sr(R_ESI, &s0, 4);
  rtl_sr(R_EDI, &s1, 4);

  switch (id_dest->width) {
    case 1: {
      print_asm("movsb");
      break;
    }
    case 2: {
      print_asm("movsw");
      break;
    }
    case 4: {
      print_asm("movsd");
      break;
    }
    default:assert(0);
  }
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
  rtl_mv(&s0, &cpu.esp);
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&s0);
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);

  print_asm("pusha");
}

make_EHelper(popa) {
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  rtl_pop(&s0);
  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);

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
