DEV_DIR=grammar/
GEN_DIR=gen/
SOURCE_NAME=grammar
FLEX_SOURCE=$(SOURCE_NAME).lex
FLEX_OUTPUT=$(SOURCE_NAME).lex.c
BISON_SOURCE=$(SOURCE_NAME).y
BISON_OUTPUT=$(SOURCE_NAME).tab.c
OUTPUT=milldb

all: folders lexer parser exec


folders:
	mkdir -p $(DEV_DIR)
	mkdir -p $(DEV_DIR)$(GEN_DIR)

lexer: $(DEV_DIR)$(FLEX_SOURCE)
	flex -o $(DEV_DIR)$(GEN_DIR)$(FLEX_OUTPUT) $(DEV_DIR)$(FLEX_SOURCE)

parser: $(DEV_DIR)${BISON_SOURCE}
	bison -o $(DEV_DIR)${GEN_DIR}${BISON_OUTPUT} -vd $(DEV_DIR)${BISON_SOURCE}

exec: folders lexer parser
	g++ -o ${OUTPUT} main.cpp -lfl -Wno-write-strings
