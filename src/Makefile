CC = gcc
all: parser.tab.o lex.yy.o ast.h ast.o 
	$(CC) -Wall -g lex.yy.o parser.tab.o ast.o -o test -lfl
parser.tab.c parser.tab.h: parser.y
	bison -Wno-yacc --defines=parser.tab.c --output=parser.tab.h parser.y
lex.yy.c: scanner.l parser.tab.h
	flex scanner.l
clean:
	rm parser.tab.*; rm lex.yy.*; rm *.o;
