#ifndef LPARSER_H
#define LPARSER_H

#include "mpc/mpc.h"

typedef struct lparser lparser;

lparser   *lparser_create ();
void       lparser_delete (lparser *p);
int        lparser_parse (lparser *p, const char *s);
int        lparser_parse_file (lparser *p, const char *filename);
mpc_ast_t *lparser_ast (const lparser *p);
void       lparser_ast_delete (const lparser *p);
char      *lparser_error (const lparser *p);

#endif
