%{
/* Text hider using flex states
   Replaces content between <HIDE> and </HIDE> tags with XXXXXXXXXXXX
   and outputs </HIDE> as <HIDE/> */
#include <stdio.h>
#include <string.h>
%}

%option noyywrap

%s HIDE_STATE

%%

<INITIAL>"<HIDE>"    { printf("%s", yytext); BEGIN(HIDE_STATE); }
<HIDE_STATE>"</HIDE>" { printf("<HIDE/>"); BEGIN(INITIAL); }
<HIDE_STATE>.|\n     { printf("X"); }
<INITIAL>.|\n        { printf("%s", yytext); }

%%

int main(int argc, char **argv) {
    yylex();
    return 0;
}
