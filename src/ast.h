/*
    Tiny Language Compiler (tlc)

    AST関連

    2016年 木村啓二
*/

#ifndef  AST_H
#define  AST_H

/* ASTの主種別 */
enum {
    AST_KIND_NONE,
    AST_KIND_FUNC,
    AST_KIND_STM,
    AST_KIND_EXP
};

/* ASTの副種別 */
enum {
    AST_SUB_NONE,
    /* 文用の副種別 */
    AST_STM_LIST,
    AST_STM_DEC,
    AST_STM_ASIGN,
    AST_STM_IF,
    AST_STM_WHILE,
    AST_STM_FOR,
    AST_STM_DOWHILE,
    AST_STM_RETURN,
    /* 式用の副種別 */
    AST_EXP_ASGN,
    AST_EXP_IDENT,
    AST_EXP_CNST_INT,
    AST_EXP_PRIME,
    AST_EXP_CALL,
    AST_EXP_PARAM,
    AST_EXP_UNARY_PLUS,
    AST_EXP_UNARY_MINUS,
    AST_EXP_MUL,
    AST_EXP_DIV,
    AST_EXP_ADD,
    AST_EXP_SUB,
    AST_EXP_LT,
    AST_EXP_GT,
    AST_EXP_LTE,
    AST_EXP_GTE,
    AST_EXP_EQ,
    AST_EXP_NE
};

/* 型 */
enum {
    TYPE_NONE,
    TYPE_INT
};

#define  AST_NUM_CHILDLEN  4

typedef struct AST_Node {
    int  kind;		/* 主種別 */
    int  sub_kind;	/* 副種別 */
    int  lineno;
    int  id;
    int  val;		/* AST_EXP_CNST_INTの時の値 */
    char *str;		/* AST_EXP_IDENTの時の文字列 */
    int  reg;		/* 割り付けられたレジスタ */
    int  rank;		/* レジスタ割り付けとコード生成時の巡回優先度 */
    /* AST_EXP_IDENTの時のsymtab（予定） */
    struct AST_List *parent_list;
    struct AST_Node *parent;
    struct AST_Node *child[AST_NUM_CHILDLEN];
    struct AST_List *list;
    struct SymTab   *symtab;
} AST_Node;

/*
 * AST双方向リスト
 */
typedef struct AST_List {
    struct AST_Node *elem;
    struct AST_List *prev;
    struct AST_List *next;
    struct AST_Node *parent;
} AST_List;

/* ASTの根 */
extern AST_List *AST_root;

#define TRAVERSE_AST_LIST(E, BEGIN, PROC) \
    { (E) = (BEGIN); if ((E) != NULL) { do { \
        PROC; \
      (E) = (E)->next; } while ((E) != (BEGIN)); }}

#define REV_TRAVERSE_AST_LIST(E, BEGIN, PROC) \
    { (E) = (BEGIN); if ((E) != NULL) { do { (E) = (E)->prev; \
        PROC; \
      } while ((E) != (BEGIN)); }}

extern AST_Node *create_AST_Node(int kind, int sub_kind);
extern AST_Node *create_AST_Exp(int sub_kind);
extern AST_Node *create_AST_Stm(int sub_kind, int line);

/* リストlにノードnの要素を追加し、追加した要素のポインタを返す。
   lはリストの先頭要素である。また、lはNULLでも良い。 */
extern AST_List *append_AST_List(AST_List *l, AST_Node *n);

extern void dump_ast();

#endif	/* AST_H */
