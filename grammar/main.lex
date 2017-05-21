%{
#include <iostream>
#include "main.tab.h"
%}

LPAREN      "("
RPAREN      ")"
SEMICOLON   ";"
COMMA       ","

CREATE_KEYWORD      ?i:"create"
WRITEPROC_KEYWORD   ?i:"writeproc"
READPROC_KEYWORD    ?i:"readproc"
TABLE_KEYWORD       ?i:"table"

IDENTIFIER          {IDENTIFIER_START}{IDENTIFIER_PART}*
IDENTIFIER_START    [[:alpha:]]
IDENTIFIER_PART     [[:alnum:]]

WHITESPACE          [ \t\n]+

%%

{CREATE_KEYWORD}        { return CREATE_KEYWORD; }
{WRITEPROC_KEYWORD}     { return WRITEPROC_KEYWORD; }
{READPROC_KEYWORD}      { return READPROC_KEYWORD; }
{TABLE_KEYWORD}         { return TABLE_KEYWORD; }

{LPAREN}        { return LPAREN; }
{RPAREN}        { return RPAREN; }
{SEMICOLON}     { return SEMICOLON; }
{COMMA}         { return COMMA; }

{IDENTIFIER}       { return IDENTIFIER; }

{WHITESPACE}        /* eat up whitespace */
.                   { std::cout << "Unrecognized character: " << yytext << std::endl; return BAD_CHARACTER; }

%%
