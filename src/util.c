/**
 *
 * Utility functions.
 *
 */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

long
min (long a, long b)
{
  return (a < b) ? a : b;
}

long
max (long a, long b)
{
  return (a > b) ? a : b;
}



char *
string_escape (const char *s)
{
  char *d = malloc(2 * strlen(s) + 1);
  long dl = 0;
  for (long sl = 0; sl < strlen(s); sl++) {
    switch (s[sl]) {
    case '\a':
    case '\b':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
      d[dl] = '\\';
      dl++;
    default: d[dl] = s[sl]; break;
    }
    dl++;
  }
  d[dl] = 0;
  d = realloc(d, 1 + dl);
  return d;
}

char *
string_unescape (const char *s)
{
  char *d = malloc(1 + strlen(s));
  long dl = 0;

  for (long sl = 0; sl < strlen(s); sl++) {
    if (s[sl] != '\\' || sl == strlen(s) - 1) {
      d[dl] = s[sl];
    } else {
      switch (s[sl  + 1]) {
      case 'a': d[dl] = '\a'; break;
      case 'b': d[dl] = '\b'; break;
      case 'f': d[dl] = '\f'; break;
      case 'n': d[dl] = '\n'; break;
      case 'r': d[dl] = '\r'; break;
      case 't': d[dl] = '\t'; break;
      case 'v': d[dl] = '\v'; break;
      default:
        d[dl] = s[sl + 1];
        break;
      }
      sl++;
    }
    dl++;
  }

  d[dl] = 0;
  d = realloc(d, 1 + dl);

  return d;
}

char *
string_substring (const char *s, long start, long length)
{
  start  = min(max(start, 0), strlen(s));
  length = min(max(length, 0), strlen(s) - start);
  char *d = malloc(1 + length);
  memcpy(d, &s[start], length);
  d[length] = 0;
  return d;
}

char *
string_ltrim (char *s)
{
  if (s && strlen(s)) {
    int length = strlen(s);
    int offset = 0;
    while (isspace(s[offset]) && offset < length) {
      offset++;
    }
    if (offset) {
      memmove(s, &s[offset], length - offset);
      s[length - offset] = 0;
      s = realloc(s, 1 + strlen(s));
    }
  }
  return s;
}

char *
string_rtrim (char *s)
{
  if (s && strlen(s)) {
    int length = strlen(s);
    while (isspace(s[length - 1]) && length > 0) {
      length--;
    }
    s[length] = 0;
    s = realloc(s, 1 + strlen(s));
  }
  return s;
}

char *
string_trim (char *s)
{
  return string_rtrim(string_ltrim(s));
}



#define LIST_RESIZE(_l_) _l_->member = realloc(_l_->member, _l_->length * sizeof(void*));

typedef struct tlist {
  void **member;
  long   length;
} tlist;

tlist *
list ()
{
  tlist *lst = malloc(sizeof(tlist));
  lst->member = NULL;
  lst->length = 0;
  return lst;
}

long
list_length (const tlist *lst)
{
  return lst->length;
}

void *
list_nth (const tlist *lst, long pos)
{
  if (pos >= lst->length) {
    return NULL;
  }
  return lst->member[pos];
}

void
list_append (tlist *lst, void *val)
{
  lst->length++;
  LIST_RESIZE(lst);
  lst->member[lst->length - 1] = val;
}

void
list_insert (tlist *lst, void *val)
{
  lst->length++;
  LIST_RESIZE(lst);
  memmove(&lst->member[1], lst->member, sizeof(void*) * (lst->length -1 ));
  lst->member[0] = val;
}

void *
list_take (tlist *lst, long pos)
{
  if (pos >= lst->length) {
    return NULL;
  }
  void *val = lst->member[pos];

  // If it is not the last element we have to move all following
  // elements one step to the left.
  if (pos < lst->length - 1) {
    memmove(&lst->member[pos], &lst->member[pos + 1], sizeof(void*) * (lst->length - pos - 1));
  }

  lst->length--;
  LIST_RESIZE(lst);
  return val;
}

void
list_free (tlist *lst)
{
  free(lst->member);
  free(lst);
}
