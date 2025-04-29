CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -Wall -g

all: compilador

compilador: lex.yy.c y.tab.c
	$(CC) $(CFLAGS) -o compilador lex.yy.c y.tab.c -lm

lex.yy.c: lexer.l
	$(FLEX) lexer.l

y.tab.c: parser.y
	$(BISON) -d -y parser.y

clean:
	rm -f compilador lex.yy.c y.tab.c y.tab.h *~

