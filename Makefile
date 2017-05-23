GEN_DIR=gen/
SOURCE_NAME=milldb
FLEX_SOURCE=$(SOURCE_NAME).lex
FLEX_OUTPUT=$(SOURCE_NAME).lex.c
BISON_SOURCE=$(SOURCE_NAME).y
BISON_OUTPUT=$(SOURCE_NAME).tab.c
OUTPUT=milldb

all: folders lexer parser exec


folders:
	mkdir -p $(GEN_DIR)

lexer: $(FLEX_SOURCE)
	flex -o $(GEN_DIR)$(FLEX_OUTPUT) $(FLEX_SOURCE)

parser: lexer $(BISON_SOURCE)
	bison -o ${GEN_DIR}${BISON_OUTPUT} -vd ${BISON_SOURCE}

exec: folders lexer parser
	g++ -o ${OUTPUT} main.cpp -lfl -Wno-write-strings
