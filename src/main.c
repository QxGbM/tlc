/*
    Tiny Language Compiler (tlc)

    2016年 木村啓二
*/

#include  <libgen.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  "ast.h"
#include  "cg.h"
#include  "symtab.h"

extern FILE  *yyin;
extern int   yynerrs;
extern void  yyparse(void);

int
main(int argc, char **argv)
{
    char *in_file, *out_file;
    int  fnlen;
    FILE *out;

    in_file = argv[1];
    if ((yyin = fopen(in_file, "r")) == NULL) {
	fprintf(stderr, "Can't open the input file %s.\n", in_file);
	exit(-1);
    }
    if ((out_file = strdup(basename(in_file))) == NULL) {
	fputs("Not enough memory for strdup.\n", stderr);
	exit(-1);
    }
    fnlen = strlen(out_file);
    if (strcmp(&out_file[fnlen-2], ".c") != 0) {
	fputs("Illegal suffix.\n", stderr);
	exit(-1);
    }
    out_file[fnlen-1] = 's';
    if ((out = fopen(out_file, "w")) == NULL) {
	fprintf(stderr, "Can't open the output file %s.\n", out_file);
	exit(-1);
    }

    yyparse();
    if (yynerrs > 0) {
	exit(-1);
    }
    assign_memory();
    assign_regs();

    dump_symtab();
    dump_ast();

    gen_code(out);

    return 0;
}
