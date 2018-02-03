#ifndef UTIL_H
#define UTIL_H

char * string_unescape (const char *s);
char * string_escape (const char *s);
char * string_substring (const char *s, long start, long length);
char * string_ltrim (char *s);
char * string_rtrim (char *s);
char * string_trim  (char *s);



typedef struct tlist tlist;

tlist * list ();
long    list_length (const tlist *lst);
void  * list_nth    (const tlist *lst, long pos);
void  * list_append (tlist *lst, void *val);
void  * list_insert (tlist *lst, void *val);
void  * list_take   (tlist *lst, long pos);
void    list_free   (tlist *lst);

#endif
