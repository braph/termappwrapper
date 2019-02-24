#ifndef _LEXER_H
#define _LEXER_H

#include <stdio.h>

#define LEX_ERROR                        -2

#define LEX_TOKEN_DOUBLE_QUOTE           10
#define LEX_ERROR_MISSING_DOUBLE_QUOTE  -10

#define LEX_TOKEN_SINGLE_QUOTE           11
#define LEX_ERROR_MISSING_SINGLE_QUOTE  -11

#define LEX_TOKEN_WORD                   12
#define LEX_TOKEN_END                    13

int   lex_line;
int   lex_line_pos;

int   lex_init(FILE *);
void  lex_destroy();
int   lex_eof();
int   lex();
char* lex_token();
char* lex_error();

#endif
