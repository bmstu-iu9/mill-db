PROJECT_NAME=milldb

all: lexer parser exec


lexer: $(FLEX_SOURCE)
	flex --header-file=$(PROJECT_NAME).lex.h -o $(PROJECT_NAME).lex.c $(PROJECT_NAME).l

parser: $(BISON_SOURCE)
	bison -d -o $(PROJECT_NAME).tab.c $(PROJECT_NAME).y

exec: lexer parser
	g++ -o ${PROJECT_NAME} main.cpp $(PROJECT_NAME).lex.c $(PROJECT_NAME).tab.c Environment.cpp Table.cpp Column.cpp -lboost_system -lboost_filesystem -std=c++11 -lfl -Wno-write-strings