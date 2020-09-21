#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <stdlib.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256,

  // binary operators
  TK_EQ,
  TK_LESSEQ,
  TK_GREATEREQ,
  TK_NEQ,
  TK_AND,
  TK_OR,

  UNARY_OP_BEG,
  // unary operators
  TK_NEG,
  TK_DEREF,
  TK_NOT,

  OP_END,

  TK_DEC,
  TK_OCT,
  TK_HEX,
  TK_VAR,
  TK_REG
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE},  // spaces
    {"\\+", '+'},       // plus
    {"-", '-'},
    {"\\*", '*'},
    {"/", '/'},
    {"==", TK_EQ},      // equal
    {"<", '<'},
    {"<=", TK_LESSEQ},
    {">", '>'},
    {">=", TK_GREATEREQ},
    {"0[xX][0-9A-Fa-f]+", TK_HEX},
    {"0[0-7]+", TK_OCT},
    {"[0-9]+", TK_DEC},
    {"\\(", '('},
    {"\\)", ')'},
    {"[a-zA-Z_][a-zA-Z0-9_]*", TK_VAR},
    {"\\$(eax|ecx|edx|ebx|esp|ebp|esi|edi|pc)", TK_REG},
    {"&&", TK_AND},
    {"\\|\\|", TK_OR},
    {"!=", TK_NEQ},
    {"!", TK_NOT}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char *str;
} Token;

static Token tokens[64] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    if (nr_token >= (sizeof(tokens) / sizeof(Token))) {
      printf("Too many tokens\n");
      return false;
    }

    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        if (rules[i].token_type == TK_NOTYPE) {
          break;
        }

        if (rules[i].token_type == TK_VAR) {
          printf("Variable evaluation has not been implemented yet\n");
          return false;
        }

        Token t = {rules[i].token_type, NULL};
        t.str = (char *) malloc(substr_len + 1);
        strncpy(t.str, substr_start, substr_len);
        t.str[substr_len] = 0;

        tokens[nr_token] = t;
        ++nr_token;

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  for (int i = 0; i < nr_token; ++i) {
    bool prev_is_op = i == 0 || (tokens[i - 1].type < OP_END && tokens[i - 1].type != ')');
    if (tokens[i].type == '*' && prev_is_op) {
      tokens[i].type = TK_DEREF;
    }
    if (tokens[i].type == '-' && prev_is_op) {
      tokens[i].type = TK_NEG;
    }
    if (tokens[i].type == '!' && prev_is_op) {
      tokens[i].type = TK_NOT;
    }
  }
  return true;
}

int precedence_table[] = {
    TK_OR,
    TK_AND,
    '<', '>', TK_LESSEQ, TK_GREATEREQ, TK_EQ, TK_NEQ,
    '+', '-',
    '*', '/',
    TK_NEG,
    TK_NOT,
    TK_DEREF
};

bool check_parentheses(int beg, int end) {
  int open_paren_cnt = 0;
  for (int i = beg; i <= end; ++i) {
    if (tokens[i].type == '(') {
      ++open_paren_cnt;
    } else if (tokens[i].type == ')') {
      --open_paren_cnt;
      if (open_paren_cnt < 0) {
        return false;
      }
    }
  }
  return open_paren_cnt == 0;
}

int64_t eval(int beg, int end, bool *success) {
  if (beg == end) {
    switch (tokens[beg].type) {
      case TK_VAR: {
        TODO();
      }
      case TK_OCT:
      case TK_DEC:
      case TK_HEX: {
        return strtol(tokens[beg].str, NULL, 0);
      }
      case TK_REG: {
        uint32_t val;
        if (strcmp(tokens[beg].str, "$pc") == 0) {
          val = cpu.pc;
        } else {
          val = isa_reg_str2val(tokens[beg].str + 1, NULL);
        }
        return val;
      }
      default: {
        printf("Bad expression\n");
        *success = false;
        return 0;
      }
    }
  } else if (beg > end) {
    printf("Bad expression\n");
    *success = false;
    return 0;
  } else {
    if (check_parentheses(beg, end)) {
      if (tokens[beg].type == '(' && tokens[end].type == ')' && check_parentheses(beg + 1, end - 1)) {
        return eval(beg + 1, end - 1, success);
      } else {
        // find main operator to split the whole expression
        int op = -1, open_paren_cnt = 0, min_precedence = sizeof(precedence_table) / sizeof(int);
        bool in_paren = false;
        for (int i = beg; i <= end; ++i) {
          if (tokens[i].type == '(') {
            ++open_paren_cnt;
          } else if (tokens[i].type == ')') {
            --open_paren_cnt;
          }
          in_paren = open_paren_cnt > 0;
          if (tokens[i].type < OP_END && !in_paren) {
            for (int l = 0; l < sizeof(precedence_table) / sizeof(int); ++l) {
              if (precedence_table[l] == tokens[i].type) {
                if (tokens[i].type > UNARY_OP_BEG) {
                  // unary operator is right associative, so we choose the leftmost operator as the main op
                  if (l < min_precedence) {
                    op = i;
                    min_precedence = l;
                  }
                } else {
                  // binary operator is left associative, so we choose the rightmost operator as the main op
                  if (l <= min_precedence) {
                    op = i;
                    min_precedence = l;
                  }
                }
              }
            }
          }
        }

        Assert(op != -1, "Bad expression\n");

        if (tokens[op].type == TK_DEREF) {
          uint32_t ret = eval(op + 1, end, success);
          if (ret >= PMEM_SIZE) {
            printf("Cannot access memory at address 0x%08x\n", ret);
            *success = false;
            return 0;
          }
          uint32_t *addr = guest_to_host(ret);
          return *addr;
        } else if (tokens[op].type == TK_NEG) {
          return -eval(op + 1, end, success);
        } else if (tokens[op].type == TK_NOT) {
          return !eval(op + 1, end, success);
        } else {
          int64_t lhs_val = eval(beg, op - 1, success);
          if (*success == false) {
            return 0;
          }
          int64_t rhs_val = eval(op + 1, end, success);
          if (*success == false) {
            return 0;
          }
          switch (tokens[op].type) {
            case '+': return lhs_val + rhs_val;
            case '-': return lhs_val - rhs_val;
            case '*': return lhs_val * rhs_val;
            case '/':
              if (rhs_val == 0) {
                *success = false;
                printf("Divide by 0 error\n");
                return 0;
              }
              return lhs_val / rhs_val;
            case TK_NEQ: return lhs_val != rhs_val;
            case TK_EQ: return lhs_val == rhs_val;
            case '<': return lhs_val < rhs_val;
            case '>': return lhs_val > rhs_val;
            case TK_LESSEQ: return lhs_val <= rhs_val;
            case TK_GREATEREQ: return lhs_val >= rhs_val;
            case TK_AND: return lhs_val && rhs_val;
            case TK_OR: return lhs_val || rhs_val;
            default:panic("Unknown operator\n");
          }
        }
      }
    } else {
      printf("Bad expression\n");
      *success = false;
      return 0;
    }
  }
}

bool is_constant_expr_helper(char *e) {
  make_token(e);

  for (int i = 0; i < nr_token; ++i) {
    if (tokens[i].type == TK_REG || tokens[i].type == TK_VAR || tokens[i].type == TK_DEREF) {
      return false;
    }
  }

  return true;
}

bool is_constant_expr(char *e) {
  bool b = is_constant_expr_helper(e);
  for (int i = 0; i < nr_token; ++i) {
    if (tokens[i].str != NULL) {
      free(tokens[i].str);
    }
  }
  return b;
}

uint32_t expr(char *e, bool *success) {
  uint32_t val;
  if (!make_token(e)) {
    *success = false;
    val = 0;
  } else {
    /* TODO: Insert codes to evaluate the expression. */
    *success = true;
    val = eval(0, nr_token - 1, success);
  }

  for (int i = 0; i < nr_token; ++i) {
    if (tokens[i].str != NULL) {
      free(tokens[i].str);
    }
  }
  return val;
}
