CC = gcc
all: parser.tab.o lex.yy.o ast.o ast.c main.o  pretty_printer.o
	$(CC) -Wall -g lex.yy.o parser.tab.o ast.o main.o  pretty_printer.o -o test -lfl
parser.tab.c parser.tab.h: parser.y
	bison -Wno-yacc --defines=parser.tab.c --output=parser.tab.h parser.y
lex.yy.c: scanner.l parser.tab.h
	flex scanner.l
ast.o:  ast.c 
	$(CC) -c ast.c

main.o : main.c
	$(CC) -c main.c

pretty_printer.o : pretty_printer.c
	$(CC) -c pretty_printer.c

clean:
	rm parser.tab.*; rm lex.yy.*; rm *.o; rm test
