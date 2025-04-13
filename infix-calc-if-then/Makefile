build:
	bison -d inf_calc.y -o inf_calc.tab.c
	flex -o inf_calc.lex.c inf_calc.l
	gcc -o inf_calc inf_calc.tab.c inf_calc.lex.c -lm
	rm *.c *.h

run: build
	./inf_calc