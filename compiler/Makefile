CC = gcc
CFLAGS = -Wall

all: compiler

compiler: lex.yy.c inter.tab.c command.c symbol_table.c code_generator.c
	$(CC) $(CFLAGS) -o compiler lex.yy.c inter.tab.c command.c symbol_table.c code_generator.c -lfl

inter.tab.c inter.tab.h: parser.y
	bison -d parser.y -b inter

lex.yy.c: lexer.l inter.tab.h
	flex lexer.l

clean:
	rm -f compiler lex.yy.c inter.tab.c inter.tab.h
