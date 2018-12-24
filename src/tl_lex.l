%{
/*
    Tiny Language Compiler (tlc)

    字句解析部

    2016年 木村啓二
*/

#include  <stdio.h>
#include  <stdlib.h>

#include  "ast.h"
#include  "tl_gram.h"

%}

%option yylineno
%%

"="    return  TOKEN_EQ;
"+"    return  TOKEN_PLUS;
"-"    return  TOKEN_MINUS;
"*"    return  TOKEN_ASTERISK;
"/"    return  TOKEN_SLASH;
"<"    return  TOKEN_LT;
"<="   return  TOKEN_LTE;
">="   return  TOKEN_GTE;
">"    return  TOKEN_GT;
"=="   return  TOKEN_EQEQ;
"!="   return  TOKEN_NE;
","    return  TOKEN_COMMA;
"("    return  TOKEN_LPAREN;
")"    return  TOKEN_RPAREN;
"{"    return  TOKEN_LBRACE;
"}"    return  TOKEN_RBRACE;
";"    return  TOKEN_SEMICOLON;

"else"  return  TOKEN_ELSE;
"for"   return  TOKEN_FOR;
"if"    return  TOKEN_IF;
"int"   return  TOKEN_INT;
"return"   return  TOKEN_RETURN;
"while" return  TOKEN_WHILE;
"do"    return  TOKEN_DO;

[0-9]+  {
            char *endp;

            yylval.y_int = strtoul(yytext, &endp, 10);
            if (*endp != '\0') {
                fprintf(stderr, "integer out of range error %s\n", endp);
                exit(-1);
            }
            return  TOKEN_CONST_INT;
        }

[a-zA-Z][_a-zA-Z0-9]* {
            yylval.y_str = yytext;
            return  TOKEN_ID;
        }

[ \t\r\n] ;

.        return  TOKEN_LEX_ERROR;

%%
