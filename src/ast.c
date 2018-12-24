/*
    Tiny Language Compiler (tlc)

    AST関連

    2016年 木村啓二
*/

#include  <stdio.h>
#include  <stdlib.h>
#include  "ast.h"
#include  "util.h"

AST_List *AST_root;

AST_Node*
create_AST_Node(int kind, int sub_kind)
{
    AST_Node *p;

    p = xcalloc(1, sizeof(AST_Node));
    p->kind = kind;
    p->sub_kind = sub_kind;
    return  p;
}

AST_Node*
create_AST_Exp(int sub_kind)
{
    return create_AST_Node(AST_KIND_EXP, sub_kind);
}

AST_Node*
create_AST_Stm(int sub_kind, int line)
{
    AST_Node *s = create_AST_Node(AST_KIND_STM, sub_kind);
    s->lineno = line;
    return s;
}

/* リストlにノードnの要素を追加し、追加した要素のポインタを返す。
   lはリストの先頭要素である。また、lはNULLでも良い。 */
AST_List*
append_AST_List(AST_List *l, AST_Node *n)
{
    AST_List *p;

    p = xcalloc(1, sizeof(AST_List));
    p->elem = n;
    if (n != NULL) {
	n->parent_list = p;
    }
    if (l != NULL) {
	l->prev->next = p;
	p->prev = l->prev;
	l->prev = p;
	p->next = l;
    } else {
	p->prev = p->next = p;
    }

    return p;
}

const char kind_name[][20] = {
    "kind_none",  /* AST_KIND_NONE */
    "func",       /* AST_KIND_FUNC */
    "statement",  /* AST_KIND_STM  */
    "experiment"  /* AST_KIND_EXP  */
};

const char sub_name[][20] = {
    "sub_none",    /* AST_SUB_NONE        */
    "list",        /* AST_STM_LIST        */
    "declaration", /* AST_STM_DEC         */
    "stm_asign",   /* AST_STM_ASIGN       */
    "if",          /* AST_STM_IF          */
    "while",       /* AST_STM_WHILE       */
    "for",         /* AST_STM_FOR         */
    "dowhile",     /* AST_STM_DOWHILE     */
    "return",      /* AST_STM_RETURN      */
    "exp_asign",   /* AST_EXP_ASGN        */
    "identifier",  /* AST_EXP_IDENT       */
    "const_int",   /* AST_EXP_CNST_INT    */
    "prime",       /* AST_EXP_PRIME       */
    "call",	   /* AST_EXP_CALL        */
    "param",	   /* AST_EXP_PARAM       */
    "plus",        /* AST_EXP_UNARY_PLUS  */
    "minus",       /* AST_EXP_UNARY_MINUS */
    "multiply",    /* AST_EXP_MUL         */
    "division",    /* AST_EXP_DIV         */
    "add",         /* AST_EXP_ADD         */
    "sub",         /* AST_EXP_SUB         */
    "lt",          /* AST_EXP_LT          */
    "gt",          /* AST_EXP_GT          */
    "lte",         /* AST_EXP_LTE         */
    "gte",         /* AST_EXP_GTE         */
    "eq",          /* AST_EXP_EQ          */
    "ne"           /* AST_EXP_NE          */
};

/*
 root
 ->  funcs
    -> stms (exp, if, while, for, ...)
      -> exp
*/

static void indent();
static void dump_ast_func(AST_Node *f);
static void dump_ast_stm(AST_Node *s);
static void dump_ast_exp(AST_Node *e);

static int  indent_count;

void
indent()
{
    int i;
    for (i = 0; i < indent_count; i++) {
	fputs(" ", stderr);
    }
}

void
dump_ast()
{
    AST_List *l;

    fputs("root\n", stderr);

    if (AST_root == NULL) {
	errexit("Invalid AST root.\n", __FILE__, __LINE__);
    }
    indent_count++;
    TRAVERSE_AST_LIST(l, AST_root, dump_ast_func(l->elem));
}

void
dump_ast_func(AST_Node *f)
{
    AST_List *l;

    if (f == NULL) {
	return;
    }
    if (f->kind != AST_KIND_FUNC) {
	errexit("function kind is required here.", __FILE__, __LINE__);
    }
    indent();
    fputs("func[", stderr);
    dump_ast_exp(f->child[0]);
    fputs("] (", stderr);
    TRAVERSE_AST_LIST(l, f->list, dump_ast_exp(l->elem));
    fprintf(stderr, ")\n");
    indent_count++;
    TRAVERSE_AST_LIST(l, f->child[1]->list, dump_ast_stm(l->elem));
    indent_count--;
    fputs("\n", stderr);
}

void
dump_ast_stm(AST_Node *s)
{
    AST_List *l;

    if (s == NULL) {
	return;
    }
    indent();
    fprintf(stderr, "l(%d): %s(", s->lineno, sub_name[s->sub_kind]);

    switch (s->sub_kind) {
    case  AST_STM_LIST:
	fputs("\n", stderr);
	indent_count++;
	TRAVERSE_AST_LIST(l, s->list, dump_ast_stm(l->elem));
	indent_count--;
	indent();
	break;
    case  AST_STM_DEC:
	dump_ast_exp(s->child[0]);
	break;
    case  AST_STM_ASIGN:
	dump_ast_exp(s->child[0]);
	dump_ast_exp(s->child[1]);
	break;
    case  AST_STM_IF:
	dump_ast_exp(s->child[0]);
	/* then-statement */
	indent_count++;
	fputs("\n", stderr);
	dump_ast_stm(s->child[1]);
	/* else-statement */
	dump_ast_stm(s->child[2]);
	indent_count--;
	indent();
	break;
    case  AST_STM_WHILE:
	dump_ast_exp(s->child[0]);
	indent_count++;
	fputs("\n", stderr);
	dump_ast_stm(s->child[1]);
	indent_count--;
	indent();
	break;
    case  AST_STM_FOR:
	dump_ast_exp(s->child[0]);
	dump_ast_exp(s->child[1]);
	dump_ast_exp(s->child[2]);
	indent_count++;
	fputs("\n", stderr);
	dump_ast_stm(s->child[3]);
	indent_count--;
	indent();
	break;
	case AST_STM_DOWHILE:
	indent_count++;
	fputs("\n", stderr);
	dump_ast_stm(s->child[0]);
	indent_count--;
	dump_ast_exp(s->child[1]);
	indent();
	break;
	/* REPORT3
	 * このあたりにdo-whileノード用のダンプ処理を追加する
	 */
    case  AST_STM_RETURN:
	dump_ast_exp(s->child[0]);
	break;
    default:
	errexit("Invalid statement kind", __FILE__, __LINE__);
    }
    fputs(")\n", stderr);
}

void
dump_ast_exp(AST_Node *e)
{
    int  i;
    AST_List *l;
    
    if (e == NULL) {
	return;
    }
    fprintf(stderr, " %s(r%d)(", sub_name[e->sub_kind], e->reg);

    if (e->sub_kind == AST_EXP_IDENT) {
	fprintf(stderr, "%s", e->str);
    } else if (e->sub_kind == AST_EXP_CNST_INT) {
	fprintf(stderr, "%d", e->val);
    }
    for (i = 0; i < AST_NUM_CHILDLEN; i++) {
	if (e->child[i] != NULL) {
	    dump_ast_exp(e->child[i]);
	}
    }
    if (e->list != NULL) {
	fputs(" (", stderr);
	TRAVERSE_AST_LIST(l, e->list, dump_ast_exp(l->elem));
	fputs(")", stderr);
    }

    fputs(")", stderr);
}
