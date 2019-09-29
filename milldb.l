%{
#include <iostream>
#include "milldb.tab.h"

// #define DEBUG

void debugl(const char* token_name) {
#ifdef DEBUG
	std::cout << token_name << std::endl;
#endif
}

%}

%option yylineno

LPAREN		"("
RPAREN		")"
SEMICOLON	";"
COMMA		","
AT			"@"
EQ			"="

TABLE_KEYWORD		?i:"table"
JOIN_KEYWORD		?i:"join"

SEQUENCE_KEYWORD	?i:"sequence"
NEXTVAL_KEYWORD		?i:"nextval"
CURRVAL_KEYWORD		?i:"currval"

CREATE_KEYWORD		?i:"create"
PK_KEYWORD          ?i:"pk"

SELECT_KEYWORD		?i:"select"
FROM_KEYWORD		?i:"from"
WHERE_KEYWORD		?i:"where"

INSERT_KEYWORD		?i:"insert"
VALUES_KEYWORD		?i:"values"

PROCEDURE_KEYWORD	?i:"procedure"
BEGIN_KEYWORD		?i:"begin"
END_KEYWORD			?i:"end"
IN_KEYWORD			?i:"in"
OUT_KEYWORD			?i:"out"
SET_KEYWORD			?i:"set"
AND_KEYWORD			?i:"and"

INDEX_KEYWORD		?i:"index"
ON_KEYWORD			?i:"on"

INT_KEYWORD			?i:"int"
FLOAT_KEYWORD		?i:"float"
DOUBLE_KEYWORD		?i:"double"
CHAR_KEYWORD		?i:"char"
TEXT_KEYWORD        ?i:"text"

IDENTIFIER			{IDENTIFIER_START}{IDENTIFIER_PART}*
IDENTIFIER_START	[[:alpha:]]
IDENTIFIER_PART		[[:alnum:]_]

PARAMETER			{AT}{IDENTIFIER}

INTEGER             {DIGIT}+
DIGIT               [[:alnum:]]

COMMENT_START		"--"

NEWLINE				\n
WHITESPACE			[ \t\n]+

%x COMMENT
%%
<COMMENT>{NEWLINE}				{ debugl("**START INITIAL**"); BEGIN(INITIAL); }
<COMMENT>.						/* it is comment, skip */
<INITIAL>{COMMENT_START}		{ debugl("**START COMMENT**"); BEGIN(COMMENT); }


<INITIAL>{LPAREN}		{ debugl("LPAREN"); return LPAREN; }
<INITIAL>{RPAREN}		{ debugl("RPAREN"); return RPAREN; }
<INITIAL>{SEMICOLON}	{ debugl("SEMICOLON"); return SEMICOLON; }
<INITIAL>{COMMA}		{ debugl("COMMA"); return COMMA; }
<INITIAL>{EQ}			{ debugl("EQ"); return EQ; }

<INITIAL>{TABLE_KEYWORD}		{ debugl("TABLE_KEYWORD"); return TABLE_KEYWORD; }

<INITIAL>{SEQUENCE_KEYWORD}		{ debugl("SEQUENCE_KEYWORD"); return SEQUENCE_KEYWORD; }
<INITIAL>{NEXTVAL_KEYWORD}		{ debugl("NEXTVAL_KEYWORD"); return NEXTVAL_KEYWORD; }
<INITIAL>{CURRVAL_KEYWORD}		{ debugl("CURRVAL_KEYWORD"); return CURRVAL_KEYWORD; }

<INITIAL>{CREATE_KEYWORD}		{ debugl("CREATE_KEYWORD"); return CREATE_KEYWORD; }
<INITIAL>{PK_KEYWORD}		    { debugl("PK_KEYWORD"); return PK_KEYWORD; }
<INITIAL>{JOIN_KEYWORD}		    { debugl("JOIN_KEYWORD"); return JOIN_KEYWORD; }

<INITIAL>{SELECT_KEYWORD}		{ debugl("SELECT_KEYWORD"); return SELECT_KEYWORD; }
<INITIAL>{FROM_KEYWORD}			{ debugl("FROM_KEYWORD"); return FROM_KEYWORD; }
<INITIAL>{WHERE_KEYWORD}		{ debugl("CREATE_KEYWORD"); return WHERE_KEYWORD; }

<INITIAL>{INSERT_KEYWORD}		{ debugl("INSERT_KEYWORD"); return INSERT_KEYWORD; }
<INITIAL>{VALUES_KEYWORD}		{ debugl("VALUES_KEYWORD"); return VALUES_KEYWORD; }

<INITIAL>{PROCEDURE_KEYWORD}	{ debugl("PROCEDURE_KEYWORD"); return PROCEDURE_KEYWORD; }
<INITIAL>{BEGIN_KEYWORD}		{ debugl("BEGIN_KEYWORD"); return BEGIN_KEYWORD; }
<INITIAL>{END_KEYWORD}			{ debugl("END_KEYWORD"); return END_KEYWORD; }
<INITIAL>{IN_KEYWORD}			{ debugl("IN_KEYWORD"); return IN_KEYWORD; }
<INITIAL>{OUT_KEYWORD}			{ debugl("OUT_KEYWORD"); return OUT_KEYWORD; }
<INITIAL>{SET_KEYWORD}			{ debugl("SET_KEYWORD"); return SET_KEYWORD; }
<INITIAL>{AND_KEYWORD}			{ debugl("AND_KEYWORD"); return AND_KEYWORD; }

<INITIAL>{INT_KEYWORD}		{ debugl("INT_KEYWORD"); return INT_KEYWORD; }
<INITIAL>{FLOAT_KEYWORD}	{ debugl("FLOAT_KEYWORD"); return FLOAT_KEYWORD; }
<INITIAL>{DOUBLE_KEYWORD}	{ debugl("DOUBLE_KEYWORD"); return DOUBLE_KEYWORD; }
<INITIAL>{CHAR_KEYWORD}     { debugl("CHAR_KEYWORD"); return CHAR_KEYWORD; }
<INITIAL>{TEXT_KEYWORD}     { debugl("TEXT_KEYWORD"); return TEXT_KEYWORD; }

<INITIAL>{IDENTIFIER}	{ yylval.char_arr = yytext; debugl("IDENTIFIER"); return IDENTIFIER; }
<INITIAL>{PARAMETER}	{ yylval.char_arr = yytext + sizeof(char); debugl("PARAMETER"); return PARAMETER; }
<INITIAL>{INTEGER}	    { yylval.char_arr = yytext; debugl("INTEGER"); return INTEGER; }


<INITIAL>{WHITESPACE}	/* eat up whitespace */
<INITIAL>.				{ return BAD_CHARACTER; }

%%
