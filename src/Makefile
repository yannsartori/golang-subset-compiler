CC = gcc
all: parser.tab.o lex.yy.o ast.h ast.o pretty_printer.o main.o 
	$(CC) -Wall -g lex.yy.o parser.tab.o ast.o pretty_printer.o main.o -o golitec -lfl
parser.tab.c parser.tab.h: parser.y
	bison -Wno-yacc --defines=parser.tab.c --output=parser.tab.h parser.y
lex.yy.c: scanner.l parser.tab.h
	flex scanner.l
clean:
	rm parser.tab.*; rm lex.yy.*; rm *.o; rm golitec;
