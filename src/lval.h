#ifndef LVAL_H
#define LVAL_H

#include "mpc/mpc.h"

#define LVAL_NIL() lval_lst();
#define LVAL_T()   lval_sym("t");

typedef enum ltype { LVAL_ERR, LVAL_SYM, LVAL_NUM, LVAL_LST, LVAL_FUN, LVAL_STR } ltype;

typedef struct lval lval;
typedef struct lenv lenv;
typedef struct lfun lfun;
typedef enum   ltype ltype;
typedef lval  *lbuiltin(lenv*, lval*);

lval * read_lval (mpc_ast_t *node);
int    lval_type (const lval *val);

lval * lval_eval  (lenv *env, lval *val);
void   lval_free  (lval *val);
void   lval_quote (lval *val);
void   lval_print (const lval *val);
lval * lval_err   (const char *fmt, ...);
lval * lval_sym   (const char *name);
lval * lval_str   (const char *value);
lval * lval_num   (float value);
lval * lval_fun   (lbuiltin *builtin);
lval * lval_lst   ();
lval * lval_lst_insert (lval *lst, lval *val);
lval * lval_lst_append (lval *lst, lval *val);

lenv * lenv_create (lenv *parent);
void   lenv_put    (lenv *env, const char *name, lval *val);
void   lenv_free   (lenv *env);
void   lenv_register_builtin (lenv *env, const char *name, lbuiltin *builtin, int is_special);

lfun * lfun_builtin    (lbuiltin *builtin, int is_special);
lfun * lfun_userdef    (lenv *env, lval *args, lval *body);
lfun * lfun_copy       (lfun *src);
int    lfun_is_special (const lfun *fun);
void   lfun_free       (lfun *fun);

lval * builtin_identity (lenv *env, lval *arg);
lval * builtin_lambda   (lenv *env, lval *arg);
lval * builtin_add      (lenv *env, lval *arg);
lval * builtin_sub      (lenv *env, lval *arg);
lval * builtin_mul      (lenv *env, lval *arg);
lval * builtin_div      (lenv *env, lval *arg);
lval * builtin_head     (lenv *env, lval *arg);
lval * builtin_tail     (lenv *env, lval *arg);
lval * builtin_list     (lenv *env, lval *arg);
lval * builtin_join     (lenv *env, lval *arg);
lval * builtin_eval     (lenv *env, lval *arg);
lval * builtin_def      (lenv *env, lval *arg);
lval * builtin_not      (lenv *env, lval *arg);
lval * builtin_and       (lenv *env, lval *arg);
lval * builtin_or       (lenv *env, lval *arg);
lval * builtin_quote    (lenv *env, lval *arg);
lval * builtin_if       (lenv *env, lval *arg);
lval * builtin_gt       (lenv *env, lval *arg);
lval * builtin_lt       (lenv *env, lval *arg);
lval * builtin_ge       (lenv *env, lval *arg);
lval * builtin_le       (lenv *env, lval *arg);
lval * builtin_eq       (lenv *env, lval *arg);
lval * builtin_equal    (lenv *env, lval *arg);
lval * builtin_load     (lenv *env, lval *arg);

#endif
