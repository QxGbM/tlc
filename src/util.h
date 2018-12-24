/*
    Tiny Language Compiler (tlc)

    メモリ確保等ユーティリティ関数

    2016年 木村啓二
*/

#ifndef  UTIL_H
#define  UTIL_H

#include  <stdlib.h>

extern void *xmalloc(size_t size);
extern void *xcalloc(size_t count, size_t size);
extern void *xrealloc(void *p, size_t size);
extern void xfree(void *ptr);

extern void errexit(const char *mes, const char *file, int line);

#endif	/* UTIL_H */
