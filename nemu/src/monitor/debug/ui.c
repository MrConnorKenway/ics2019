#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <errno.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
  if (args == NULL) {
    cpu_exec(1);
  } else {
    char *arg = strtok(NULL, " ");
    if (arg == NULL) {
      cpu_exec(1);
      return 0;
    }
    char *endptr;
    long n = strtol(arg, &endptr, 0);
    if (n < 0 || *endptr != 0 || errno == ERANGE) {
      printf("Invalid argument: '%s'\n", arg);
      return 0;
    }
    cpu_exec(n);
  }
  return 0;
}

static int cmd_info(char *args) {
  if (args == NULL) {
    printf("Usage: info SUBCMD\n");
    return 0;
  }
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("Usage: info SUBCMD\n");
    return 0;
  }

  if (strcmp(arg, "r") == 0) {
    isa_reg_display();
  } else if (strcmp(arg, "w") == 0) {
    wp_info();
  } else {
    printf("Unknown argument: '%s'\n", arg);
  }
  return 0;
}

static int cmd_p(char *args) {
  if (args == NULL) {
    printf("Usage: p EXPR\n");
    return 0;
  }

  bool success;
  int r = expr(args, &success);
  if (success) {
    printf("%d\n", r);
  }
  return 0;
}

static int cmd_x(char *args) {
  if (args == NULL) {
    printf("Usage: x N EXPR\n");
    return 0;
  }
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("Usage: x N EXPR\n");
    return 0;
  }

  char *endptr;
  long n = strtol(arg, &endptr, 0);
  if (n < 0 || *endptr != 0 || errno == ERANGE) {
    printf("Invalid argument: '%s'\n", arg);
  }

  arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("Usage: x N EXPR\n");
    return 0;
  }
  bool success;
  uint32_t ret = expr(arg, &success);
  if (!success) {
    return 0;
  }
  if (ret >= PMEM_SIZE) {
    printf("Cannot access memory at address 0x%08x\n", ret);
    return 0;
  }
  uint32_t *addr = guest_to_host(ret);
  while (n > 0) {
    printf("0x%08x:\t", host_to_guest(addr));
    for (int i = 0; i < 4 && n > 0; ++i) {
      printf("0x%08x", *addr);
      if (i < 3) {
        printf("\t");
      }
      ++addr;
      --n;
    }
    printf("\n");
  }

  return 0;
}

static int cmd_w(char *args) {
  if (args == NULL) {
    printf("Usage: w EXPR\n");
    return 0;
  }

  bool success;
  uint32_t val = expr(args, &success);
  if (!success) {
    return 0;
  }

  if (is_constant_expr(args)) {
    printf("Cannot watch constant expression\n");
    return 0;
  }

  WP *wp = new_wp();
  wp->expr = (char *) malloc(strlen(args) + 1);
  strcpy(wp->expr, args);
  printf("Watchpoint %d: %s\n", wp->NO, wp->expr);
  wp->old_val = val;
  return 0;
}

static int cmd_d(char *args) {
  if (args == NULL) {
    printf("Usage: d N\n");
  }
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("Usage: d N\n");
  }

  char *endptr;
  long n = strtol(arg, &endptr, 0);
  if (n < 0 || *endptr != 0 || errno == ERANGE) {
    printf("Invalid argument: '%s'\n", arg);
    return 0;
  }
  free_wp((int) n);
  return 0;
}

static struct {
  char *name;
  char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display informations about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},

    /* TODO: Add more commands */
    {"si", "si [N]: Execute N machine instructions, then stop and return to the debugger", cmd_si},
    {"info", "\n\tinfo r: Display register status\n\tinfo w: Print watchpoint info", cmd_info},
    {"p", "p EXPR: Print the value of expression 'EXPR'", cmd_p},
    {"x", "x N EXPR: Examine N words of memory starting at the value of 'EXPR'", cmd_x},
    {"w", "w EXPR: Set watchpoint to stop program when the value of 'EXPR' changed", cmd_w},
    {"d", "d N: Delete watchpoint N", cmd_d}
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  } else {
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
