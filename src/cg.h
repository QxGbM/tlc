/*
    Tiny Language Compiler (tlc)

    レジスタ管理・コード生成

    2016年 木村啓二
*/

#ifndef  CG_H
#define  CG_H

#include  <stdio.h>

extern void  assign_regs(void);
extern void  gen_code(FILE *out);

#endif	/* CG_H */
