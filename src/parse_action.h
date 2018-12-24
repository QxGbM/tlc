/*
    Tiny Language Compiler (tlc)

    構文解析器下請け関数

    2016年 木村啓二
*/

#ifndef  PARSE_ACTION_H
#define  PARSE_ACTION_H

#include  "ast.h"

extern AST_Node  *act_ID(char *id);
extern AST_Node  *act_const_int(int c);
extern AST_Node  *act_postfix_func(AST_Node *e, AST_List *l);
extern AST_List  *act_argument_list(AST_List *lp, AST_Node *e);
extern AST_Node  *act_unary_expr(int ope, AST_Node *n1);
extern AST_Node  *act_expr_n2(int ope, AST_Node *n1, AST_Node *n2);
extern AST_Node  *act_dec_int(AST_Node *d);
extern AST_Node  *act_ident_list(AST_Node *dec1, AST_Node *dec2);
extern AST_List  *act_param_list(AST_List *lp, AST_Node *e);
extern AST_Node  *act_param_dec(AST_Node *e);
extern AST_Node  *act_compound_stm(AST_List *stm_list);
extern AST_Node  *act_exp_stm(AST_Node *e);
extern AST_Node  *act_if_stm(AST_Node *e, AST_Node *s1, AST_Node *s2);
extern AST_Node  *act_while_stm(AST_Node *e, AST_Node *s);
extern AST_Node  *act_for_stm(AST_Node *e1, AST_Node *e2, AST_Node *e3, AST_Node *s);
extern AST_Node  *act_dowhile_stm(AST_Node *s, AST_Node *e);
/* REPORT3
   ここにアクション関数のプロトタイプ宣言を追加する
*/
extern AST_Node  *act_return_stm(AST_Node *e);
extern AST_List  *act_block_item(AST_Node *s);
extern AST_List  *act_block_item_list(AST_List *l, AST_Node *item);
extern AST_List  *act_unit_list(AST_List *lu, AST_Node *f);
extern AST_Node  *act_function_def(AST_Node *id, AST_List *lp, AST_Node *s);

#endif	/* PARSE_ACTION_H */
