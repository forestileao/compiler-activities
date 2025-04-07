build:
	bison -d calc.y -o calc.tab.c
	flex -o calc.lex.c calc.l
	gcc -o calc calc.tab.c calc.lex.c -lm
	rm *.c *.h

run: build
	./calc
