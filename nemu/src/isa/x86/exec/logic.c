#include "cpu/exec.h"
#include "cc.h"

make_EHelper(test) {
  rtl_li(&s0, 0);
  rtl_set_CF(&s0);
  rtl_set_OF(&s0);

  rtl_and(&s0, &id_dest->val, &id_src->val);

  rtl_update_ZFSF(&s0, id_dest->width);

  print_asm_template2(test);
}

make_EHelper(and) {
  rtl_li(&s0, 0);
  rtl_set_CF(&s0);
  rtl_set_OF(&s0);

  rtl_and(&s0, &id_dest->val, &id_src->val);

  rtl_update_ZFSF(&s0, id_dest->width);

  operand_write(id_dest, &s0);

  print_asm_template2(and);
}

make_EHelper(xor) {
  rtl_li(&s0, 0);
  rtl_set_CF(&s0);
  rtl_set_OF(&s0);

  rtl_xor(&s0, &id_dest->val, &id_src->val);

  rtl_update_ZFSF(&s0, id_dest->width);

  operand_write(id_dest, &s0);

  print_asm_template2(xor);
}

make_EHelper(or) {
  rtl_li(&s0, 0);
  rtl_set_CF(&s0);
  rtl_set_OF(&s0);

  rtl_or(&s0, &id_dest->val, &id_src->val);

  rtl_update_ZFSF(&s0, id_dest->width);

  operand_write(id_dest, &s0);

  print_asm_template2(or);
}

make_EHelper(sar) {
  rtl_get_SF(&s2);
  rtl_shli(&s2, &s2, id_dest->width * 8 - 1);
  rtl_get_ZF(&s1);
  rtl_xori(&s1, &s1, 1);
  rtl_or(&s2, &s2, &s1);

  rtl_sext(&s0, &id_dest->val, id_dest->width);
  rtl_andi(&id_src->val, &id_src->val, 0x1f);
  rtl_sar(&s0, &s0, &id_src->val);

  // unnecessary to update CF and OF in NEMU
  flag_mask &= (~cf_mask) & (~of_mask);

  rtl_setrelopi(RELOP_EQ, &s1, &id_src->val, 0);
  rtl_mux(&s2, &s1, &s2, &s0);
  rtl_update_ZFSF(&s2, id_dest->width);

  operand_write(id_dest, &s0);

  print_asm_template2(sar);
}

make_EHelper(shl) {
  rtl_get_SF(&s2);
  rtl_shli(&s2, &s2, id_dest->width * 8 - 1);
  rtl_get_ZF(&s1);
  rtl_xori(&s1, &s1, 1);
  rtl_or(&s2, &s2, &s1);

  rtl_andi(&id_src->val, &id_src->val, 0x1f);
  rtl_shl(&s0, &id_dest->val, &id_src->val);

  // unnecessary to update CF and OF in NEMU
  flag_mask &= (~cf_mask) & (~of_mask);

  rtl_setrelopi(RELOP_EQ, &s1, &id_src->val, 0);
  rtl_mux(&s2, &s1, &s2, &s0);
  rtl_update_ZFSF(&s2, id_dest->width);

  operand_write(id_dest, &s0);

  print_asm_template2(shl);
}

make_EHelper(shr) {
  rtl_get_SF(&s2);
  rtl_shli(&s2, &s2, id_dest->width * 8 - 1);
  rtl_get_ZF(&s1);
  rtl_xori(&s1, &s1, 1);
  rtl_or(&s2, &s2, &s1);

  rtl_andi(&id_src->val, &id_src->val, 0x1f);
  rtl_shr(&s0, &id_dest->val, &id_src->val);

  // unnecessary to update CF and OF in NEMU
  flag_mask &= (~cf_mask) & (~of_mask);

  rtl_setrelopi(RELOP_EQ, &s1, &id_src->val, 0);
  rtl_mux(&s2, &s1, &s2, &s0);
  rtl_update_ZFSF(&s2, id_dest->width);

  operand_write(id_dest, &s0);

  print_asm_template2(shr);
}

make_EHelper(shrd) {
  rtl_get_SF(&s2);
  rtl_shli(&s2, &s2, id_dest->width * 8 - 1);
  rtl_get_ZF(&s1);
  rtl_xori(&s1, &s1, 1);
  rtl_or(&s2, &s2, &s1);

  rtl_andi(&id_src->val, &id_src->val, 0x1f);
  rtl_li(&s0, id_dest->width * 8);
  rtl_sub(&s0, &s0, &id_src->val);
  rtl_shl(&s0, &id_src2->val, &s0);
  rtl_shr(&s1, &id_dest->val, &id_src->val);
  rtl_or(&s0, &s0, &s1);

  // unnecessary to update CF and OF in NEMU
  flag_mask &= (~cf_mask) & (~of_mask);

  rtl_setrelopi(RELOP_EQ, &s1, &id_src->val, 0);
  rtl_mux(&s2, &s1, &s2, &s0);
  rtl_update_ZFSF(&s2, id_dest->width);

  operand_write(id_dest, &s0);

  print_asm_template3(shrd);
}

make_EHelper(rol) {
  rtl_andi(&s0, &id_src->val, 0x1f);
  rtl_li(&s1, 32);
  rtl_sub(&s1, &s1, &s0);
  rtl_shr(&s1, &id_dest->val, &s1);
  rtl_shl(&s0, &id_dest->val, &s0);
  rtl_or(&s0, &s0, &s1);

  flag_mask &= (~cf_mask) & (~of_mask);

  operand_write(id_dest, &s0);

  print_asm_template2(rol);
}

make_EHelper(setcc) {
  uint32_t cc = decinfo.opcode & 0xf;

  rtl_setcc(&s0, cc);
  operand_write(id_dest, &s0);

  print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}

make_EHelper(not) {
  rtl_not(&id_dest->val, &id_dest->val);
  operand_write(id_dest, &id_dest->val);

  print_asm_template1(not);
}
