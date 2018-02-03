#include <string.h>
extern char *strdup (const char *s);

#include "unity/unity.h"
#include "../src/util.c"

void
test_string_escape ()
{
  char *d = NULL;
  char *s = NULL;

  s = strdup("\n");
  d = string_escape(s);
  TEST_ASSERT_EQUAL(2, strlen(d));

  free(s);
  free(d);
}

void
test_string_unescape ()
{
  char *d = NULL;
  char *s = NULL;

  s = strdup("\\x");
  d = string_unescape(s);
  TEST_ASSERT_EQUAL(2, strlen(s));
  TEST_ASSERT_EQUAL_STRING("x", d);
  TEST_ASSERT_EQUAL(1, strlen(d));

  free(s);
  free(d);

  s = strdup("\\n");
  d = string_unescape(s);
  TEST_ASSERT_EQUAL(2, strlen(s));
  TEST_ASSERT_EQUAL_STRING("\n", d);
  TEST_ASSERT_EQUAL(1, strlen(d));

  free(s);
  free(d);
}

void
test_string_substring ()
{
  char *d = NULL;
  char *s = strdup("0123456789");

  d = string_substring(s, 1, strlen(s) - 2);
  TEST_ASSERT_EQUAL_STRING("12345678", d);
  free(d);

  d = string_substring(s, -1, 100);
  TEST_ASSERT_EQUAL_STRING("0123456789", d);
  free(d);

  d = string_substring(s, 100, 100);
  TEST_ASSERT_EQUAL_STRING("", d);
  free(d);

  d = string_substring(s, 0, 0);
  TEST_ASSERT_EQUAL_STRING("", d);
  free(d);

  free(s);
}

void
test_string_ltrim ()
{
  char *s = string_ltrim(strdup("\t foobar\t "));
  TEST_ASSERT_EQUAL_STRING("foobar\t ", s);
  free(s);
}

void
test_string_rtrim ()
{
  char *s = string_rtrim(strdup("\t foobar\t "));
  TEST_ASSERT_EQUAL_STRING("\t foobar", s);
  free(s);
}

void
test_string_trim ()
{
  char *s = string_trim(strdup("\t foobar\t "));
  TEST_ASSERT_EQUAL_STRING("foobar", s);
  free(s);
}

void
test_list_append ()
{
  tlist *lst = list();
  char  *one = strdup("one");
  char  *two = strdup("two");
  list_append(lst, one);
  list_append(lst, two);

  TEST_ASSERT_EQUAL(2, list_length(lst));
  TEST_ASSERT_EQUAL_STRING(one, list_nth(lst, 0));
  TEST_ASSERT_EQUAL_STRING(two, list_nth(lst, 1));

  free(one);
  free(two);
  list_free(lst);
}

void
test_list_insert ()
{
  tlist *lst = list();
  char  *one = strdup("one");
  char  *two = strdup("two");
  list_insert(lst, one);
  list_insert(lst, two);
  TEST_ASSERT_EQUAL(2, list_length(lst));
  TEST_ASSERT_EQUAL_STRING(two, list_nth(lst, 0));
  TEST_ASSERT_EQUAL_STRING(one, list_nth(lst, 1));
  free(one);
  free(two);
  list_free(lst);
}

void
test_list_take ()
{
  tlist *lst = list();
  char  *one = strdup("one");
  char  *two = strdup("two");

  list_append(lst, one);
  list_append(lst, two);
  list_append(lst, one);

  TEST_ASSERT_EQUAL_STRING(two, list_take(lst, 1));
  TEST_ASSERT_EQUAL(2, list_length(lst));

  free(one);
  free(two);
  list_free(lst);
}

int
main()
{
    UNITY_BEGIN();
    RUN_TEST(test_string_substring);
    RUN_TEST(test_string_unescape);
    RUN_TEST(test_string_escape);
    RUN_TEST(test_string_ltrim);
    RUN_TEST(test_string_rtrim);
    RUN_TEST(test_string_trim);
    RUN_TEST(test_list_append);
    RUN_TEST(test_list_insert);
    RUN_TEST(test_list_take);
    return UNITY_END();
}
