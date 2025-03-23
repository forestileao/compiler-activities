%{
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* funcao para criptografar um caractere usando a cifra de Cesar */
char encrypt(char c) {
    if (isalpha(c)) {
        char base = islower(c) ? 'a' : 'A';
        return ((c - base + 3) % 26) + base;
    }
    return c;
}

/* funcao para descriptografar um caractere */
char decrypt(char c) {
    if (isalpha(c)) {
        char base = islower(c) ? 'a' : 'A';
        return ((c - base + 23) % 26) + base;
    }
    return c;
}

%}

%option noyywrap

%s CRYPTO_STATE
%s DECRYPTO_STATE

%%

<INITIAL>"<CRIPTO>"     { printf("%s", yytext); BEGIN(CRYPTO_STATE); }
<INITIAL>"<DECRIPTO>"     { printf("%s", yytext); BEGIN(DECRYPTO_STATE); }
<CRYPTO_STATE>"</CRIPTO>" { printf("<CRIPTO/>"); BEGIN(INITIAL); }
<DECRYPTO_STATE>"</DECRIPTO>" { printf("<DECRIPTO/>"); BEGIN(INITIAL); }
<CRYPTO_STATE>.         { putchar(encrypt(yytext[0])); }
<DECRYPTO_STATE>.         { putchar(decrypt(yytext[0])); }
<CRYPTO_STATE>\n        { putchar('\n'); }
<DECRYPTO_STATE>\n        { putchar('\n'); }
<INITIAL>.|\n           { printf("%s", yytext); }

%%

int main(int argc, char **argv) {
    yylex();
    return 0;
}
