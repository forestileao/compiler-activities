CC = gcc
CFLAGS = -Wall
LLVM_CFLAGS = $(shell llvm-config --cflags)
LLVM_LDFLAGS = $(shell llvm-config --ldflags --libs core bitwriter)

all: compiler

compiler: lex.yy.c inter.tab.c command.c symbol_table.c code_generator.c
	$(CC) $(CFLAGS) $(LLVM_CFLAGS) -o compiler lex.yy.c inter.tab.c command.c symbol_table.c code_generator.c -lfl $(LLVM_LDFLAGS)

inter.tab.c inter.tab.h: parser.y
	bison -d parser.y -b inter

lex.yy.c: lexer.l inter.tab.h
	flex lexer.l

clean:
	rm -f compiler lex.yy.c inter.tab.c inter.tab.h
