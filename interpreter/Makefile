CC = gcc
CFLAGS = -Wall -g
FLEX = flex
BISON = bison -d

TARGET = interpreter
SRCS = symbol_table.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): lex.yy.o parser.tab.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

lex.yy.c: lexer.l parser.tab.h
	$(FLEX) $<

parser.tab.c parser.tab.h: parser.y
	$(BISON) $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(TARGET) $(OBJS) lex.yy.c lex.yy.o parser.tab.c parser.tab.h parser.tab.o

.PHONY: all clean
