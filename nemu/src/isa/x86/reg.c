#include "nemu.h"
#include <stdlib.h>
#include <time.h>

const char *regsl[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
const char *regsw[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char *regsb[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

const uint32_t cf_mask = 1u << 0;
const uint32_t zf_mask = 1u << 6;
const uint32_t sf_mask = 1u << 7;
const uint32_t if_mask = 1u << 9;
const uint32_t df_mask = 1u << 10;
const uint32_t of_mask = 1u << 11;
uint32_t flag_mask = cf_mask | zf_mask | sf_mask | if_mask | df_mask | of_mask;

void reg_test() {
  srand(time(0));
  uint32_t sample[8];
  uint32_t pc_sample = rand();
  cpu.pc = pc_sample;

  int i;
  for (i = R_EAX; i <= R_EDI; i++) {
    sample[i] = rand();
    reg_l(i) = sample[i];
    assert(reg_w(i) == (sample[i] & 0xffff));
  }

  assert(reg_b(R_AL) == (sample[R_EAX] & 0xff));
  assert(reg_b(R_AH) == ((sample[R_EAX] >> 8) & 0xff));
  assert(reg_b(R_BL) == (sample[R_EBX] & 0xff));
  assert(reg_b(R_BH) == ((sample[R_EBX] >> 8) & 0xff));
  assert(reg_b(R_CL) == (sample[R_ECX] & 0xff));
  assert(reg_b(R_CH) == ((sample[R_ECX] >> 8) & 0xff));
  assert(reg_b(R_DL) == (sample[R_EDX] & 0xff));
  assert(reg_b(R_DH) == ((sample[R_EDX] >> 8) & 0xff));

  assert(sample[R_EAX] == cpu.eax);
  assert(sample[R_ECX] == cpu.ecx);
  assert(sample[R_EDX] == cpu.edx);
  assert(sample[R_EBX] == cpu.ebx);
  assert(sample[R_ESP] == cpu.esp);
  assert(sample[R_EBP] == cpu.ebp);
  assert(sample[R_ESI] == cpu.esi);
  assert(sample[R_EDI] == cpu.edi);

  assert(pc_sample == cpu.pc);
}

void isa_reg_display() {
  int i;
  for (i = R_EAX; i <= R_EDI; i++) {
    printf("%s\t0x%08x\t%d\n", regsl[i], reg_l(i), reg_l(i));
  }
  printf("CF\t%d\n", cpu.CF);
  printf("OF\t%d\n", cpu.OF);
  printf("ZF\t%d\n", cpu.ZF);
  printf("SF\t%d\n", cpu.SF);
  printf("IF\t%d\n", cpu.IF);
  printf("DF\t%d\n", cpu.DF);
  printf("eip\t0x%08x\t%d\n", cpu.pc, cpu.pc);
}

uint32_t isa_reg_str2val(const char *s, bool *success) {
  for (int i = R_EAX; i < R_EDI; ++i) {
    if (strcmp(regsl[i], s) == 0) {
      if (success != NULL) {
        *success = true;
      }
      return cpu.gpr[i]._32;
    }
  }
  if (success != NULL) {
    *success = false;
  }
  return 0;
}
