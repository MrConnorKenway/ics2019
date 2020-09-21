#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#include <stdlib.h>

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
    wp_pool[i].prev = &wp_pool[i - 1];
  }
  wp_pool[NR_WP - 1].next = NULL;
  wp_pool[0].prev = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP *new_wp() {
  if (free_ == NULL) {
    printf("No free watchpoint\n");
    return NULL;
  }
  WP *t = free_;
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

bool check_wp() {
  WP *ptr = head;
  bool success, changed = false;
  while (ptr != NULL) {
    uint32_t new_val = expr(ptr->expr, &success);
    if (new_val != ptr->old_val) {
      changed = true;
      printf("Watchpoint %d: %s\n\nOld value = %u\nNew value = %u\n",
             ptr->NO,
             ptr->expr,
             ptr->old_val,
             new_val);
      ptr->old_val = new_val;
    }
    ptr = ptr->next;
  }

  return changed;
}

void wp_info() {
  WP *ptr = head;
  if (head == NULL) {
    printf("No watchpoints\n");
    return;
  }

  printf("Num\tExpr\n");
  while (ptr != NULL) {
    printf("%d\t%s\n", ptr->NO, ptr->expr);
    ptr = ptr->next;
  }
}

void free_wp_helper(WP *wp) {
  Assert(wp != NULL, "Invalid watchpoint\n");
  free(wp->expr);
  if (wp == head) {
    head = wp->next;
  }
  wp->expr = NULL;
  if (wp->prev != NULL) {
    wp->prev->next = wp->next;
  }
  if (wp->next != NULL) {
    wp->next->prev = wp->prev;
  }
  wp->next = free_;
  wp->prev = NULL;
  if (free_ != NULL) {
    free_->prev = wp;
  }
  free_ = wp;
}

void free_wp(int NO) {
  WP *ptr = head;
  while (ptr != NULL) {
    if (ptr->NO == NO) {
      free_wp_helper(ptr);
      return;
    }
    ptr = ptr->next;
  }
  printf("No watchpoint number: %d\n", NO);
}
