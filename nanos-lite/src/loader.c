#include <elf.h>

#include "fs.h"
#include "proc.h"

#ifdef __ISA_AM_NATIVE__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr elf_ehdr;
  int fd = fs_open(filename, 0, 0);
  fs_read(fd, &elf_ehdr, sizeof(elf_ehdr));
  if (strncmp((const char *) elf_ehdr.e_ident, ELFMAG, SELFMAG) != 0) {
    panic("Not an ELF file - it has the wrong magic bytes at the start");
  }
  Elf_Phdr elf_ph[elf_ehdr.e_phnum];
  fs_lseek(fd, elf_ehdr.e_phoff, SEEK_SET);
  fs_read(fd, &elf_ph, sizeof(elf_ph));
  Elf_Phdr *ph = &elf_ph[0], *eph = ph + elf_ehdr.e_phnum;
  for (; ph < eph; ++ph) {
    if (ph->p_type == PT_LOAD) {
      fs_lseek(fd, ph->p_offset, SEEK_SET);
      uintptr_t va_start = PGROUNDDOWN(ph->p_vaddr),
                va_end = PGROUNDUP(ph->p_vaddr + ph->p_memsz),
                bss_start = PGROUNDDOWN(ph->p_vaddr + ph->p_filesz);
      for (uintptr_t va = va_start, remain_mem_sz = ph->p_memsz,
                     remain_bss_sz = ph->p_memsz - ph->p_filesz;
           va < va_end; va += PGSIZE) {
        void *pa = new_page(1);
        _map(&pcb->as, (void *)va, pa, 0);
        if (va <= bss_start) {
          if (va == va_start) {
            uintptr_t offset = ph->p_vaddr & 0xfff;
            fs_read(fd, pa + offset, MIN(remain_mem_sz, PGSIZE - offset));
            remain_mem_sz -= PGSIZE - offset;
          } else {
            fs_read(fd, pa, MIN(remain_mem_sz, PGSIZE));
            remain_mem_sz -= PGSIZE;
          }
          if (va == bss_start) {
            uintptr_t offset = (ph->p_vaddr + ph->p_filesz) & 0xfff;
            memset((uint8_t *)pa + offset, 0,
                   MIN(remain_bss_sz, PGSIZE - offset));
            remain_bss_sz -= PGSIZE - offset;
          }
        } else {
          memset((uint8_t *)pa, 0, MIN(remain_bss_sz, PGSIZE));
          remain_bss_sz -= PGSIZE;
        }
      }
    }
  }
  return elf_ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %x", entry);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry) {
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _kcontext(stack, entry, NULL);
  pcb->max_brk = 0;
}

void context_uload(PCB *pcb, const char *filename) {
  _protect(&pcb->as);
  uintptr_t entry = loader(pcb, filename);

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
  pcb->max_brk = 0;
}
