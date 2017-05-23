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
STR_KEYWORD         ?i:"str"

IDENTIFIER          {IDENTIFIER_START}{IDENTIFIER_PART}*
IDENTIFIER_START    [[:alpha:]]
IDENTIFIER_PART     [[:alnum:]_]

VALUE               {VALUE_PART}+
VALUE_PART          [[:alnum:]]

WHITESPACE          [ \t\n]+

%%

{CREATE_KEYWORD}        { print_token("CREATE_KEYWORD"); return CREATE_KEYWORD; }
{INSERT_KEYWORD}        { print_token("INSERT_KEYWORD"); return INSERT_KEYWORD; }
{WRITEPROC_KEYWORD}     { print_token("WRITEPROC_KEYWORD"); return WRITEPROC_KEYWORD; }
{READPROC_KEYWORD}      { print_token("READPROC_KEYWORD"); return READPROC_KEYWORD; }
{TABLE_KEYWORD}         { print_token("TABLE_KEYWORD"); return TABLE_KEYWORD; }
{PK_KEYWORD}            { print_token("PK_KEYWORD"); return PK_KEYWORD; }
{BEGIN_KEYWORD}         { print_token("BEGIN_KEYWORD"); return BEGIN_KEYWORD; }
{END_KEYWORD}           { print_token("END_KEYWORD"); return END_KEYWORD; }
{INDEX_KEYWORD}         { print_token("END_KEYWORD"); return INDEX_KEYWORD; }
{ON_KEYWORD}            { print_token("END_KEYWORD"); return ON_KEYWORD; }

{INT_KEYWORD}            { print_token("END_KEYWORD"); return INT_KEYWORD; }
{FLOAT_KEYWORD}          { print_token("END_KEYWORD"); return FLOAT_KEYWORD; }
{DOUBLE_KEYWORD}         { print_token("END_KEYWORD"); return DOUBLE_KEYWORD; }
{STR_KEYWORD}            { print_token("END_KEYWORD"); return STR_KEYWORD; }

{LPAREN}        { print_token("LPAREN"); return LPAREN; }
{RPAREN}        { print_token("RPAREN"); return RPAREN; }
{SEMICOLON}     { print_token("SEMICOLON"); return SEMICOLON; }
{COMMA}         { print_token("COMMA"); return COMMA; }

{IDENTIFIER}    { print_token("IDENTIFIER"); return IDENTIFIER; }
{VALUE}         { print_token("VALUE"); return VALUE; }

{WHITESPACE}    /* eat up whitespace */
.               { return BAD_CHARACTER; }

%%
