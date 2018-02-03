/**
 *
 * Holden, Daniel. Build Your Own Lisp. Learn C and Build Your Own
 * Programming Language in 1000 Lines of Code! CreateSpace Independent
 * Publishing Platform, 2014.
 *
 * http://www.buildyourownlisp.com/.
 *
 */

#include <editline/readline.h>
#include <histedit.h>

#include "util.h"
#include "lval.h"
#include "mpc/mpc.h"
#include "lparser.h"

int
main (int argc, char **argv)
{
  puts("My very own Lisp");
  puts("Press ctrl+c to exit");

  lenv  *env = lenv_create(NULL);
  lparser *p = lparser_create();

  lenv_register_builtin(env, "identity", builtin_identity, 0);
  lenv_register_builtin(env, "lambda", builtin_lambda, 1);
  lenv_register_builtin(env, "equal", builtin_equal, 0);
  lenv_register_builtin(env, "quote", builtin_quote, 1);
  lenv_register_builtin(env, "eval", builtin_eval, 0);
  lenv_register_builtin(env, "load", builtin_load, 0);
  lenv_register_builtin(env, "head", builtin_head, 0);
  lenv_register_builtin(env, "tail", builtin_tail, 0);
  lenv_register_builtin(env, "list", builtin_list, 0);
  lenv_register_builtin(env, "join", builtin_join, 0);
  lenv_register_builtin(env, "def", builtin_def, 1);
  lenv_register_builtin(env, "not", builtin_not, 0);
  lenv_register_builtin(env, "and", builtin_and, 1);
  lenv_register_builtin(env, "or", builtin_or, 1);
  lenv_register_builtin(env, ">", builtin_gt, 0);
  lenv_register_builtin(env, "<", builtin_lt, 0);
  lenv_register_builtin(env, "=", builtin_eq, 0);
  lenv_register_builtin(env, "if", builtin_if, 1);
  lenv_register_builtin(env, ">=", builtin_ge, 0);
  lenv_register_builtin(env, "<=", builtin_le, 0);
  lenv_register_builtin(env, "+", builtin_add, 0);
  lenv_register_builtin(env, "-", builtin_sub, 0);
  lenv_register_builtin(env, "*", builtin_mul, 0);
  lenv_register_builtin(env, "/", builtin_div, 0);

  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      lval *arg = lval_lst_append(lval_lst(), lval_str(argv[i]));
      lval *err = builtin_load(env, arg);
      lval_free(arg);
      if (lval_type(err) == LVAL_ERR) {
        lval_print(err);
        lval_free(err);
        break;
      }
      lval_free(err);
    }
  }

  while (1) {
    char *input = string_trim(readline("> "));
    if (input) {
      add_history(input);
      if (lparser_parse(p, input)) {

        lval *val = lval_eval(env, read_lval(lparser_ast(p)));

        lval_print(val);
        putchar('\n');

        lval_free(val);
        lparser_ast_delete(p);
      } else {
        char *errmsg = lparser_error(p);
        printf("%s", errmsg);
        free(errmsg);
      }
    }
    free(input);
  }

  lenv_free(env);
  lparser_delete(p);
  return 0;
}
