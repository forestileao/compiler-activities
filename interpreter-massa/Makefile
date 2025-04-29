all:
	flex -o lexer.c lexer.l
	bison -d -o parser.c parser.y
	gcc parser.c lexer.c astgen.c astgen.c -o toylang-noop
	./toylang-noop < example.toy


clean:
	rm -f lexer.c parser.h parser.c parser.tab.c parser.tab.h toylang-noop
	rm -f *.o
