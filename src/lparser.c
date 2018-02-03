/**
 *
 * Parser functions.
 *
 */

#include "mpc/mpc.h"
#include "lval.h"

#define NR_OF_PARSERS 8

static const char *grammar =
  "symbol: /[a-zA-Z0-9+\\-*\\/!?%<=>&]+/    ; "
  "number: /[+-]?[0-9]+]*/                  ; "
  "comment: /;[^\\r\\n]*/                   ; "
  "string: /\"(\\\\.|[^\"])*\"/             ; "
  "atom: <number> | <symbol> | <string>     ; "
  "list: '(' <sexp>* ')' | '{' <sexp>* '}'  ; "
  "sexp: <atom> | <list> | <comment>        ; "
  "lisp: /^/ <sexp> /$/                     ; "
  ;

static const char *lparser_names[] = {
  "lisp", "sexp", "list", "atom", "string", "comment", "number", "symbol"
};

typedef struct lparser {
  mpc_parser_t *parsers[NR_OF_PARSERS];
  mpc_result_t   result;
} lparser;

lparser *
lparser_create ()
{
  lparser *p = malloc(sizeof(lparser));

  for (long i = 0; i < NR_OF_PARSERS; i++) {
    p->parsers[i] = mpc_new(lparser_names[i]);
  }

  mpca_lang(MPCA_LANG_DEFAULT, grammar, 
            p->parsers[0],
            p->parsers[1],
            p->parsers[2],
            p->parsers[3],
            p->parsers[4],
            p->parsers[5],
            p->parsers[6],
            p->parsers[7]);

  return p;
}

void
lparser_delete (lparser *p)
{
  mpc_cleanup(NR_OF_PARSERS,
              p->parsers[0],
              p->parsers[1],
              p->parsers[2],
              p->parsers[3],
              p->parsers[4],
              p->parsers[5],
              p->parsers[6],
              p->parsers[7]);
  free(p);
}

int
lparser_parse (lparser *p, const char *s)
{
  return mpc_parse("<stdin>", s, p->parsers[0], &p->result);
}

int
lparser_parse_file (lparser *p, const char *filename)
{
  return mpc_parse_contents(filename, p->parsers[0], &p->result);
}

mpc_ast_t *
lparser_ast (const lparser *p)
{
  return p->result.output;
}

void
lparser_ast_delete (const lparser *p)
{
  mpc_ast_delete(p->result.output);
}

char *
lparser_error (const lparser *p)
{
  char *errmsg = mpc_err_string(p->result.error);
  mpc_err_delete(p->result.error);
  return errmsg;
}
