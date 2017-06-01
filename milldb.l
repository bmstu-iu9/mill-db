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
AT          "@"
EQ          "="

TABLE_KEYWORD       ?i:"table"

CREATE_KEYWORD      ?i:"create"

SELECT_KEYWORD      ?i:"select"
FROM_KEYWORD        ?i:"from"
WHERE_KEYWORD       ?i:"where"

INSERT_KEYWORD      ?i:"insert"
VALUES_KEYWORD      ?i:"values"

PROCEDURE_KEYWORD   ?i:"procedure"
BEGIN_KEYWORD       ?i:"begin"
END_KEYWORD         ?i:"end"
IN_KEYWORD          ?i:"in"
OUT_KEYWORD         ?i:"out"
SET_KEYWORD         ?i:"set"

INDEX_KEYWORD       ?i:"index"
ON_KEYWORD          ?i:"on"

INT_KEYWORD         ?i:"int"
FLOAT_KEYWORD       ?i:"float"
DOUBLE_KEYWORD      ?i:"double"

IDENTIFIER          {IDENTIFIER_START}{IDENTIFIER_PART}*
IDENTIFIER_START    [[:alpha:]]
IDENTIFIER_PART     [[:alnum:]_]

PARAMETER           {AT}{IDENTIFIER}

COMMENT_START       "--"

NEWLINE             \n
WHITESPACE          [ \t\n]+

%x COMMENT
%%
<COMMENT>{NEWLINE}              { print_token("**START INITIAL**"); BEGIN(INITIAL); }
<COMMENT>.                      /* it is comment, skip */
<INITIAL>{COMMENT_START}        { print_token("**START COMMENT**"); BEGIN(COMMENT); }


<INITIAL>{LPAREN}       { print_token("LPAREN"); return LPAREN; }
<INITIAL>{RPAREN}       { print_token("RPAREN"); return RPAREN; }
<INITIAL>{SEMICOLON}    { print_token("SEMICOLON"); return SEMICOLON; }
<INITIAL>{COMMA}        { print_token("COMMA"); return COMMA; }
<INITIAL>{EQ}           { print_token("EQ"); return EQ; }

<INITIAL>{TABLE_KEYWORD}        { print_token("TABLE_KEYWORD"); return TABLE_KEYWORD; }

<INITIAL>{CREATE_KEYWORD}       { print_token("CREATE_KEYWORD"); return CREATE_KEYWORD; }

<INITIAL>{SELECT_KEYWORD}       { print_token("SELECT_KEYWORD"); return SELECT_KEYWORD; }
<INITIAL>{FROM_KEYWORD}         { print_token("FROM_KEYWORD"); return FROM_KEYWORD; }
<INITIAL>{WHERE_KEYWORD}        { print_token("CREATE_KEYWORD"); return WHERE_KEYWORD; }

<INITIAL>{INSERT_KEYWORD}       { print_token("INSERT_KEYWORD"); return INSERT_KEYWORD; }
<INITIAL>{VALUES_KEYWORD}       { print_token("VALUES_KEYWORD"); return VALUES_KEYWORD; }

<INITIAL>{PROCEDURE_KEYWORD}    { print_token("PROCEDURE_KEYWORD"); return PROCEDURE_KEYWORD; }
<INITIAL>{BEGIN_KEYWORD}        { print_token("BEGIN_KEYWORD"); return BEGIN_KEYWORD; }
<INITIAL>{END_KEYWORD}          { print_token("END_KEYWORD"); return END_KEYWORD; }
<INITIAL>{IN_KEYWORD}           { print_token("IN_KEYWORD"); return IN_KEYWORD; }
<INITIAL>{OUT_KEYWORD}          { print_token("OUT_KEYWORD"); return OUT_KEYWORD; }
<INITIAL>{SET_KEYWORD}          { print_token("SET_KEYWORD"); return SET_KEYWORD; }

<INITIAL>{INDEX_KEYWORD}        { print_token("INDEX_KEYWORD"); return INDEX_KEYWORD; }
<INITIAL>{ON_KEYWORD}           { print_token("ON_KEYWORD"); return ON_KEYWORD; }

<INITIAL>{INT_KEYWORD}      { yylval.str = new std::string(yytext); print_token("INT_KEYWORD"); return INT_KEYWORD; }
<INITIAL>{FLOAT_KEYWORD}    { yylval.str = new std::string(yytext); print_token("FLOAT_KEYWORD"); return FLOAT_KEYWORD; }
<INITIAL>{DOUBLE_KEYWORD}   { yylval.str = new std::string(yytext); print_token("DOUBLE_KEYWORD"); return DOUBLE_KEYWORD; }

<INITIAL>{IDENTIFIER}   { yylval.str = new std::string(yytext); print_token("IDENTIFIER"); return IDENTIFIER; }
<INITIAL>{PARAMETER}    { yylval.str = new std::string(yytext); print_token("PARAMETER"); return PARAMETER; }


<INITIAL>{WHITESPACE}   /* eat up whitespace */
<INITIAL>.              { return BAD_CHARACTER; }

%%