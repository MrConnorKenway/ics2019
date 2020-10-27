#include "monitor/breakpoint.h"

#include <stdlib.h>

#define NR_BP 32

static BP bp_pool[NR_BP] = {};
static BP *head = NULL, *free_ = NULL;

void init_bp_pool() {
  int i;
  for (i = 0; i < NR_BP; i++) {
    bp_pool[i].NO = i;
    bp_pool[i].next = &bp_pool[i + 1];
    bp_pool[i].prev = &bp_pool[i - 1];
  }
  bp_pool[NR_BP - 1].next = NULL;
  bp_pool[0].prev = NULL;

  head = NULL;
  free_ = bp_pool;
}

BP *new_bp() {
  if (free_ == NULL) {
    printf("No free breakpoint\n");
    return NULL;
  }
  BP *t = free_;
  free_ = free_->next;
  free_->prev = NULL;
  t->next = head;
  t->prev = NULL;
  if (head != NULL) {
    head->prev = t;
  }
  head = t;
  return t;
}

bool check_bp(vaddr_t pc) {
  BP *ptr = head;
  while (ptr != NULL) {
    if (pc == ptr->addr) {
      printf("Breakpoint %d: %s\n", ptr->NO, ptr->expr);
      return true;
    }
    ptr = ptr->next;
  }
  return false;
}

void bp_info() {
  BP *ptr = head;
  if (head == NULL) {
    printf("No breakpoints\n");
    return;
  }

  printf("Num\tExpr\n");
  while (ptr != NULL) {
    printf("%d\t%s\n", ptr->NO, ptr->expr);
    ptr = ptr->next;
  }
}

void free_bp_helper(BP *bp) {
  Assert(bp != NULL, "Invalid breakpoint\n");
  free(bp->expr);
  if (bp == head) {
    head = bp->next;
  }
  bp->expr = NULL;
  if (bp->prev != NULL) {
    bp->prev->next = bp->next;
  }
  if (bp->next != NULL) {
    bp->next->prev = bp->prev;
  }
  bp->next = free_;
  bp->prev = NULL;
  if (free_ != NULL) {
    free_->prev = bp;
  }
  free_ = bp;
}

void free_bp(int NO) {
  BP *ptr = head;
  while (ptr != NULL) {
    if (ptr->NO == NO) {
      free_bp_helper(ptr);
      return;
    }
    ptr = ptr->next;
  }
  printf("No breakpoint number: %d\n", NO);
}
