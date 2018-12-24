/*
    Tiny Language Compiler (tlc)

    構文解析器下請け関数

    2016年 木村啓二
*/

#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  "ast.h"
#include  "symtab.h"

extern int yylineno;
extern int yynerrs;

/* 処理中関数のid */
static int current_func_id;

static void append_arg_sym(AST_Node *p);
static void check_stm(AST_Node *s);
static void check_exp(AST_Node *n);

AST_Node*
act_ID(char *id)
{
    char* str;
    AST_Node *ret;
    ret = create_AST_Exp(AST_EXP_IDENT);
    if ((str = strdup(id)) == NULL) {
	fprintf(stderr, "Not enough memory for strdup.\n");
	abort();
    }
    ret->str = str;
    return ret;
}

AST_Node*
act_const_int(int c)
{
    AST_Node *ret = create_AST_Exp(AST_EXP_CNST_INT);
    ret->val = c;
    return ret;
}

AST_Node*
act_postfix_func(AST_Node *e, AST_List *l)
{
    AST_Node *ret = create_AST_Exp(AST_EXP_CALL);
    ret->child[0] = e;
    ret->list = l;
    return ret;
}

AST_List*
act_argument_list(AST_List *lp, AST_Node *e)
{
    AST_List *retl;
    retl = append_AST_List(lp, e);
    return lp == NULL ? retl : lp;
}

AST_Node*
act_unary_expr(int ope, AST_Node *n1)
{
    AST_Node *ret = create_AST_Exp(ope);
    ret->child[0] = n1;
    if (n1 != NULL) {
	n1->parent = ret;
    }
    return ret;
}

AST_Node*
act_expr_n2(int ope, AST_Node *n1, AST_Node *n2)
{
    AST_Node *ret = create_AST_Exp(ope);
    ret->child[0] = n1;
    ret->child[1] = n2;

    if (n1 != NULL) {
	n1->parent = ret;
    }
    if (n2 != NULL) {
	n2->parent = ret;
    }
    return ret;
}

AST_Node*
act_dec_int(AST_Node *d)
{
    AST_Node *n;
    AST_Node *ret = create_AST_Stm(AST_STM_DEC, yylineno);
    for (n = d; n != NULL; n = n->child[0]) {
	if (append_sym(TYPE_INT, SYM_AUTOVAR, n->str) == 0) {
	    fprintf(stderr, "Duplicate variable declaration: %s\n", n->str);
	    yynerrs++;
	}
    }
    ret->child[0] = d;
    if (d != NULL) {
	d->parent = ret;
    }
    return ret;
}

AST_Node*
act_ident_list(AST_Node *dec1, AST_Node *dec2)
{
    if (dec1 == NULL) {
	return dec2;
    } else {
	AST_Node *d;
	for (d = dec1; d->child[0] != NULL; d = d->child[0])
	    ;
	d->child[0] = dec2;
	if (dec2 != NULL) {
	    dec2->parent = d;
	}
	return dec1;
    }
}

AST_List*
act_param_list(AST_List *lp, AST_Node *e)
{
    AST_List *retl;
    retl = append_AST_List(lp, e);
    return lp == NULL ? retl : lp;
}

AST_Node*
act_param_dec(AST_Node *e)
{
    AST_Node *ret;
    
    ret = create_AST_Exp(AST_EXP_PARAM);
    /* Each parameter is registered when the current function is registered. */
    ret->child[0] = e;
    return ret;
}

AST_Node*
act_compound_stm(AST_List *stm_list)
{
    AST_Node *ret = create_AST_Stm(AST_STM_LIST, yylineno);
    ret->list = stm_list;
    if (stm_list != NULL) {
	stm_list->parent = ret;
    }
    return ret;
}

AST_Node*
act_exp_stm(AST_Node *e)
{
    AST_Node *ret = create_AST_Stm(AST_STM_ASIGN, yylineno);
    ret->child[0] = e;
    if (e != NULL) {
	e->parent = ret;
    }
    return ret;
}

AST_Node*
act_if_stm(AST_Node *e, AST_Node *s1, AST_Node *s2)
{
    AST_Node *ret = create_AST_Stm(AST_STM_IF, yylineno);
    ret->child[0] = e;
    ret->child[1] = s1;
    ret->child[2] = s2;
    if (e != NULL) {
	e->parent = ret;
    }
    if (s1 != NULL) {
	s1->parent = ret;
    }
    if (s2 != NULL) {
	s2->parent = ret;
    }
    return ret;
}

AST_Node*
act_while_stm(AST_Node *e, AST_Node *s)
{
    AST_Node *ret = create_AST_Stm(AST_STM_WHILE, yylineno);
    ret->child[0] = e;
    ret->child[1] = s;
    if (e != NULL) {
	e->parent = ret;
    }
    if (s != NULL) {
	s->parent = ret;
    }
    return ret;
}

AST_Node*
act_for_stm(AST_Node *e1, AST_Node *e2, AST_Node *e3, AST_Node *s)
{
    AST_Node *ret = create_AST_Stm(AST_STM_FOR, yylineno);
    ret->child[0] = e1;
    ret->child[1] = e2;
    ret->child[2] = e3;
    ret->child[3] = s;
    if (e1 != NULL) {
	e1->parent = ret;
    }
    if (e2 != NULL) {
	e2->parent = ret;
    }
    if (e3 != NULL) {
	e3->parent = ret;
    }
    if (s != NULL) {
	s->parent = ret;
    }
    return ret;
}

AST_Node*
act_dowhile_stm(AST_Node *s, AST_Node *e)
{
	AST_Node *ret = create_AST_Stm(AST_STM_DOWHILE, yylineno);
    ret->child[0] = s;
    ret->child[1] = e;
    if (s != NULL) {
	s->parent = ret;
    }
    if (e != NULL) {
	e->parent = ret;
    }
    return ret;
}

/* REPORT3
   ここにdo-while文のアクション関数を記述する
*/

AST_Node*
act_return_stm(AST_Node *e)
{
    AST_Node *ret = create_AST_Stm(AST_STM_RETURN, yylineno);
    ret->child[0] = e;
    if (e != NULL) {
	e->parent = ret;
    }
    return ret;
}

AST_List*
act_unit_list(AST_List *lu, AST_Node *f)
{
    AST_List *retl;
    retl = append_AST_List(lu, f);
    return lu == NULL ? retl : lu;
}

void
append_arg_sym(AST_Node *p)
{
    /* Only TYPE_INT is assumed. */
    if (append_sym(TYPE_INT, SYM_ARG, p->child[0]->str) == 0) {
	fprintf(stderr,
		"Duplicate argument declaration: %s\n",	p->child[0]->str);
	yynerrs++;
    }
}

static
void check_stm(AST_Node *s)
{
    if (s->sub_kind != AST_STM_DEC) {
	check_exp(s);
    }
}

static void
check_exp(AST_Node *n)
{
    int i;
    AST_List *l;

    if (n->sub_kind == AST_EXP_IDENT) {
	if ((n->symtab = lookup_sym(0, SYM_VAR, n->str)) == NULL) {
	    fprintf(stderr, "Undeclared variable: %s\n", n->str);
	    yynerrs++;
	}
    }
    TRAVERSE_AST_LIST(l, n->list, check_stm(l->elem));
    if (n->sub_kind != AST_EXP_CALL) {
	for (i = 0; i < AST_NUM_CHILDLEN; i++) {
	    if (n->child[i] != NULL) {
		check_exp(n->child[i]);
	    }
	}
    }
}

AST_Node*
act_function_def(AST_Node *id, AST_List *lp, AST_Node *b)
{
    AST_List *l;
    AST_Node *ret = create_AST_Node(AST_KIND_FUNC, AST_SUB_NONE);

    ret->child[0] = id;
    ret->list = lp;
    ret->child[1] = b;
    if (b != NULL) {
	b->parent = ret;
    }
    /* Only TYPE_INT is assumed. */
    append_sym(TYPE_INT, SYM_FUNC, id->str);
    TRAVERSE_AST_LIST(l, lp, append_arg_sym(l->elem));
    check_exp(b);

    commit_current_symtab(++current_func_id);
    ret->id = current_func_id;
    return ret;
}

AST_List*
act_block_item(AST_Node *s)
{
    return  append_AST_List(NULL, s);
}

AST_List*
act_block_item_list(AST_List *l, AST_Node *item)
{
    AST_List *nl;
    nl = append_AST_List(l, item);
    return l == NULL ? nl : l;
}
