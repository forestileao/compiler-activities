CC = gcc
CFLAGS = -Wall

all: inter

inter: lex.yy.c inter.tab.c command.c symbol_table.c
	$(CC) $(CFLAGS) -o inter lex.yy.c inter.tab.c command.c symbol_table.c -lfl

inter.tab.c inter.tab.h: parser.y
	bison -d parser.y -b inter

lex.yy.c: lexer.l inter.tab.h
	flex lexer.l

clean:
	rm -f inter lex.yy.c inter.tab.c inter.tab.h
