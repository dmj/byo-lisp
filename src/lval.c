/**
 *
 * Lisp values and their environment.
 *
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include <string.h>
extern char *strdup (const char *s);

#include "util.h"
#include "lval.h"
#include "lparser.h"

char *
ltype_name (ltype type)
{
  switch (type) {
  case LVAL_ERR:
    return "error";
  case LVAL_SYM:
    return "symbol";
  case LVAL_NUM:
    return "number";
  case LVAL_LST:
    return "list";
  case LVAL_FUN:
    return "function";
  case LVAL_STR:
    return "string";
  }
  return "unknown";
}



#define LVAL_ALLOC(_v_,_t_) \
  lval *_v_ = malloc(sizeof(lval)); \
  _v_->type = _t_; \
  _v_->is_quoted = 0;

#define LVAL_NUM_VALUE(_v_) *(float*)_v_->value

#define LVAL_ASSERT_TYPE(_v_,_t_) \
  if (_v_->type != _t_) { \
    return lval_err("Wrong type of argument: %s, %s", ltype_name(_t_), ltype_name(_v_->type)); \
  }

#define LVAL_LST_ASSERT_TYPE(_v_,_t_) \
  for (long i = 0; i < lval_lst_length(_v_); i++) { \
    LVAL_ASSERT_TYPE(lval_lst_nth(_v_, i), _t_);    \
  }

#define LVAL_ASSERT_NUMARG(_v_,_n_) \
  if (lval_lst_length(_v_) != _n_) { \
    return lval_err("Invalid number of arguments: %d, %d", _n_, lval_lst_length(_v_)); \
  }

#define LVAL_ASSERT_NUMARG_GE(_v_,_n_) \
  if (lval_lst_length(_v_) < _n_) { \
    return lval_err("Invalid number of arguments: >= %d, %d", _n_, lval_lst_length(_v_)); \
  }

#define LVAL_IS_NIL(_v_) (_v_->type == LVAL_LST && lval_lst_length(_v_) == 0)

typedef struct lval {
  ltype  type;
  void  *value;
  int    is_quoted;
} lval;

int
lval_type (const lval *val)
{
  return val->type;
}

lval *
lval_err (const char *fmt, ...)
{
  LVAL_ALLOC(val, LVAL_ERR);
  val->value = malloc(256);

  va_list args;
  va_start(args, fmt);

  vsnprintf(val->value, 255, fmt, args);

  va_end(args);

  val->value = realloc(val->value, 1 + strlen(val->value));
  return val;
}

lval *
lval_str (const char *value)
{
  LVAL_ALLOC(val, LVAL_STR);
  val->value = strdup(value);
  return val;
}

lval *
lval_sym (const char *name)
{
  LVAL_ALLOC(val, LVAL_SYM);
  val->value = strdup(name);
  return val;
}

lval *
lval_num (float value)
{
  LVAL_ALLOC(val, LVAL_NUM);
  val->value = malloc(sizeof(float));
  memcpy(val->value, &value, sizeof(float));
  return val;
}

lval *
lval_num_compare (const lval *a, const lval *b)
{
  LVAL_ASSERT_TYPE(a, LVAL_NUM);
  LVAL_ASSERT_TYPE(b, LVAL_NUM);
  if (LVAL_NUM_VALUE(a) < LVAL_NUM_VALUE(b)) {
    return lval_num(-1);
  }
  if (LVAL_NUM_VALUE(b) < LVAL_NUM_VALUE(a)) {
    return lval_num(1);
  }
  return lval_num(0);
}

lval *
lval_fun_builtin (lbuiltin *builtin, int is_special)
{
  LVAL_ALLOC(val, LVAL_FUN);
  val->value = lfun_builtin(builtin, is_special);
  return val;
}

lval *
lval_fun_userdef (lenv *env, lval *args, lval *body)
{
  LVAL_ALLOC(val, LVAL_FUN);
  val->value = lfun_userdef(env, args, body);
  return val;
}

int
lval_fun_is_special (const lval *fun)
{
  return lfun_is_special(fun->value);
}

lval *
lval_lst ()
{
  LVAL_ALLOC(val, LVAL_LST);
  val->value = list();
  return val;
}

lval *
lval_lst_append (lval *lst, lval *val)
{
  LVAL_ASSERT_TYPE(lst, LVAL_LST);
  list_append(lst->value, val);
  return lst;
}

lval *
lval_lst_insert (lval *lst, lval *val)
{
  LVAL_ASSERT_TYPE(lst, LVAL_LST);
  list_insert(lst->value, val);
  return lst;
}

long
lval_lst_length (const lval *lst)
{
  return list_length(lst->value);
}

lval *
lval_lst_nth (const lval *lst, long pos)
{
  LVAL_ASSERT_TYPE(lst, LVAL_LST);
  if (pos >= lval_lst_length(lst)) {
    return lval_err("Index out of range: %d, %d", lval_lst_length(lst), pos);
  }
  return list_nth(lst->value, pos);
}

lval *
lval_lst_take (lval *lst, long pos)
{
  LVAL_ASSERT_TYPE(lst, LVAL_LST);
  if (pos >= lval_lst_length(lst)) {
    return lval_err("Index out of range: %d, %d", lval_lst_length(lst), pos);
  }
  return list_take(lst->value, pos);
}

void
lval_free_lst (lval *lst)
{
  long length = list_length(lst->value);
  for (long i = 0; i < length; i++) {
    lval_free(list_nth(lst->value, i));
  }
  list_free(lst->value);
}

void
lval_free_fun (lval *fun)
{
  lfun_free(fun->value);
}

void
lval_free (lval *val)
{
  switch (val->type) {
  case LVAL_FUN:
    lval_free_fun(val);
    break;
  case LVAL_STR:
  case LVAL_SYM:
  case LVAL_ERR:
  case LVAL_NUM:
    free(val->value);
    break;
  case LVAL_LST:
    lval_free_lst(val);
    break;
  }

  free(val);
}

lval *
lval_copy (const lval *src)
{
  LVAL_ALLOC(dst, src->type);

  switch (src->type) {
  case LVAL_FUN:
    dst->value = lfun_copy(src->value);
    break;
  case LVAL_STR:
  case LVAL_SYM:
  case LVAL_ERR:
    dst->value = strdup(src->value);
    break;
  case LVAL_NUM:
    dst->value = malloc(sizeof(float));
    dst->value = memcpy(dst->value, src->value, sizeof(float));
    break;
  case LVAL_LST:
    dst->value = list();
    for (long i = list_length(src->value) - 1; i >= 0; i--) {
      list_insert(dst->value, lval_copy(list_nth(src->value, i)));
    }
    dst->is_quoted = src->is_quoted;
    break;
  }

  return dst;
}

void
lval_quote (lval *val)
{
  val->is_quoted = 1;
}

void
lval_unquote (lval *val)
{
  val->is_quoted = 0;
}

int
lval_is_quoted (const lval *val)
{
  return val->is_quoted;
}

void
lval_print (const lval *val)
{
  switch (val->type) {
  case LVAL_NUM:
    printf("%G", LVAL_NUM_VALUE(val));
    break;
  case LVAL_STR:
    printf("\"%s\"", (char*)val->value);
    break;
  case LVAL_SYM:
    printf("%s", (char*)val->value);
    break;
  case LVAL_ERR:
    printf("<error> %s", (char*)val->value);
    break;
  case LVAL_FUN:
    printf("<function>");
    break;
  case LVAL_LST:
    if (LVAL_IS_NIL(val)) {
      printf("nil");
    } else {
      putchar(lval_is_quoted(val) ? '{' : '(');
      for (long i = 0; i < lval_lst_length(val); i++) {
        if (i > 0) {
          putchar(' ');
        }
        lval_print(lval_lst_nth(val, i));
      }
      putchar(lval_is_quoted(val) ? '}' : ')');
    }
    break;
  }
}



typedef struct lenv {
  lenv   *parent;
  long    size;
  char  **names;
  lval  **lvals;
} lenv;

lenv *
lenv_create (lenv *parent)
{
  lenv *env = malloc(sizeof(lenv));
  env->size = 0;
  env->lvals = NULL;
  env->names = NULL;
  env->parent = parent;
  return env;
}

lenv *
lenv_copy (const lenv *src)
{
  lenv *dst = lenv_create(src->parent);
  for (long i = 0; i < src->size; i++) {
    lenv_put(dst, src->names[i], src->lvals[i]);
  }
  return dst;
}

/*
 * Put value into environment.
 */
void
lenv_put (lenv *env, const char *name, lval *val)
{
  for (long i = 0; i < env->size; i++) {
    if (strcmp(env->names[i], name) == 0) {
      lval_free(env->lvals[i]);
      env->lvals[i] = lval_copy(val);
      return;
    }
  }

  env->size++;
  env->names = realloc(env->names, env->size * sizeof(char*));
  env->lvals = realloc(env->lvals, env->size * sizeof(lval*));
  env->names[env->size - 1] = strdup(name);
  env->lvals[env->size - 1] = lval_copy(val);

}

/*
 * Return value from environment.
 */
lval *
lenv_get (lenv *env, const char *name)
{
  for (long i = 0; i < env->size; i++) {
    if (strcmp(env->names[i], name) == 0) {
      return lval_copy(env->lvals[i]);
    }
  }
  if (env->parent) {
    return lenv_get(env->parent, name);
  }
  return lval_err("Void variable: %s", name);
}

/*
 * Register new builtin function.
 */
void
lenv_register_builtin (lenv *env, const char *name, lbuiltin *builtin, int is_special)
{
  lval *fun = lval_fun_builtin(builtin, is_special);
  lenv_put(env, name, fun);
  lval_free(fun);
}

void
lenv_free (lenv *env)
{
  for (long i = 0; i < env->size; i++) {
    lval_free(env->lvals[i]);
    free(env->names[i]);
  }
  free(env->names);
  free(env->lvals);
  free(env);
}



typedef lval *lbuiltin(lenv*, lval*);

typedef struct lfun {
  lbuiltin *builtin;
  lval     *body;
  lval     *args;
  lenv     *env;
  int       is_special;
} lfun;

lfun *
lfun_builtin (lbuiltin *builtin, int is_special)
{
  lfun *fun = malloc(sizeof(lfun));
  fun->is_special = is_special;
  fun->builtin = builtin;
  fun->body = NULL;
  fun->args = NULL;
  fun->env = NULL;
  return fun;
}

lfun *
lfun_userdef (lenv *env, lval *args, lval *body)
{
  lfun *fun = malloc(sizeof(lfun));
  fun->is_special = 0;
  fun->builtin = NULL;
  fun->body = lval_copy(body);
  fun->args = lval_copy(args);
  fun->env = lenv_create(env);
  return fun;
}

lfun *
lfun_copy (lfun *src)
{
  if (src->builtin) {
    return lfun_builtin(src->builtin, src->is_special);
  }
  lfun *dst = malloc(sizeof(lfun));
  dst->builtin = NULL;
  dst->is_special = 0;
  dst->args = lval_copy(src->args);
  dst->body = lval_copy(src->body);
  dst->env  = lenv_copy(src->env);
  return dst;
}

void
lfun_free (lfun *fun)
{
  if (fun->builtin == NULL) {
    lval_free(fun->body);
    lval_free(fun->args);
    lenv_free(fun->env);
  }
  free(fun);
}

int
lfun_is_special (const lfun *fun)
{
  return fun->is_special;
}

lval *
lval_fun_call (lenv *env, lval *fun, lval *arg)
{
  lfun *f = (lfun*)fun->value;
  if (f->builtin) {
    return f->builtin(env, arg);
  }

  long restpos = 0;
  for (long i = 0; i < lval_lst_length(f->args); i++) {
    if (strcmp(lval_lst_nth(f->args, i)->value, "&rest") == 0) {
      break;
    }
    restpos++;
  }

  LVAL_ASSERT_NUMARG_GE(arg, restpos);

  lval *body = lval_copy(f->body);
  lenv *fenv = lenv_create(f->env);

  // bind the formal arguments
  for (long i = 0; i < restpos; i++) {
    lenv_put(fenv, lval_lst_nth(f->args, i)->value, lval_lst_nth(arg, i));
  }

  if (restpos < lval_lst_length(arg)) {
    lval *rest = lval_lst();
    for (long i = restpos; i < lval_lst_length(arg); i++) {
      rest = lval_lst_append(rest, lval_copy(lval_lst_nth(arg, i)));
    }
    restpos++;
    lenv_put(fenv, lval_lst_nth(f->args, restpos)->value, rest);
    lval_free(rest);
  }
  for (long i = 1 + restpos; i < lval_lst_length(f->args); i++) {
    lval *rest = lval_lst();
    lenv_put(fenv, lval_lst_nth(f->args, i)->value, rest);
    lval_free(rest);
  }

  lval_unquote(body);
  lval *ret = lval_eval(fenv, body);
  lenv_free(fenv);
  return ret;
}



lval *
builtin_identity (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG(arg, 1);
  return lval_copy(lval_lst_nth(arg, 0));
}

lval *
builtin_add (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG_GE(arg, 1);
  LVAL_LST_ASSERT_TYPE(arg, LVAL_NUM);

  float sum = 0;
  for (long i = 0; i < lval_lst_length(arg); i++) {
    sum += LVAL_NUM_VALUE(lval_lst_nth(arg, i));
  }
  return lval_num(sum);
}

lval *
builtin_sub (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG_GE(arg, 1);
  LVAL_LST_ASSERT_TYPE(arg, LVAL_NUM);

  float dif;

  if (lval_lst_length(arg) > 1) {
    dif = LVAL_NUM_VALUE(lval_lst_nth(arg, 0));

    for (long i = 1; i < lval_lst_length(arg); i++) {
      dif -= LVAL_NUM_VALUE(lval_lst_nth(arg, i));
    }
  } else {
    dif = 0 - LVAL_NUM_VALUE(lval_lst_nth(arg, 0));
  }

  return lval_num(dif);
}

lval *
builtin_mul (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG_GE(arg, 1);
  LVAL_LST_ASSERT_TYPE(arg, LVAL_NUM);

  float mul = LVAL_NUM_VALUE(lval_lst_nth(arg, 0));

  for (long i = 1; i < lval_lst_length(arg); i++) {
    mul *= LVAL_NUM_VALUE(lval_lst_nth(arg, i));
  }

  return lval_num(mul);
}

lval *
builtin_div (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG_GE(arg, 1);
  LVAL_LST_ASSERT_TYPE(arg, LVAL_NUM);

  float quo = LVAL_NUM_VALUE(lval_lst_nth(arg, 0));
  if (lval_lst_length(arg) == 1 && quo == 0) {
      return lval_err("Division by zero");
  }

  for (long i = 1; i < lval_lst_length(arg); i++) {
    quo /= LVAL_NUM_VALUE(lval_lst_nth(arg, i));
  }

  return lval_num(quo);
}

lval *
builtin_head (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG(arg, 1);
  LVAL_LST_ASSERT_TYPE(arg, LVAL_LST);
  lval *lst = lval_lst_nth(arg, 0);
  if (lval_lst_length(lst) == 0) {
    return lval_err("Cannot return head of an empty list");
  }
  return lval_copy(lval_lst_nth(lst, 0));
}

lval *
builtin_tail (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG(arg, 1);
  LVAL_LST_ASSERT_TYPE(arg, LVAL_LST);
  lval *lst = lval_lst_nth(arg, 0);
  if (lval_lst_length(lst) == 0) {
    return lval_err("Cannot return tail of an empty list");
  }
  lval *cdr = lval_copy(lst);
  lval_lst_take(cdr, 0);
  return cdr;
}

lval *
builtin_eval (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG(arg, 1);
  LVAL_LST_ASSERT_TYPE(arg, LVAL_LST);
  lval *lst = lval_copy(lval_lst_nth(arg, 0));
  lval_unquote(lst);
  return lval_eval(env, lst);
}

lval *
builtin_list (lenv *env, lval *arg)
{
  return lval_copy(arg);
}

lval *
builtin_join (lenv *env, lval *arg)
{
  LVAL_LST_ASSERT_TYPE(arg, LVAL_LST);
  lval *lst = lval_lst();
  for (long j = 0; j < lval_lst_length(arg); j++) {
    for (long i = 0; i < lval_lst_length(lval_lst_nth(arg, j)); i++) {
      lval_lst_append(lst, lval_copy(lval_lst_nth(lval_lst_nth(arg, j), i)));
    }
  }
  return lst;
}

lval *
builtin_def (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG_GE(arg, 2);
  LVAL_ASSERT_TYPE(lval_lst_nth(arg, 0), LVAL_SYM);
  lval *sym = lval_lst_nth(arg, 0);
  lval *val = lval_eval(env, lval_lst_take(arg, 1));
  if (val->type == LVAL_ERR) {
    return val;
  }
  lenv_put(env, sym->value, val);
  return val;
}


lval *
builtin_lambda (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG(arg, 2);
  LVAL_ASSERT_TYPE(lval_lst_nth(arg, 0), LVAL_LST);
  lval *args = lval_lst_nth(arg, 0);
  lval *body = lval_lst_nth(arg, 1);
  lval *fun  = lval_fun_userdef(env, args, body);
  return fun;
}

lval *
builtin_quote (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG(arg, 1);
  lval_quote(lval_lst_nth(arg, 0));
  return lval_lst_take(arg, 0);
}

lval *
builtin_if (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG(arg, 3);
  lval *cond = lval_eval(env, lval_lst_take(arg, 0));
  if (cond->type == LVAL_ERR) {
    return cond;
  }
  if (LVAL_IS_NIL(cond)) {
    lval_free(cond);
    return lval_eval(env, lval_lst_take(arg, 1));
  }
  lval_free(cond);
  return lval_eval(env, lval_lst_take(arg, 0));
}

lval *
builtin_gt (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG(arg, 2);
  LVAL_LST_ASSERT_TYPE(arg, LVAL_NUM);
  lval *ord = lval_num_compare(lval_lst_nth(arg, 0), lval_lst_nth(arg, 1));
  if (ord->type == LVAL_ERR) {
    return ord;
  }
  if (LVAL_NUM_VALUE(ord) == 1) {
    lval_free(ord);
    return LVAL_T();
  }
  lval_free(ord);
  return LVAL_NIL();
}

lval *
builtin_lt (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG(arg, 2);
  LVAL_LST_ASSERT_TYPE(arg, LVAL_NUM);
  lval *ord = lval_num_compare(lval_lst_nth(arg, 0), lval_lst_nth(arg, 1));
  if (ord->type == LVAL_ERR) {
    return ord;
  }
  if (LVAL_NUM_VALUE(ord) == -1) {
    lval_free(ord);
    return LVAL_T();
  }
  lval_free(ord);
  return LVAL_NIL();
}

lval *
builtin_le (lenv *env, lval *arg)
{
  lval *ord = builtin_gt(env, arg);
  if (ord->type == LVAL_ERR) {
    return ord;
  }
  if (LVAL_IS_NIL(ord)) {
    lval_free(ord);
    return LVAL_T();
  }
  lval_free(ord);
  return LVAL_NIL();
}

lval *
builtin_ge (lenv *env, lval *arg)
{
  lval *ord = builtin_lt(env, arg);
  if (ord->type == LVAL_ERR) {
    return ord;
  }
  if (LVAL_IS_NIL(ord)) {
    lval_free(ord);
    return LVAL_T();
  }
  lval_free(ord);
  return LVAL_NIL();
}

lval *
builtin_eq (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG(arg, 2);
  LVAL_LST_ASSERT_TYPE(arg, LVAL_NUM);
  lval *ord = lval_num_compare(lval_lst_nth(arg, 0), lval_lst_nth(arg, 1));
  if (ord->type == LVAL_ERR) {
    return ord;
  }
  if (LVAL_NUM_VALUE(ord) == 0) {
    lval_free(ord);
    return LVAL_T();
  }
  lval_free(ord);
  return LVAL_NIL();
}

lval *
builtin_equal (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG(arg, 2);
  lval *a = lval_lst_nth(arg, 0);
  lval *b = lval_lst_nth(arg, 1);
  if (a->type == b->type) {
    switch (a->type) {
    case LVAL_NUM:
      if (LVAL_NUM_VALUE(a) == LVAL_NUM_VALUE(b)) {
        return LVAL_T();
      }
      break;
    case LVAL_STR:
    case LVAL_ERR:
    case LVAL_SYM:
      if (strcmp(a->value, b->value) == 0) {
        return LVAL_T();
      }
      break;
    case LVAL_FUN:
    case LVAL_LST:
      break;
    }
  }
  return LVAL_NIL();
}

lval *
builtin_not (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG(arg, 1);
  if (LVAL_IS_NIL(lval_lst_nth(arg, 0))) {
    return LVAL_T();
  }
  return LVAL_NIL();
}

lval *
builtin_and (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG_GE(arg, 1);
  long length = lval_lst_length(arg);
  lval *val;
  for (long i = 0; i < length; i++) {
    val = lval_eval(env, lval_lst_take(arg, 0));
    if (val->type == LVAL_ERR) {
      break;
    }
    if (LVAL_IS_NIL(val)) {
      break;
    }
    if (i < length - 1) {
      lval_free(val);
    }
  }
  return val;
}

lval *
builtin_or (lenv *env, lval *arg)
{
  LVAL_ASSERT_NUMARG_GE(arg, 1);
  long length = lval_lst_length(arg);
  lval *val;
  for (long i = 0; i < length; i++) {
    val = lval_eval(env, lval_lst_take(arg, 0));
    if (val->type == LVAL_ERR) {
      break;
    }
    if (!LVAL_IS_NIL(val)) {
      break;
    }
    if (i < length - 1) {
      lval_free(val);
    }
  }
  return val;
}



/*
 * Evaluate all members of a list.
 */
lval *
lval_eval_lst (lenv *env, lval *val)
{
  LVAL_ASSERT_TYPE(val, LVAL_LST);
  lval *lst = lval_lst();
  long size = lval_lst_length(val);

  for (long i = 0; i < size; i++) {
    lval *mem = lval_eval(env, lval_lst_take(val, 0));
    if (mem->type == LVAL_ERR) {
      lval_free(lst);
      return mem;
    }
    lval_lst_append(lst, mem);
  }
  return lst;
}

lval *
lval_eval_fun (lenv *env, lval *val)
{
  lval *fun = lval_eval(env, lval_lst_take(val, 0));
  if (fun->type != LVAL_FUN) {
    lval_free(fun);
    return lval_err("Invalid function");
  }

  lval *lst;

  if (lval_fun_is_special(fun)) {
    lst = lval_copy(val);
  } else {
    lst = lval_eval_lst(env, val);
    if (lst->type == LVAL_ERR) {
      lval_free(fun);
      return lst;
    }
  }

  lval *dst = lval_fun_call(env, fun, lst);
  lval_free(lst);
  lval_free(fun);
  return dst;
}

lval *
lval_eval (lenv *env, lval *val)
{
  if (val->type == LVAL_SYM) {
    lval *dst = lenv_get(env, val->value);
    lval_free(val);
    return dst;
  }
  if (val->type == LVAL_LST && lval_lst_length(val) && !lval_is_quoted(val)) {
    lval *dst = lval_eval_fun(env, val);
    lval_free(val);
    return dst;
  }
  lval *dst = lval_copy(val);
  lval_free(val);
  return dst;
}

lval *
read_lval (mpc_ast_t *node)
{
  if (strcmp(node->tag, ">") == 0) {
    return read_lval(node->children[1]);
  }
  if (strstr(node->tag, "string")) {
    char *str = string_substring(node->contents, 1, strlen(node->contents) - 2);
    lval *val = lval_str(str);
    free(str);
    return val;
  }
  if (strstr(node->tag, "number")) {
    return lval_num(atof(node->contents));
  }
  if (strstr(node->tag, "symbol")) {
    if (strcmp(node->contents, "nil") == 0) {
      return LVAL_NIL();
    }
    return lval_sym(node->contents);
  }
  if (strstr(node->tag, "list")) {
    lval *lst = lval_lst();
    if (strcmp(node->children[0]->contents, "{") == 0) {
      lval_quote(lst);
    }
    for (int i = 0; i < node->children_num; i++) {
      if (strcmp(node->children[i]->tag, "char") == 0) {
        continue;
      }
      lval_lst_append(lst, read_lval(node->children[i]));
    }
    return lst;
  }
  return lval_err("Invalid node: %s", node->tag);
}

lval *
builtin_load (lenv *env, lval *val)
{
  LVAL_ASSERT_NUMARG(val, 1);
  LVAL_LST_ASSERT_TYPE(val, LVAL_STR);

  lval *expr;
  char *filename = lval_lst_nth(val, 0)->value;

  lparser *p = lparser_create();
  if (lparser_parse_file(p, filename)) {
    expr = lval_eval(env, read_lval(lparser_ast(p)));
    lparser_ast_delete(p);
  } else {
    char *errmsg = lparser_error(p);
    expr = lval_err("Error loading library %s", errmsg);
    free(errmsg);
  }

  lparser_delete(p);
  return expr;
}
