rpcalc: flex.o bison.o
	gcc flex.o bison.o -lfl -o rpcalc -lm

flex.o: flex.c bison.c
	gcc -c flex.c

bison.o: bison.c
	gcc -c bison.c

flex.c: calc.l
	flex -o flex.c calc.l

bison.c: calc.y
	bison -d -o bison.c calc.y

clean:
	rm *.o *.c *.h # nao sei se funciona no DOS
