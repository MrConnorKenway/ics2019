#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next, *prev;

  /* TODO: Add more members if necessary */
  char *expr;
  uint32_t old_val;
} WP;

WP *new_wp();
void free_wp(int);
bool check_wp();
void wp_info();

#endif
