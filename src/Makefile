CC = gcc
all: parser.tab.o lex.yy.o AST.o AST.c main.o
	$(CC) -Wall -g lex.yy.o parser.tab.o AST.o main.o -o test -lfl
parser.tab.c parser.tab.h: parserWithConstructors.y
	bison -Wno-yacc --defines=parser.tab.c --output=parser.tab.h parserWithConstructors.y
lex.yy.c: scanner.l parser.tab.h
	flex scanner.l
AST.o:  AST.c 
	$(CC) -c AST.c

main.o : main.c
	$(CC) -c main.c

clean:
	rm parser.tab.*; rm lex.yy.*; rm *.o;