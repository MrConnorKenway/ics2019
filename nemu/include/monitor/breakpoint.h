#ifndef __BREAKPOINT_H__
#define __BREAKPOINT_H__

#include "common.h"

typedef struct breakpoint {
  int NO;
  struct breakpoint *next, *prev;

  char *expr;
  uint32_t addr;
} BP;

BP *new_bp();
void free_bp(int);
void bp_info();
bool check_bp(vaddr_t);

#endif
