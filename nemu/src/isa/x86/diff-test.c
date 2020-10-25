#include "nemu.h"
#include "monitor/diff-test.h"
#include "../../../tools/qemu-diff/src/isa/x86/include/isa.h"

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
  uint16_t data[3];
  data[0] = cpu.IDTR.limit - 1;
  data[1] = (uint32_t)cpu.IDTR.base;
  data[2] = (uint32_t)cpu.IDTR.base >> 16;

  ref_difftest_memcpy_from_dut(0x7e00, data, sizeof(data));

  uint8_t lidt[] = {
      0xb8, 0x00, 0x7e, 0x00, 0x00,  // mov $0x7e00, $eax
      0x0f, 0x01, 0x18               // lidt (%eax)
  };

  ref_difftest_memcpy_from_dut(0x7e40, lidt, sizeof(lidt));

  union isa_gdb_regs r;
  ref_difftest_getregs(&r);

  r.eip = 0x7e40;
  ref_difftest_setregs(&r);

  ref_difftest_exec(2);

  ref_difftest_setregs(&cpu);

  ref_difftest_memcpy_from_dut(0x0, pmem, 0x7c00);
  ref_difftest_memcpy_from_dut(0x100000, pmem + 0x100000, PMEM_SIZE - 0x100000);
}
