#include "isa/mmu.h"
#include "nemu.h"

paddr_t page_translate(vaddr_t addr) {
  if (cpu.cr0.paging) {
    paddr_t pgdir = cpu.cr3.val & 0xfffff000;
    uint32_t pdx = (addr >> 22) & 0x3ff, ptx = (addr >> 12) & 0x3ff;
    paddr_t pde_addr = pgdir + pdx * sizeof(PDE);
    PDE pde;
    pde.val = paddr_read(pde_addr, 4);
    if (!pde.present) {
      panic("pgdir: 0x%08x addr: 0x%08x pde addr: 0x%08x", pgdir, addr, pde_addr);
    }
    paddr_t pgtbl = pde.val & 0xfffff000;
    paddr_t pte_addr = pgtbl + ptx * sizeof(PTE);
    PTE pte;
    pte.val = paddr_read(pte_addr, 4);
    if (!pte.present) {
      panic("pgdir: 0x%08x addr: 0x%08x pte addr: 0x%08x", pgdir, addr, pte_addr);
    }
    return (pte.val & 0xfffff000) | (addr & 0xfff);
  } else {
    return addr;
  }
}

uint32_t isa_vaddr_read(vaddr_t addr, int len) {
  vaddr_t offset = addr & 0xfff;
  if (offset + len > PAGE_SIZE) {
    paddr_t paddr_lo = page_translate(addr),
            paddr_hi = page_translate((addr & 0xfffff000) + PAGE_SIZE);
    int len_lo = PAGE_SIZE - offset;
    uint32_t data_lo = paddr_read(paddr_lo, len_lo),
             data_hi = paddr_read(paddr_hi, len - len_lo);
    return data_hi << (len_lo * 8) | data_lo;
  } else {
    paddr_t paddr = page_translate(addr);
    return paddr_read(paddr, len);
  }
}

void isa_vaddr_write(vaddr_t addr, uint32_t data, int len) {
  vaddr_t offset = addr & 0xfff;
  if (offset + len > PAGE_SIZE) {
    paddr_t paddr_lo = page_translate(addr),
            paddr_hi = page_translate((addr & 0xfffff000) + PAGE_SIZE);
    int len_lo = PAGE_SIZE - offset;
    paddr_write(paddr_lo, data & ((1 << (len_lo * 8)) - 1), len_lo);
    paddr_write(paddr_hi, data >> (len_lo * 8), len - len_lo);
  } else {
    paddr_t paddr = page_translate(addr);
    paddr_write(paddr, data, len);
  }
}
