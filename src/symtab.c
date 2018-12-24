/*
    Tiny Language Compiler (tlc)

    シンボルテーブル

    2016年 木村啓二
*/

#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  "ast.h"
#include  "util.h"
#include  "symtab.h"

/* 現在処理関数のシンボルテーブル */
SymTab current_symtab;

/* 各関数のシンボルテーブルを納める領域のポインタ */
SymTab **symtab_array;

/* 関数名のテーブルの先頭（先頭はダミー） */
SymTab func_symtab;

/* 登録済み関数idの最大値 */
int  max_id;

/* symtab_arrayのサイズ */
int  size_symtab_array;

/* 変数identを型typeで現在処理関数のシンボルテーブルに追加する
   既に登録済みなら0を返す */
int
append_sym(int type, int symkind, char *ident)
{
    int  ok = 1;
    SymTab *t = NULL;

    if (symkind == SYM_NONE) {
	errexit("Illegal symbol kind.\n", __FILE__, __LINE__);
    } else if (symkind == SYM_FUNC) {
	t = &func_symtab;
    } else {
	t = &current_symtab;
    }
    for ( ; t->next != NULL; t = t->next) {
	if (strcmp(ident, t->next->ident) == 0) {
	    ok = 0;
	    break;
	}
    }
    if (ok) {
	t->next = xcalloc(1, sizeof(SymTab));
	t->next->type = type;
	t->next->kind = symkind;
	t->next->entry = t->entry+1;
	if ((t->next->ident = strdup(ident)) == NULL) {
	    fprintf(stderr, "Not enough memory for strdup.\n");
	    abort();
	}
    }
    return ok;
}

/* idで識別される関数のシンボルテーブルより変数identを探す
   idが0の時は現在処理関数
   存在したらそのエントリーのポインタを返す。なければNULL */
SymTab*
lookup_sym(int id, int symkind, char *ident)
{
    SymTab *t;
    if (symkind == SYM_NONE) {
	errexit("Illegal symbol kind.\n", __FILE__, __LINE__);
    }
    if (id == 0) {
	if (symkind == SYM_FUNC) {
	    t = func_symtab.next;
	} else {
	    t = current_symtab.next;
	}
    } else if (id <= max_id) {
	if (symkind == SYM_FUNC) {
	    errexit("Illegal symbol kind (for functions).\n",
		    __FILE__, __LINE__);
	}
	t = symtab_array[id];
    } else {
	fprintf(stderr, "Illegal function id(%d).\n", id);
	abort();
    }
    for (; t != NULL; t = t->next) {
	if (strcmp(t->ident, ident) == 0) {
	    break;
	}
    }
    return t;
}

/* 現在処理関数をid(1以上)で識別される関数のシンボルテーブルとして登録する */
#define CHUNK 10

void
commit_current_symtab(int id)
{
    if (id == 0) {
	fprintf(stderr, "Illegal id number.\n");
	abort();
    }
    if (id >= size_symtab_array) {
	size_symtab_array
	    = (id > size_symtab_array+CHUNK) ? id : size_symtab_array+CHUNK;
	symtab_array
	    = xrealloc(symtab_array, size_symtab_array*sizeof(SymTab*));
    }
    if (max_id < id) {
	max_id = id;
    }
    symtab_array[id] = current_symtab.next;
    current_symtab.next = NULL;
}

/* tlcにおけるx86 (32bit)スタックレイアウトメモ
   （例）
   int func(int a1, int a2, int a3) { int v1, v2, v3; ... }

(higher address)
... |          | %old ebpを16byte整列にするための隙間(pad)
    |   %eax   |
    |   %ecx   |関数呼び出し前に保存 
    |   %edx   |
------------------
+16 |    a3    |
+12 |    a2    |
+8  |    a1    |
+4  |ret. addr.|
+0  | old %ebp | <- %ebp (16byte整列番地-8となるように呼び出される)
-4  |    v1    |
-8  |    v2    |
-12 |    v3    |
... |          | %espを16byte整列にするための隙間(pad)
    |          | <- %esp（変数無しの状態で16byte整列番地-8）
(lower address)

上記の状態を作るため、
- 関数呼び出し直前に%eax,%ecx,%edxと実引数用に16byte整列番地に%espを合わせる
-> パッドの数をPa、実引数の数とNaとすると(Pa+Na+3)*4%16を0にする ((Pa+Na+3)%4を0)
- 関数呼び出し直後にret.addr.とold %ebpと自動変数用に16byte整列に%espを合わせる
-> パッドの数をPv、自動変数の数をNvとすると(Pv+Nv+2)*4%16を0にする ((Pv+Nv+2)%4を0)
*/

/*
  自動変数のオフセット（の絶対値）の最大値を返す
  すでにold %ebpの分のカウントがしてある
  上記の整列補正のためのpad数計算はコード生成側(cg.c)で行う
*/
int
get_frame_size(int id)
{
    int  maxo;
    SymTab *t;
    if (id == 0) {
	t = current_symtab.next;
    } else if (id <= max_id) {
	t = symtab_array[id];
    } else {
	fprintf(stderr, "Illegal function id(%d).\n", id);
	abort();
    }
    maxo = 0;
    for (; t != NULL; t = t->next) {
	if (t->kind == SYM_AUTOVAR && maxo < -t->offset) {
	    maxo = -t->offset;
	}
    }
    return maxo;
}

void
assign_memory(void)
{
    int i;
    int id_arg, id_var;
    SymTab *t;

    id_arg = 1; id_var = 0;
    for (i = 1; i <= max_id; i++) {
	for (t = symtab_array[i]; t != NULL; t = t->next) {
	    /* 変数のサイズはint 4byteで固定 */
	    if (t->kind == SYM_ARG) {
		t->offset = (++id_arg)*4;
	    } else if (t->kind == SYM_AUTOVAR) {
		t->offset = (++id_var)*(-4);
	    }
	}
    }
}

void
dump_symtab(void)
{
    int i;
    SymTab  *t;

    fputs("FuncTab\n", stderr);
    for (t = func_symtab.next; t != NULL; t = t->next) {
	fprintf(stderr, " %s #%d\n", t->ident, t->entry);
    }
    fputs("\nSymTab\n", stderr);
    for (i = 1; i <= max_id; i++) {
	fprintf(stderr, "id(%d)\n", i);
	for (t = symtab_array[i]; t != NULL; t = t->next) {
	    fprintf(stderr, " %s #%d, offset(%d)\n", t->ident, t->entry, t->offset);
	}
    }
}

