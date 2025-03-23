%option case-insensitive
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int char_count = 0;
int word_count = 0;
int line_count = 0;
int command_count = 0;
%}

CHAR    .
NEWLINE \n
COMMENT1 \(\*([^*]|\*[^)])*\*\)
COMMENT2 \{[^}]*\}
STRING  \'[^']*\'
KEYWORDS (begin|end|if|then|else|while|do|for|to|downto|repeat|until|case|of|function|procedure|var|const|type|record|array|program|uses|unit|interface|implementation|inherited|override|private|public|protected|class|object|constructor|destructor)
WORD    [A-Za-z_][A-Za-z0-9_]*
SPACE   [ \t]+

%%
{COMMENT1}   ;
{COMMENT2}   ;
{STRING}     ;
{NEWLINE}    { line_count++; }
{CHAR}       { char_count++; }
{SPACE}      ;
{KEYWORDS}   { command_count++; }
{WORD}       { word_count++; }

%%

int main() {
    yylex();
    printf("Caracteres: %d\n", char_count);
    printf("Linhas: %d\n", line_count);
    printf("Palavras: %d\n", word_count);
    printf("Comandos Pascal: %d\n", command_count);
    return 0;
}

int yywrap() {
    return 1;
}
