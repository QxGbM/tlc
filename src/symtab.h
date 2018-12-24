/*
    Tiny Language Compiler (tlc)

    シンボルテーブル

    2016年 木村啓二
*/

#ifndef  SYMTAB_H
#define  SYMTAB_H

/* 変数の種別 */
enum {
    SYM_NONE,
    SYM_FUNC,			/* 関数   */
    SYM_VAR,			/* 変数全般（仮引数+自動変数） */
    SYM_ARG,			/* 仮引数 */
    SYM_AUTOVAR			/* 自動変数 */
};

typedef struct SymTab {
    int  entry;   /* エントリー番号 */
    int  kind;    /* 変数種別 */
    int  offset;  /* メモリ領域（現在はスタックフレーム）中のオフセット */
    int  type;	  /* 変数型（現在はintのみ) */
    char  *ident; /* 変数名 */
    struct SymTab *next;
} SymTab;

/* 変数identを型typeで現在処理関数のシンボルテーブルに追加する
   既に登録済みなら0を返す */
extern  int  append_sym(int type, int symkind, char *ident);

/* idで識別される関数のシンボルテーブルより変数identを探す
   idが0の時は現在処理関数
   存在したらそのエントリーのポインタを返す。なければNULL */
extern  SymTab  *lookup_sym(int id, int symkind, char *ident);

/* 現在処理関数をid(1以上)で識別される関数のシンボルテーブルとして登録する */
extern  void commit_current_symtab(int id);

/* 読み出された関数で必要とするスタックフレームのサイズを返す */
extern  int get_frame_size(int id);

/* メモリの割り付け */
extern  void assign_memory(void);

extern  void dump_symtab(void);

#endif	/* SYMTAB_H */
