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

  rtl_sar(&s0, &id_dest->val, &id_src->val);

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
  TODO();
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint32_t cc = decinfo.opcode & 0xf;

  rtl_setcc(&s0, cc);
  operand_write(id_dest, &s0);

  print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}

make_EHelper(not) {
  TODO();

  print_asm_template1(not);
}
