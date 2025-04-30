CC = gcc
CFLAGS = -Wall

all: inter

inter: lex.yy.c inter.tab.c
	$(CC) $(CFLAGS) -o inter lex.yy.c inter.tab.c -lfl

inter.tab.c: parser.y
	bison -d parser.y -b inter

lex.yy.c: lexer.l inter.tab.h
	flex lexer.l

inter.tab.h: parser.y
	bison -d parser.y -b inter

clean:
	rm -f inter lex.yy.c inter.tab.c inter.tab.h
