PROJECT_NAME=milldb

all: lexer parser exec


lexer: $(PROJECT_NAME).l
	flex --header-file=$(PROJECT_NAME).lex.h -o $(PROJECT_NAME).lex.c $(PROJECT_NAME).l

parser: $(PROJECT_NAME).y
	bison -d -o $(PROJECT_NAME).tab.c $(PROJECT_NAME).y -Wno-other

CLASS_SOURCES=env/*.cpp
FLAGS=-lboost_system -lboost_filesystem -std=c++0x -Wno-write-strings -g

exec: lexer parser
	g++ -o ${PROJECT_NAME} main.cpp $(PROJECT_NAME).lex.c $(PROJECT_NAME).tab.c $(CLASS_SOURCES) $(FLAGS)

clear clean:
	rm -f *.tab.? *.lex.? milldb milldb.exe