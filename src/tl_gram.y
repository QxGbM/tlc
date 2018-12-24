/*
    Tiny Language Compiler (tlc)

    構文解析部

    2016年 木村啓二
*/

/* 制限事項（バグ）
   文法上は関数の実引数に関数呼び出しを持った式を記述できるが、
   これを正しく処理するコードは生成できない。
   これは、関数のスタック操作中に実引数の式に利用される別関数の
   スタック操作が含まれるためである。
   これを正しく処理するためにはどのような操作が必要か?
 */
%{

#include  <stdio.h>

#include  "ast.h"
#include  "parse_action.h"

extern int  yylex(void);
extern int  yyerror(const char*);
extern int  yylineno;

%}

%union {
    char      *y_str;
    int        y_int;
    AST_Node  *y_AST_Node;
    AST_List  *y_AST_List;
}

%token <y_int>  TOKEN_CONST_INT
%token <y_str>  TOKEN_ID

%token  TOKEN_LEX_ERROR

%token  TOKEN_EQ
%token  TOKEN_PLUS
%token  TOKEN_MINUS
%token  TOKEN_ASTERISK
%token  TOKEN_SLASH
%token  TOKEN_LT
%token  TOKEN_LTE
%token  TOKEN_GTE
%token  TOKEN_GT
%token  TOKEN_EQEQ
%token  TOKEN_NE
%token  TOKEN_COMMA
%token  TOKEN_LPAREN
%token  TOKEN_RPAREN
%token  TOKEN_LBRACE
%token  TOKEN_RBRACE
%token  TOKEN_SEMICOLON

%token  TOKEN_ELSE
%token  TOKEN_FOR
%token  TOKEN_IF
%token  TOKEN_INT
%token  TOKEN_RETURN
%token  TOKEN_WHILE
%token  TOKEN_DO

%type <y_AST_Node> expression
%type <y_AST_Node> identifier
%type <y_AST_Node> primary_expression
%type <y_AST_Node> postfix_expression
%type <y_AST_List> argument_expression_list
%type <y_AST_Node> unary_expression
%type <y_AST_Node> multiplicative_expression
%type <y_AST_Node> additive_expression
%type <y_AST_Node> relational_expression
%type <y_AST_Node> equality_expression
%type <y_AST_Node> assignment_expression
%type <y_AST_Node> identifier_list
%type <y_AST_Node> statement
%type <y_AST_Node> compound_statement
%type <y_AST_Node> expression_statement
%type <y_AST_Node> if_statement
%type <y_AST_Node> iteration_statement
%type <y_AST_Node> return_statement
%type <y_AST_Node> declaration
%type <y_AST_List> parameter_list
%type <y_AST_Node> parameter_declaration
%type <y_AST_List> translation_unit
%type <y_AST_Node> external_declaration
%type <y_AST_Node> function_definition
%type <y_AST_Node> block_item
%type <y_AST_List> block_item_list

%start file
%%
expression
	: assignment_expression
	{ $$ = $1; }

identifier
	: TOKEN_ID
	{ $$ = act_ID($1); }

primary_expression
	: identifier
	{ $$ = $1; }
	| TOKEN_CONST_INT
	{ $$ = act_const_int($1); }
	| TOKEN_LPAREN expression TOKEN_RPAREN
	{ $$ = $2; }

postfix_expression
	: primary_expression
	{ $$ = $1; }
	| identifier TOKEN_LPAREN argument_expression_list TOKEN_RPAREN
	{ $$ = act_postfix_func($1, $3); }
	| identifier TOKEN_LPAREN TOKEN_RPAREN
	{ $$ = act_postfix_func($1, NULL); }
	
argument_expression_list
	: assignment_expression
	{ $$ = act_argument_list(NULL, $1); }
	| argument_expression_list TOKEN_COMMA assignment_expression
	{ $$ = act_argument_list($1, $3); }

unary_expression
	: postfix_expression
	{ $$ = $1; }
	| TOKEN_PLUS unary_expression
	{ $$ = act_unary_expr(AST_EXP_UNARY_PLUS, $2); }
	| TOKEN_MINUS unary_expression
	{ $$ = act_unary_expr(AST_EXP_UNARY_MINUS, $2); }

multiplicative_expression
	: unary_expression
	{ $$ = $1; }
	| multiplicative_expression TOKEN_ASTERISK unary_expression
	{ $$ = act_expr_n2(AST_EXP_MUL, $1, $3); }
	| multiplicative_expression TOKEN_SLASH unary_expression
	{ $$ = act_expr_n2(AST_EXP_DIV, $1, $3); }

additive_expression
	: multiplicative_expression
	{ $$ =$1; }
	| additive_expression TOKEN_PLUS  multiplicative_expression
	{ $$ = act_expr_n2(AST_EXP_ADD, $1, $3); }
	| additive_expression TOKEN_MINUS multiplicative_expression
	{ $$ = act_expr_n2(AST_EXP_SUB, $1, $3); }

relational_expression
	: additive_expression
	{ $$ = $1; }
	| relational_expression TOKEN_LT additive_expression
	{ $$ = act_expr_n2(AST_EXP_LT, $1, $3); }
	| relational_expression TOKEN_GT additive_expression
	{ $$ = act_expr_n2(AST_EXP_GT, $1, $3); }
	| relational_expression TOKEN_LTE additive_expression
	{ $$ = act_expr_n2(AST_EXP_LTE, $1, $3); }
	| relational_expression TOKEN_GTE additive_expression
	{ $$ = act_expr_n2(AST_EXP_GTE, $1, $3); }

equality_expression
	: relational_expression
	{ $$ = $1; }
	| equality_expression TOKEN_EQEQ relational_expression
	{ $$ = act_expr_n2(AST_EXP_EQ, $1, $3); }
	| equality_expression TOKEN_NE relational_expression
	{ $$ = act_expr_n2(AST_EXP_NE, $1, $3); }

assignment_expression
	: equality_expression
	{ $$ = $1; }
	| identifier TOKEN_EQ equality_expression
	{ $$ = act_expr_n2(AST_EXP_ASGN, $1, $3); }

declaration
	: TOKEN_INT identifier_list TOKEN_SEMICOLON
	{ $$ = act_dec_int($2); }

identifier_list
	: identifier
	{ $$ = $1; }
	| identifier_list TOKEN_COMMA identifier
	{ $$ = act_ident_list($1, $3); }

parameter_list
	: parameter_declaration
	{ $$ = act_param_list(NULL, $1); }
	| parameter_list TOKEN_COMMA parameter_declaration
	{ $$ = act_param_list($1, $3); }

parameter_declaration
	: TOKEN_INT identifier
	{ $$ = act_param_dec($2); }

statement
	: compound_statement
	{ $$ = $1; }
	| expression_statement
	{ $$ = $1; }
	| if_statement
	{ $$ = $1; }
	| iteration_statement
	{ $$ = $1; }
	| return_statement
	{ $$ = $1; }

compound_statement
	: TOKEN_LBRACE block_item_list TOKEN_RBRACE
	{ $$ = act_compound_stm($2); }
	| TOKEN_LBRACE TOKEN_RBRACE
	{ $$ = act_compound_stm(NULL); }

block_item_list
	: block_item
	{ $$ = act_block_item($1); }
	| block_item_list block_item
	{ $$ = act_block_item_list($1, $2); }

block_item
	: declaration
	{ $$ = $1; }
	| statement
	{ $$ = $1; }

expression_statement
	: expression TOKEN_SEMICOLON
	{ $$ = act_exp_stm($1); }
	| TOKEN_SEMICOLON
	{ $$ = act_exp_stm(NULL); }

if_statement
	: TOKEN_IF TOKEN_LPAREN expression TOKEN_RPAREN statement
	{ $$ = act_if_stm($3, $5, NULL); }
	| TOKEN_IF TOKEN_LPAREN expression TOKEN_RPAREN statement TOKEN_ELSE statement
	{ $$ = act_if_stm($3, $5, $7); }

iteration_statement
	: TOKEN_WHILE TOKEN_LPAREN expression TOKEN_RPAREN statement
	{ $$ = act_while_stm($3, $5); }
	| TOKEN_FOR TOKEN_LPAREN expression TOKEN_SEMICOLON expression TOKEN_SEMICOLON expression TOKEN_RPAREN statement
	{ $$ = act_for_stm($3, $5, $7, $9); }
	| TOKEN_DO statement TOKEN_WHILE TOKEN_LPAREN expression TOKEN_RPAREN
	{ $$ = act_dowhile_stm($2, $5); }
/** REPORT3
    このあたりにdo-while文のルールを追加する
 */

return_statement
	: TOKEN_RETURN expression TOKEN_SEMICOLON
	{ $$ = act_return_stm($2); }
	| TOKEN_RETURN TOKEN_SEMICOLON
	{ $$ = act_return_stm(NULL); }

translation_unit
	: external_declaration
	{ $$ = act_unit_list(NULL, $1); }
	| translation_unit external_declaration
	{ $$ = act_unit_list($1, $2); }

external_declaration
	: function_definition
	{ $$ = $1; }

function_definition
	: identifier TOKEN_LPAREN parameter_list TOKEN_RPAREN compound_statement
	{ $$ = act_function_def($1, $3, $5); }
	| identifier TOKEN_LPAREN TOKEN_RPAREN compound_statement
	{ $$ = act_function_def($1, NULL, $4); }

file
	: translation_unit
	{ AST_root = $1; }

%%

extern int yynerrs;

int
yyerror(const char *mes)
{
    fprintf(stderr, "[error %d] line %d: %s\n", yynerrs, yylineno, mes);
    return 0;
}
