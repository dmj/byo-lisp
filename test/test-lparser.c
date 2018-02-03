#include <string.h>
extern char *strdup (const char *s);

#include "unity/unity.h"
#include "../src/lparser.c"

int
main()
{
    UNITY_BEGIN();
    lparser *p = lparser_create();
    lparser_delete(p);
    return UNITY_END();
}
