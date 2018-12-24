/*
    Tiny Language Compiler (tlc)

    メモリ確保等ユーティリティ関数

    2016年 木村啓二
*/

#include  <stdio.h>
#include  "util.h"

void*
xmalloc(size_t size)
{
    void  *p;

    if ((p = malloc(size)) == NULL) {
	fprintf(stderr, "No room for calloc.\n");
	abort();
    }

    return  p;
}

void*
xcalloc(size_t count, size_t size)
{
    void  *p;

    if ((p = calloc(count, size)) == NULL) {
	fprintf(stderr, "No room for calloc.\n");
	abort();
    }

    return p;
}

void*
xrealloc(void *p, size_t size)
{
    void *np;

    if ((np = realloc(p, size)) == NULL) {
	fprintf(stderr, "No room for realloc.\n");
	abort();
    }

    return np;
}

void
xfree(void *ptr)
{
    free(ptr);
}

void
errexit(const char *mes, const char *file, int line)
{
    fprintf(stderr, "%s (%s : %d)\n", mes, file, line);
    exit(-1);
}

