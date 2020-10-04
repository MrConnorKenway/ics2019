int main() {
  asm("mov $0x7fff, %ax");
  asm("cwtl");

  asm("mov $0x8fff, %ax");
  asm("cwtl");

  asm("mov $0x7f, %al");
  asm("cbtw");

  asm("mov $0x8f, %al");
  asm("cbtw");

  return 0;
}
