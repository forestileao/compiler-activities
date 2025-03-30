%{
#include <stdio.h>
#include <stdlib.h>

float stack[255];
int current = 0;

float top() {
    if (current == 0) {
        printf("Empty Stack!\n");
        return 0;
    }
    return stack[current - 1];
}

void push(float elem) {
    stack[current] = elem;
    current++;
}

float pop() {
    if (current == 0) {
        printf("Empty stack!\n");
        return 0;
    }
    float elem = top();
    current--;
    return elem;
}

float calculate(char op, float elem1, float elem2) {
    switch (op) {
        case '+':
            return elem1 + elem2;
        case '-':
            return elem1 - elem2;
        case '*':
            return elem1 * elem2;
        case '/':
            return elem1 / elem2;
        default:
            return 0;
    }
    return 0;
}

%}

DIGITO  [0-9]
NUMERO  -?{DIGITO}+(\.{DIGITO}+)?
OP      [\+\-\*\/]
PRINT   \=
ENTRADA \<\>

%%

{NUMERO}        { push(atof(yytext)); }
{OP}            { float elem1 = pop(); float elem2 = pop(); push(calculate(yytext[0], elem1, elem2)); }
{PRINT}         { printf("Resultado: %f", pop()); }
{ENTRADA}       { /* nada */ }
.+              { printf("Comando nao reconhecido!\n"); }

%%

int main() {
    yylex();
    return 0;
}

int yywrap() {
    return 1;
}