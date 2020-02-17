CC = gcc
all: parser.tab.o lex.yy.o AST.o AST.c
	$(CC) -Wall -g lex.yy.o parser.tab.o AST.o -o test -lfl
parser.tab.c parser.tab.h: parser.y
	bison -Wno-yacc --defines=parser.tab.c --output=parser.tab.h parser.y
lex.yy.c: scanner.l parser.tab.h
	flex scanner.l
AST.o:  AST.c 
	$(CC) -c AST.c


clean:
	rm parser.tab.*; rm lex.yy.*; rm *.o;