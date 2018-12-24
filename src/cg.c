/*
    Tiny Language Compiler (tlc)

    レジスタ管理・コード生成

    2016年 木村啓二
*/

#include  <assert.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>

#include  "ast.h"
#include  "cg.h"
#include  "symtab.h"
#include  "util.h"

/*
 * レジスタ割り付け系
 * 方針：
 * 式の構文木で深い方から優先してレジスタを割り付け、レジスタを3つだけ使う
 *
 * 流れ：
 * 1. 各式を深さ優先で探索し、末端から自分までいくつノードがあるかrankに記録する
 * 2. rankの大きい方から優先して探索し、使えるレジスタを割り付ける
 *  いずれも分の巡回までは同じ道筋なので、されぞれpass1/pass2で処理を分ける
 *
 * 簡単のため、式の子は高々2つであることを前提とする
 *
 * 関数呼び出しの際のレジスタの扱い:
 * - 引数を処理する前に（現状では）%eax, %ecx, %edxをスタックに保存し実引数の評価を行う
 * - 戻り値は%eaxに格納されるので、関数ノードに割り当てられたレジスタが%eaxでなければ値をコピーする
 *
 */

#define  MAX_REG_NUM 3

static void traverse_ast_func(AST_Node *f, int pass);
static void traverse_ast_stm(AST_Node *s, int pass);
static void traverse_ast_exp(AST_Node *e, int pass);
static int  ranking_ast_exp(AST_Node *e);
static void assign_ast_exp(AST_Node *e);
static void assign_ast_call(AST_Node *e);
static void assign_ast_exp_body(AST_Node *e, int regs[]);

void
assign_regs(void)
{
    AST_List *l;

    TRAVERSE_AST_LIST(l, AST_root, traverse_ast_func(l->elem, 1));
    TRAVERSE_AST_LIST(l, AST_root, traverse_ast_func(l->elem, 2));
}

void
traverse_ast_func(AST_Node *f, int pass)
{
    traverse_ast_stm(f->child[1], pass);
}

void
traverse_ast_stm(AST_Node *s, int pass)
{
    AST_List *l;

    if (s == NULL) {
	return;
    }
    switch (s->sub_kind) {
    case  AST_STM_LIST:
	TRAVERSE_AST_LIST(l, s->list, traverse_ast_stm(l->elem, pass));
	break;
    case  AST_STM_DEC:
	/* Nothing to do */
	break;
    case  AST_STM_ASIGN:
	traverse_ast_exp(s->child[0], pass);
	break;
    case  AST_STM_IF:
	traverse_ast_exp(s->child[0], pass);
	/* then-statement */
	traverse_ast_stm(s->child[1], pass);
	/* else-statement */
	traverse_ast_stm(s->child[2], pass);
	break;
    case  AST_STM_WHILE:
	traverse_ast_exp(s->child[0], pass);
	traverse_ast_stm(s->child[1], pass);
	break;
    case  AST_STM_FOR:
	traverse_ast_exp(s->child[0], pass);
	traverse_ast_exp(s->child[1], pass);
	traverse_ast_exp(s->child[2], pass);
	traverse_ast_stm(s->child[3], pass);
	break;
	case AST_STM_DOWHILE:
	traverse_ast_stm(s->child[0], pass);
	traverse_ast_exp(s->child[1], pass);
	break;
/* REPORT3
   このあたりにdo-while文ノード用のレジスタ割り付け巡回処理を追加する
*/
    case  AST_STM_RETURN:
	traverse_ast_exp(s->child[0], pass);
	break;
    default:
	errexit("Invalid statement kind", __FILE__, __LINE__);
    }
}

void
traverse_ast_exp(AST_Node *e, int pass)
{
    if (e == NULL) {
	return;
    }
    if (pass == 1) {
	ranking_ast_exp(e);
    } else if (pass == 2) {
	assign_ast_exp(e);
    } else {
	fputs("Illegal register assignemnt pass.\n", stderr);
	abort();
    }
}

int
ranking_ast_exp(AST_Node *e)
{
    int  r0, r1, maxr;
    AST_List *l;
    
    TRAVERSE_AST_LIST(l, e->list, ranking_ast_exp(l->elem));
    r0 = r1 = 0;
    if (e->sub_kind != AST_EXP_CALL && e->child[0] != NULL) {
	r0 = ranking_ast_exp(e->child[0]);
    }
    if (e->child[1] != NULL) {
	r1 = ranking_ast_exp(e->child[1]);
    }
    maxr = r0 >= r1 ? r0 : r1;
    e->rank = maxr+1; /* 末端でもこれでOK */
    return e->rank;
}


void
assign_ast_exp(AST_Node *e)
{
    if (e->sub_kind == AST_EXP_CALL) {
	assign_ast_call(e);
    } else {
	int regs[MAX_REG_NUM];	/* 利用可能レジスタのフラグ */
	memset(regs, 0, sizeof(regs));
	assign_ast_exp_body(e, regs);
    }
}

/*
 * 関数呼び出し前にREGISTERをスタックに保存するのでレジスタ使用状況はリセット
 */
void
assign_ast_call(AST_Node *e)
{
    AST_List *l;
    /* 引き数列の処理 */
    TRAVERSE_AST_LIST(l, e->list, assign_ast_exp(l->elem));
}

void
assign_ast_exp_body(AST_Node *e, int regs[])
{
    int  i, i0, i1, r0, r1;

    r0 = r1 = 0;
    if (e->child[0] != NULL) {
	r0 = e->child[0]->rank;
    }
    if (e->child[1] != NULL) {
	r1 = e->child[1]->rank;
    }
    if (r0 >= r1) {
	i0 = 0; i1 = 1;
    } else {
	i0 = 1; i1 = 0;
    }
    if (r0 != 0 || r1 != 0) { /* 子がある */
	if (e->child[i0] != NULL) {
	    assign_ast_exp_body(e->child[i0], regs);
	}
	if (e->child[i1] != NULL) {
	    assign_ast_exp_body(e->child[i1], regs);
	}
	if (e->child[0] != NULL) {
	    e->reg = e->child[0]->reg;
	}
	if (e->child[1] != NULL) {
	    regs[e->child[1]->reg] = 0;
	}
    } else {
	for (i = 0; i < MAX_REG_NUM; i++) {
	    if (regs[i] == 0) {
		e->reg = i;
		regs[i] = 1;
		break;
	    }
	}
	if (i == MAX_REG_NUM) {
	    fputs("Number of registers is not sufficient.\n", stderr);
	    abort();
	}
    }
}

/*
 * コード生成系
 */

/*
 * ターゲット依存部分
 * MakefileのTARGET_FLAGを適切に選択する
 */
#if defined(TARGET_LINUX) || defined(TARGET_CYGWIN)
#if defined(TARGET_LINUX)
const char MAIN_LABEL[]   = "main";
const char PUTINT_CODE[]  =
    "\t.section\t.rodata\n"
    ".LC0:\n"
    "\t.string \"%d\\n\"\n"
    "\t.text\n"
    "put_int:\n"
    "\tpushl\t%ebp\n"
    "\tmovl\t%esp, %ebp\n"
    "\tsubl\t$24,%esp\n"
    "\tmovl\t$.LC0, %eax\n"
    "\tmovl\t8(%ebp), %edx\n"
    "\tmovl\t%edx, 4(%esp)\n"
    "\tmovl\t%eax, (%esp)\n"
    "\tcall\tprintf\n"
    "\tleave\n"
    "\tret\n";
#elif defined(TARGET_CYGWIN)
const char MAIN_LABEL[]   = "_main";
const char PUTINT_CODE[]  =
    "\t.section\t.rodata\n"
    ".LC0:\n"
    "\t.string \"%d\\n\"\n"
    "\t.text\n"
    "put_int:\n"
    "\tpushl\t%ebp\n"
    "\tmovl\t%esp, %ebp\n"
    "\tsubl\t$24,%esp\n"
    "\tmovl\t$.LC0, %eax\n"
    "\tmovl\t8(%ebp), %edx\n"
    "\tmovl\t%edx, 4(%esp)\n"
    "\tmovl\t%eax, (%esp)\n"
    "\tcall\t_printf\n"
    "\tleave\n"
    "\tret\n";
#endif
const char SECTION_TEXT[] =  "\t.text\n";
const char CALL_OP[]      =  "call";

#elif defined(TARGET_MAC)
const char MAIN_LABEL[]   = "_main";
const char SECTION_TEXT[] = "\t.section\t__TEXT,__text\n";
const char CALL_OP[]      =  "calll";
const char PUTINT_CODE[]  =
    "\t.section\t__TEXT,__cstring\n"
    ".LC0:\n"
    "\t.string \"%d\\n\"\n"
    "\t.section\t__TEXT,__text\n"
    "put_int:\n"
    "\tpushl\t%ebp\n"
    "\tmovl\t%esp, %ebp\n"
    "\tsubl\t$24,%esp\n"
    "\tcalll\tL0$pb\n"
    "L0$pb:\n"
    "\tpopl\t%eax\n"
    "\tmovl\t8(%ebp),%ecx\n"
    "\tmovl\t%ecx, 4(%esp)\n"
    "\tleal\t.LC0-L0$pb(%eax), %eax\n"
    "\tmovl\t%eax, (%esp)\n"
    "\tcalll\t_printf\n"
    "\tleave\n"
    "\tret\n";
#elif defined(TARGET_CYGWIN)
#endif

static void init_label(void);
static void make_func_last_label(AST_Node *f);
static int  get_label(void);
static char *gen_label(int label);
static void gen_label_stm(FILE *out, int label);
static void gen_header(FILE *out);
static void gen_func(FILE *out, AST_Node *f);
static void gen_func_header(FILE *out, char *name, int frame_size);
static void gen_func_footer(FILE *out);
static void gen_put_int(FILE *out);
static void gen_stm(FILE *out, AST_Node *s);
static void gen_stm_asign(FILE *out, AST_Node *s);
static void gen_stm_rel(FILE *out, AST_Node *e, int l_cmp);
static void gen_stm_if(FILE *out, AST_Node *s);
static void gen_stm_while(FILE *out, AST_Node *s);
static void gen_stm_for(FILE *out, AST_Node *s);
static void gen_stm_dowhile(FILE *out, AST_Node *s);
static void gen_stm_return(FILE *out, AST_Node *s);
static void gen_exp(FILE *out, AST_Node *e);
static void gen_exp_asgn(FILE *out, AST_Node *e);
static void gen_exp_cnst(FILE *out, AST_Node *c);
static void gen_exp_ident(FILE *out, AST_Node *idnt);
static void gen_exp_rel(FILE *out, AST_Node *rel);
static void gen_exp_call(FILE *out, AST_Node *e);
static void gen_exp_call_param(FILE *out, AST_Node *p, int offset);
static void gen_exp_n2(FILE *out, AST_Node *e);

static int local_label;		/* 関数内ラベルの番号 */
static char *func_end_label;	/* 関数末尾のラベル */

static char reg_name[][5] = {"%eax", "%ecx", "%edx"};

void
init_label(void)
{
    local_label = 0;
}

/*
 * 関数末尾のラベル
 * func1()のラベルの場合は_END_func1
 */
void
make_func_last_label(AST_Node *f)
{
    char *name;
    int len;
    assert(f->child[0]->sub_kind == AST_EXP_IDENT);
    name = f->child[0]->str;
    len = strlen(name)+6;
    func_end_label = xmalloc(len);
    snprintf(func_end_label, len, "_END_%s", name);
}

int
get_label(void)
{
    return local_label++;
}

char*
gen_label(int label)
{
    static char buf[10];

    snprintf(buf, sizeof(buf), ".L%d", label);
    return buf;
}

void
gen_label_stm(FILE *out, int label)
{
    fprintf(out, "%s:\n", gen_label(label));
}

void
gen_code(FILE *out)
{
    AST_List *l;
    
    gen_header(out);
    init_label();
    TRAVERSE_AST_LIST(l, AST_root, gen_func(out, l->elem));
    gen_put_int(out);
}

void
gen_header(FILE *out)
{
    fprintf(out, "%s", SECTION_TEXT);
}

void
gen_func(FILE *out, AST_Node *f)
{
    AST_List *l;

    assert(f->child[0]->sub_kind == AST_EXP_IDENT);
    make_func_last_label(f);
    gen_func_header(out, f->child[0]->str, get_frame_size(f->id));
    TRAVERSE_AST_LIST(l, f->child[1]->list, gen_stm(out, l->elem));
    gen_func_footer(out);
    free(func_end_label);
    func_end_label = NULL;
}

void
gen_func_header(FILE *out, char *name, int frame_size)
{
    const char *targetn = name;
    int pad;

    /* 整列補正用のpad計算。symtab.cのスタックに関するメモを参照 */
    pad = 16 - (frame_size+8)%16;
    if (pad == 16) {
	pad = 0;
    }
    if (strcmp(name, "main") == 0) {
	targetn = MAIN_LABEL;
    }
    fprintf(out,
	    "\t.globl\t%s\n"
	    "%s:\n", targetn, targetn);
    fputs("\tpushl\t%ebp\n"
	  "\tmovl\t%esp, %ebp\n", out);
    if (frame_size+pad > 0) {
	fprintf(out, "\tsubl\t$%d, %%esp\n", frame_size+pad);
    }
}

void
gen_func_footer(FILE *out)
{
    fprintf(out, "%s:\n"
	    "\tleave\n"
	    "\tret\n\n", func_end_label);
}

void
gen_put_int(FILE *out)
{
    fprintf(out, "%s", PUTINT_CODE);
}

void
gen_stm(FILE *out, AST_Node *s)
{
    AST_List *l;
    
    if (s == NULL) {
	return;
    }
    switch (s->sub_kind) {
    case  AST_STM_LIST:
	TRAVERSE_AST_LIST(l, s->list, gen_stm(out, l->elem));
	break;
    case  AST_STM_DEC:
	/* Nothing to do */
	break;
    case  AST_STM_ASIGN:
	gen_stm_asign(out, s);
	break;
    case  AST_STM_IF:
	gen_stm_if(out, s);
	break;
    case  AST_STM_WHILE:
	gen_stm_while(out, s);
	break;
    case  AST_STM_FOR:
	gen_stm_for(out, s);
	break;
    case  AST_STM_DOWHILE:
	gen_stm_dowhile(out, s);
	break;
    case  AST_STM_RETURN:
	gen_stm_return(out, s);
	break;
    default:
	errexit("Invalid statement kind", __FILE__, __LINE__);
    }
}

void
gen_stm_asign(FILE *out, AST_Node *s)
{
    gen_exp(out, s->child[0]);
}

/*
 * if文やwhile文等の条件式のための適切な条件分岐コードを生成する
 * eは条件式
 * l_cmpは条件が偽だった場合の飛び先ラベル
 */
void
gen_stm_rel(FILE *out, AST_Node *e, int l_cmp)
{
    int  op;

    op = e->sub_kind;
    switch (op) {
    case  AST_EXP_LT:
	fprintf(out, "\tjge\t%s\n", gen_label(l_cmp));
	break;
    case  AST_EXP_GT:
	fprintf(out, "\tjle\t%s\n", gen_label(l_cmp));
	break;
    case  AST_EXP_LTE:
	fprintf(out, "\tjg\t%s\n", gen_label(l_cmp));
	break;
    case  AST_EXP_GTE:
	fprintf(out, "\tjl\t%s\n", gen_label(l_cmp));
	break;
    case  AST_EXP_EQ:
	fprintf(out, "\tjne\t%s\n", gen_label(l_cmp));
	break;
    case  AST_EXP_NE:
	fprintf(out, "\tje\t%s\n", gen_label(l_cmp));
	break;
    default:
	/* "0" stands for "false". */
	fprintf(out, "\tcmpl\t$0,%s\n", reg_name[e->reg]);
	fprintf(out, "\tje\t%s\n", gen_label(l_cmp));
    }
}

void
gen_stm_if(FILE *out, AST_Node *s)
{
    int  l_else = -1, l_end, l_cmp;
    l_cmp = l_end = get_label();
    if (s->child[2] != NULL) { /* else */
	l_cmp = l_else = get_label();
    }

    gen_exp(out, s->child[0]);
    gen_stm_rel(out, s->child[0], l_cmp);
    gen_stm(out, s->child[1]);
    if (s->child[2] != NULL) {
	fprintf(out, "\tjmp\t%s\n", gen_label(l_end));
	gen_label_stm(out, l_else);
	gen_stm(out, s->child[2]);
    }
    gen_label_stm(out, l_end);
}

void
gen_stm_while(FILE *out, AST_Node *s)
{
    int  l_begin, l_exit;
    l_begin = get_label();
    l_exit = get_label();
    gen_label_stm(out, l_begin);
    gen_exp(out, s->child[0]);
    gen_stm_rel(out, s->child[0], l_exit);
    gen_stm(out, s->child[1]);
    fprintf(out, "\tjmp\t%s\n", gen_label(l_begin));
    gen_label_stm(out, l_exit);
}

void
gen_stm_for(FILE *out, AST_Node *s)
{
    int  l_begin, l_exit;
    l_begin = get_label();
    l_exit = get_label();
    gen_exp(out, s->child[0]);
    gen_label_stm(out, l_begin);
    gen_exp(out, s->child[1]);
    gen_stm_rel(out, s->child[1], l_exit);
    gen_stm(out, s->child[3]);
    gen_exp(out, s->child[2]);
    fprintf(out, "\tjmp\t%s\n", gen_label(l_begin));
    gen_label_stm(out, l_exit);
}

void
gen_stm_dowhile(FILE *out, AST_Node *s)
{
    /* REPORT3
       ここにdo-while文のコード生成処理を追加する
    */
	int  l_begin, l_exit;
    l_begin = get_label();
    l_exit = get_label();
    gen_label_stm(out, l_begin);
    gen_stm(out, s->child[0]);
    gen_exp(out, s->child[1]);
	gen_stm_rel(out, s->child[1], l_exit);
    fprintf(out, "\tjmp\t%s\n", gen_label(l_begin));
    gen_label_stm(out, l_exit);
}

void
gen_stm_return(FILE *out, AST_Node *s)
{
    gen_exp(out, s->child[0]);
    if (s->reg != 0) {
	fprintf(out, "\tmovl\t%s, %s\n", reg_name[s->reg], reg_name[0]);
    }
    fprintf(out, "\tjmp\t%s\n", func_end_label);
}

void
gen_exp(FILE *out, AST_Node *e)
{
    if (e == NULL) {
	return;
    }
    if (e->sub_kind == AST_EXP_ASGN) {
	gen_exp_asgn(out, e);
    } else if (e->sub_kind == AST_EXP_IDENT) {
	gen_exp_ident(out, e);
    } else if (e->sub_kind == AST_EXP_CNST_INT) {
	gen_exp_cnst(out, e);
    } else if (e->sub_kind == AST_EXP_CALL) {
	gen_exp_call(out, e);
    } else {
	gen_exp_n2(out, e);
    }
}

void
gen_exp_asgn(FILE *out, AST_Node *e)
{
    gen_exp(out, e->child[1]);
    if (e->child[0]->sub_kind != AST_EXP_IDENT) {
	errexit("Invalid destination operand for assign.", __FILE__, __LINE__);
    }
    fprintf(out, "\tmovl\t%s, %d(%%ebp)\n",
	    reg_name[e->child[1]->reg], e->child[0]->symtab->offset);
}

void
gen_exp_cnst(FILE *out, AST_Node *c)
{
    fprintf(out, "\tmovl\t$%d, %s\n", c->val, reg_name[c->reg]);
}

void
gen_exp_ident(FILE *out, AST_Node *idnt)
{
    fprintf(out, "\tmovl\t%d(%%ebp), %s\n",
	    idnt->symtab->offset, reg_name[idnt->reg]);
}

void
gen_exp_rel(FILE *out, AST_Node *e)
{
    fprintf(out, "\tcmpl\t%s, %s\n",
	    reg_name[e->child[1]->reg], reg_name[e->child[0]->reg]);
    if (e->parent->kind == AST_KIND_STM
	&& (e->parent->sub_kind == AST_STM_IF
	    || e->parent->sub_kind == AST_STM_WHILE
	    || e->parent->sub_kind == AST_STM_FOR)) {
	/* The parent statement generates a branch operation. */
    } else {
	switch (e->sub_kind) {
	case  AST_EXP_LT:
	    fputs("\tsetl\t%al\n", out);
	    break;
	case  AST_EXP_GT:
	    fputs("\tsetg\t%al\n", out);
	    break;
	case  AST_EXP_LTE:
	    fputs("\tsetle\t%al\n", out);
	    break;
	case  AST_EXP_GTE:
	    fputs("\tsetge\t%al\n", out);
	    break;
	case  AST_EXP_EQ:
	    fputs("\tsete\t%al\n", out);
	    break;
	case  AST_EXP_NE:
	    fputs("\tsetne\t%al\n", out);
	    break;
	default:
	    errexit("Invalid relation-op.", __FILE__, __LINE__);
	}
	fprintf(out, "\tmovzbl\t%%al, %s\n", reg_name[e->reg]);
    }
}

/*
   関す呼び出し手順：
   - %espの移動
   - %eax, %ecx, %edxの待避
   - 実引数の評価・スタックに格納
   - call
   - 戻り値(%eax)をノードに割り当てられたレジスタに移動
   - %espを戻す
*/
void
gen_exp_call(FILE *out, AST_Node *e)
{
    int i;
    int psize, fsize, pad;
    AST_List *l;
    
    psize = 0;
    TRAVERSE_AST_LIST(l, e->list, ++psize);
    /* %espの整列補正。symtab.cのスタックに関するメモを参照 */
    pad = (psize+3)%4;
    if (pad == 4) {
	pad = 0;
    }
    pad *= 4; psize *= 4;
    fsize = pad+psize+3*4; /* 実引数+%eax, %ecx, %edx, 全てint(4byte) */

    /* 実引数とpadと待避するレジスタの分だけ%espをずらす */
    fprintf(out, "\tsubl\t$%d, %%esp\n", fsize);
    for (i = 0; i < 3; i++) {
	if (e->reg != i) {
	    fprintf(out, "\tmovl\t%s, %d(%%esp)\n",
		    reg_name[i], psize+12-4*(i+1));
	}
    }
    /* 各実引数は逆順でスタックに格納する
       これは実引数の数が仮引数の数よりも多くても動作するようにするため */
    i = 0;
    REV_TRAVERSE_AST_LIST(l, e->list,
			  gen_exp_call_param(out, l->elem, psize-((i++)+1)*4));
    assert(e->child[0]->sub_kind == AST_EXP_IDENT);
    fprintf(out, "\tcall\t%s\n", e->child[0]->str);
    /* 戻り値の格納 */
    if (e->reg != 0) {
	fprintf(out, "\tmovl\t%s, %s\n", reg_name[0], reg_name[e->reg]);
    }
    /* %espを戻す */
    for (i = 0; i < 3; i++) {
	if (e->reg != i) {
	    fprintf(out, "\tmovl\t%d(%%esp), %s\n",
		    psize+12-4*(i+1), reg_name[i]);
	}
    }
    fprintf(out, "\taddl\t$%d, %%esp\n", fsize);
}

void
gen_exp_call_param(FILE *out, AST_Node *p, int offset)
{
    gen_exp(out, p);
    fprintf(out, "\tmovl\t%s, %d(%%esp)\n", reg_name[p->reg], offset);
}


void
gen_exp_n2(FILE *out, AST_Node *e)
{
    int  i0, i1, r0, r1, src;

    /* レジスタ割り付けと同じ順番で巡回する必要がある */
    r0 = r1 = 0;
    src = e->reg;
    if (e->child[0] != NULL) {
	r0 = e->child[0]->rank;
    }
    if (e->child[1] != NULL) {
	r1 = e->child[1]->rank;
	src = e->child[1]->reg;
    }
    if (r0 >= r1) {
	i0 = 0; i1 = 1;
    } else {
	i0 = 1; i1 = 0;
    }
    if (r0 != 0 || r1 != 0) {
	if (e->child[i0] != NULL) {
	    gen_exp(out, e->child[i0]);
	}
	if (e->child[i1] != NULL) {
	    gen_exp(out, e->child[i1]);
	}
    }
    switch (e->sub_kind) {
    case  AST_EXP_UNARY_PLUS:
	break;			/* nothing to do */
    case  AST_EXP_UNARY_MINUS:
	fprintf(out, "\tnegl\t%s\n", reg_name[e->reg]);
	break;
    case  AST_EXP_MUL:
	fprintf(out, "\timull\t%s, %s\n", reg_name[src], reg_name[e->reg]);
	break;
    case  AST_EXP_DIV:
	/* "div" is not supported now because of its register restriction. */
	fputs("Sorry, div is not suppoted.\n", stderr);
	exit(-1);
	break;
    case  AST_EXP_ADD:
	fprintf(out, "\taddl\t%s, %s\n", reg_name[src], reg_name[e->reg]);
	break;
    case  AST_EXP_SUB:
	fprintf(out, "\tsubl\t%s, %s\n", reg_name[src], reg_name[e->reg]);
	break;
    case  AST_EXP_LT:
    case  AST_EXP_GT:
    case  AST_EXP_LTE:
    case  AST_EXP_GTE:
    case  AST_EXP_EQ:
    case  AST_EXP_NE:
	gen_exp_rel(out, e);
	break;
    default:
	fprintf(stderr, "Unsupported sub_kind %d\n", e->sub_kind);
    }
}

