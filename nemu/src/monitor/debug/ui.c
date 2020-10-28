#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/breakpoint.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <errno.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);
void difftest_attach();
void difftest_detach();

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
  } else if (strcmp(arg, "b") == 0) {
    bp_info();
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
  uint32_t addr = expr(arg, &success);
  if (!success) {
    return 0;
  }

  while (n > 0) {
    printf("0x%08x:\t", addr);
    for (int i = 0; i < 4 && n > 0; ++i) {
      printf("0x%08x", vaddr_read(addr, 4));
      if (i < 3) {
        printf("\t");
      }
      addr += 4;
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
  if (wp == NULL) {
    return 0;
  }
  wp->expr = (char *) malloc(strlen(args) + 1);
  strcpy(wp->expr, args);
  printf("Watchpoint %d: %s\n", wp->NO, wp->expr);
  wp->old_val = val;
  return 0;
}

static int cmd_b(char *args) {
  if (args == NULL) {
    printf("Usage: b ADDR\n");
    return 0;
  }

  bool success;
  uint32_t val = expr(args, &success);
  if (!success) {
    return 0;
  }

  BP *bp = new_bp();
  if (bp == NULL) {
    return 0;
  }
  bp->expr = (char *)malloc(strlen(args) + 1);
  strcpy(bp->expr, args);
  printf("Breakpoint %d: %s\n", bp->NO, bp->expr);
  bp->addr = val;
  return 0;
}

static int cmd_d(char *args) {
  if (args == NULL) {
    printf("Usage: d [w|b] N\n");
    return 0;
  }
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("Usage: d [w|b] N\n");
    return 0;
  }

  char mode;
  if (strcmp(arg, "w") == 0) {
    mode = 'w';
  } else if (strcmp(arg, "b") == 0) {
    mode = 'b';
  } else {
    printf("Invalid argument: '%s'\n", arg);
    return 0;
  }

  arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("Usage: d [w|b] N\n");
    return 0;
  }
  char *endptr;
  long n = strtol(arg, &endptr, 0);
  if (n < 0 || *endptr != 0 || errno == ERANGE) {
    printf("Invalid argument: '%s'\n", arg);
    return 0;
  }
  if (mode == 'w') {
    free_wp((int)n);
  } else {
    free_bp((int) n);
  }
  return 0;
}

static int cmd_attach(char *args) {
  difftest_attach();
  return 0;
}

static int cmd_detach(char *args) {
  difftest_detach();
  return 0;
}

static int cmd_save(char *args) {
  if (args == NULL) {
    printf("Usage: save PATH\n");
    return 0;
  }

  char *path = strtok(NULL, " ");
  if (path == NULL) {
    printf("Usage: save PATH\n");
    return 0;
  }

  FILE *snapshot = fopen(path, "w+");
  if (!snapshot) {
    printf("Failed to open snapshot '%s': %s\n", path, strerror(errno));
    return 0;
  }
  if (fwrite(&cpu, sizeof(cpu), 1, snapshot) == 0) {
    printf("Failed to save snapshot '%s': %s\n", path, strerror(errno));
    return 0;
  }
  if (fwrite(pmem, PMEM_SIZE, 1, snapshot) == 0) {
    printf("Failed to save snapshot '%s': %s\n", path, strerror(errno));
    return 0;
  }
  fclose(snapshot);
  return 0;
}

static int cmd_load(char *args) {
  if (args == NULL) {
    printf("Usage: load PATH\n");
    return 0;
  }

  char *path = strtok(NULL, " ");
  if (path == NULL) {
    printf("Usage: load PATH\n");
    return 0;
  }

  FILE *snapshot = fopen(path, "r");
  if (!snapshot) {
    printf("Failed to open snapshot '%s': %s\n", path, strerror(errno));
    return 0;
  }
  if (fread(&cpu, sizeof(cpu), 1, snapshot) == 0) {
    printf("Failed to load content of snapshot '%s': %s\n", path,
           strerror(errno));
    return 0;
  }
  if (fread(pmem, PMEM_SIZE, 1, snapshot) == 0) {
    printf("Failed to load content of snapshot '%s': %s\n", path,
           strerror(errno));
    return 0;
  }
  fclose(snapshot);
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
    {"info", "info r: Display register status\n\t"
             "info w: Print watchpoint info\n\t"
             "info b: Print breakpoint info", cmd_info},
    {"p", "p EXPR: Print the value of expression 'EXPR'", cmd_p},
    {"x", "x N EXPR: Examine N words of memory starting at the value of 'EXPR'", cmd_x},
    {"b", "b ADDR: Set breakpoint at specified location", cmd_b},
    {"w", "w EXPR: Set watchpoint to stop program when the value of 'EXPR' changed", cmd_w},
    {"d", "d w N: Delete watchpoint N\n    d b N: Delete breakpoint N", cmd_d},
    {"attach", "Enter DiffTest mode", cmd_attach},
    {"detach", "Exit DiffTest mode", cmd_detach},
    {"save", "save PATH: Save snapshot", cmd_save},
    {"load", "load PATH: Load snapshot", cmd_load}
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
