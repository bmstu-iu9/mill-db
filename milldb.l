%{
#include <iostream>
#include "milldb.tab.h"

// #define PRINT_DEBUG

void print_token(char* token_name) {
#ifdef PRINT_DEBUG
    std::cout << token_name << std::endl;
#endif
}

%}

%option yylineno

LPAREN      "("
RPAREN      ")"
SEMICOLON   ";"
COMMA       ","

CREATE_KEYWORD      ?i:"create"
INSERT_KEYWORD      ?i:"insert"
WRITEPROC_KEYWORD   ?i:"writeproc"
READPROC_KEYWORD    ?i:"readproc"
TABLE_KEYWORD       ?i:"table"
PK_KEYWORD          ?i:"pk"
BEGIN_KEYWORD       ?i:"begin"
END_KEYWORD         ?i:"end"
INDEX_KEYWORD       ?i:"index"
ON_KEYWORD          ?i:"on"

INT_KEYWORD         ?i:"int"
FLOAT_KEYWORD       ?i:"float"
DOUBLE_KEYWORD      ?i:"double"


IDENTIFIER          {IDENTIFIER_START}{IDENTIFIER_PART}*
IDENTIFIER_START    [[:alpha:]]
IDENTIFIER_PART     [[:alnum:]_]

VALUE               {VALUE_PART}+
VALUE_PART          [[:alnum:]]

COMMENT_START       "--"

NEWLINE             \n
WHITESPACE          [ \t\n]+

%x COMMENT
%%
<COMMENT>{NEWLINE}              { print_token("**START INITIAL**"); BEGIN(INITIAL); }
<COMMENT>.                      /* it is comment, skip */
<INITIAL>{COMMENT_START}        { print_token("**START COMMENT**"); BEGIN(COMMENT); }

<INITIAL>{CREATE_KEYWORD}       { print_token("CREATE_KEYWORD"); return CREATE_KEYWORD; }
<INITIAL>{INSERT_KEYWORD}       { print_token("INSERT_KEYWORD"); return INSERT_KEYWORD; }
<INITIAL>{WRITEPROC_KEYWORD}    { print_token("WRITEPROC_KEYWORD"); return WRITEPROC_KEYWORD; }
<INITIAL>{READPROC_KEYWORD}     { print_token("READPROC_KEYWORD"); return READPROC_KEYWORD; }
<INITIAL>{TABLE_KEYWORD}        { print_token("TABLE_KEYWORD"); return TABLE_KEYWORD; }
<INITIAL>{PK_KEYWORD}           { print_token("PK_KEYWORD"); return PK_KEYWORD; }
<INITIAL>{BEGIN_KEYWORD}        { print_token("BEGIN_KEYWORD"); return BEGIN_KEYWORD; }
<INITIAL>{END_KEYWORD}          { print_token("END_KEYWORD"); return END_KEYWORD; }
<INITIAL>{INDEX_KEYWORD}        { print_token("INDEX_KEYWORD"); return INDEX_KEYWORD; }
<INITIAL>{ON_KEYWORD}           { print_token("ON_KEYWORD"); return ON_KEYWORD; }

<INITIAL>{INT_KEYWORD}      { yylval.str = new std::string(yytext); print_token("INT_KEYWORD"); return INT_KEYWORD; }
<INITIAL>{FLOAT_KEYWORD}    { yylval.str = new std::string(yytext); print_token("FLOAT_KEYWORD"); return FLOAT_KEYWORD; }
<INITIAL>{DOUBLE_KEYWORD}   { yylval.str = new std::string(yytext); print_token("DOUBLE_KEYWORD"); return DOUBLE_KEYWORD; }

<INITIAL>{LPAREN}       { print_token("LPAREN"); return LPAREN; }
<INITIAL>{RPAREN}       { print_token("RPAREN"); return RPAREN; }
<INITIAL>{SEMICOLON}    { print_token("SEMICOLON"); return SEMICOLON; }
<INITIAL>{COMMA}        { print_token("COMMA"); return COMMA; }

<INITIAL>{IDENTIFIER}   { yylval.str = new std::string(yytext); print_token("IDENTIFIER"); return IDENTIFIER; }
<INITIAL>{VALUE}        { print_token("VALUE"); return VALUE; }

<INITIAL>{WHITESPACE}   /* eat up whitespace */
<INITIAL>.              { return BAD_CHARACTER; }

%%
