#include <string.h>
extern char *strdup (const char *s);

#include "unity/unity.h"
#include "../src/lval.c"

void
test_lenv_put ()
{
  lenv *env = lenv_create(NULL);
  lval *one = lval_err("Error");

  lenv_put(env, "one", one);

  lval_free(one);
  lenv_free(env);
}

void
test_lenv_get ()
{
  lenv *env = lenv_create(NULL);
  lval *one = lval_err("Error");

  lenv_put(env, "one", one);

  lval *two = lenv_get(env, "one");
  TEST_ASSERT_NOT_EQUAL(one, two);
  TEST_ASSERT_EQUAL_STRING(one->value, two->value);

  lval_free(one);
  lval_free(two);
  lenv_free(env);

}

void
test_lenv_get_err ()
{
  lenv *env = lenv_create(NULL);
  lval *err = lenv_get(env, "one");

  TEST_ASSERT_EQUAL(LVAL_ERR, err->type);

  lval_free(err);
  lenv_free(env);
}

void test_lval_sym_leak ()
{
  lval *sym = lval_sym("Symbol");
  lval_free(sym);
}

void test_lval_num_leak ()
{
  lval *num = lval_num(8192);
  lval_free(num);
}

void test_lval_eval_sym ()
{
  lval *num = lval_num(8192);
  lval *val = lval_sym("Symbol");
  lenv *env = lenv_create(NULL);

  val = lval_eval(env, val);
  TEST_ASSERT_EQUAL(LVAL_ERR, val->type);
  lval_free(val);

  val = lval_sym("Symbol");
  lenv_put(env, "Symbol", num);
  val = lval_eval(env, val);
  TEST_ASSERT_EQUAL(LVAL_NUM, val->type);

  lval_free(val);
  lval_free(num);
  lenv_free(env);
}

void
test_lval_eval_lst ()
{
  lval *lst = lval_lst();
  lenv *env = lenv_create(NULL);
  lval *num = lval_num(8192);
  lval *sym = lval_sym("Symbol");

  lenv_put(env, "Symbol", num);
  lval_lst_append(lst, sym);

  lval *dst = lval_eval_lst(env, lst);
  TEST_ASSERT_EQUAL(LVAL_LST, dst->type);
  TEST_ASSERT_EQUAL(1, lval_lst_length(dst));
  TEST_ASSERT_EQUAL(LVAL_NUM, lval_lst_nth(dst, 0)->type);

  lval_free(num);
  lval_free(dst);
  lval_free(lst);
  lenv_free(env);
}

void
test_lval_eval_lst_error ()
{
  lval *lst = lval_lst();
  lenv *env = lenv_create(NULL);
  lval *sym = lval_sym("Symbol");

  lval_lst_append(lst, sym);

  lval *dst = lval_eval_lst(env, lst);
  TEST_ASSERT_EQUAL(LVAL_ERR, dst->type);

  lval_free(lst);
  lval_free(dst);
  lenv_free(env);
}

void
test_builtin_identity ()
{
  lenv *env = lenv_create(NULL);
  lval *fun = lval_fun_builtin(&builtin_identity, 0);
  lval *arg = lval_lst_append(lval_lst(), lval_num(8192));

  lval *num = lval_fun_call(env, fun, arg);
  TEST_ASSERT_EQUAL(LVAL_NUM, num->type);

  lval_free(fun);
  lval_free(arg);
  lval_free(num);
  lenv_free(env);
}

int
main()
{
    UNITY_BEGIN();
    RUN_TEST(test_lenv_put);
    RUN_TEST(test_lenv_get);
    RUN_TEST(test_lenv_get_err);
    RUN_TEST(test_lval_sym_leak);
    RUN_TEST(test_lval_num_leak);
    RUN_TEST(test_lval_eval_sym);
    RUN_TEST(test_lval_eval_lst);
    RUN_TEST(test_lval_eval_lst_error);
    RUN_TEST(test_builtin_identity);
    return UNITY_END();
}
