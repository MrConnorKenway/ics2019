int main() {
  asm("mov $0x12ae, %cx");
  asm("sub $0x12af, %cx");

  asm("mov $0xffffffff, %edx");
  asm("add $0x80000000, %edx");

  asm("mov $0xf, %bl");
  asm("add $0xf1, %bl");

  asm("mov $0x72, %dl");
  asm("add $0xe, %dl");

  asm("mov $127, %bl");
  asm("add $127, %bl");

  asm("mov $-128, %bl");
  asm("add $-128, %bl");

  asm("mov $-128, %dl");
  asm("sub $127, %dl");

  asm("mov $127, %dl");
  asm("sub $-128, %dl");

  asm("mov $-128, %dx");
  asm("sub $127, %dx");

  asm("mov $127, %dx");
  asm("sub $-128, %dx");

  asm("mov $0x7f, %al");
  asm("inc %ax");
  asm("mov $0x7fff, %ax");
  asm("inc %ax");
  asm("mov $0x7fffffff, %eax");
  asm("inc %ax");
  asm("mov $0xffffffff, %eax");
  asm("inc %ax");

  asm("mov $0, %cl");
  asm("sar %cl, %eax");
  asm("shl %cl, %eax");

  asm("mov $0xffff, %cx");
  asm("mov $0x8000, %dx");
  asm("imul %cx, %dx");

  asm("mov $0x8000, %cx");
  asm("mov $0x8000, %dx");
  asm("imul %cx, %dx");

  asm("mov $0xffffffff, %ecx");
  asm("mov $0x80000000, %edx");
  asm("imul %ecx, %edx");

  asm("mov $0xffffffff, %edx");
  asm("imul %ecx, %edx");
  asm("imul %cx, %dx");

  asm("mov $0x80000000, %ecx");
  asm("mov $0x80000000, %edx");
  asm("imul %ecx, %edx");

  return 0;
}
