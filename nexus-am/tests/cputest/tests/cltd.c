int main() {
  asm("mov $0x7fff, %ax");
  asm(".byte 0x66");
  asm(".byte 0x99");

  asm("mov $0x8fff, %ax");
  asm(".byte 0x66");
  asm(".byte 0x99");

  asm("mov $-1, %eax");
  asm("cltd");
  return 0;
}
