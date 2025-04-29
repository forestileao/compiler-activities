%error-verbose /* instruct bison to generate verbose error messages*/
%{
/* enable debugging of the parser: when yydebug is set to 1 before the
 * yyparse call the parser prints a lot of messages about what it does */
#define YYDEBUG 1
%}



%union {
    int val;
    char op;
    char* name;
}

%token TOKEN_BEGIN TOKEN_END TOKEN_WHILE TOKEN_DO TOKEN_ID TOKEN_NUMBER TOKEN_OPERATOR
%start program

%{
/* Forward declarations */
void yyerror(const char* const message);
int yylex(void);

%}

%%

program: statement';';

block: TOKEN_BEGIN statements TOKEN_END;

statements:
    | statements statement ';'
    | statements block';';

statement:
      assignment
    | whileStmt
    | block
    | call;

assignment: TOKEN_ID '=' expression;

expression: TOKEN_ID
    | TOKEN_NUMBER
    | expression TOKEN_OPERATOR expression;

whileStmt: TOKEN_WHILE expression TOKEN_DO statement;

call: TOKEN_ID '(' expression ')';

%%

#include <stdlib.h>
#include <assert.h>

void yyerror(const char* const message)
{
    fprintf(stderr, "Parse error:%s\n", message);
    exit(1);
}

int main()
{
    yydebug = 0;
    struct AstElement *a = 0;
    yyparse(&a);
    /* Q&D WARNING: in production code this assert must be replaced by
     * real error handling. */
    assert(a);
    struct ExecEnviron* e = createEnv();
    execAst(e, a);
    freeEnv(e);
    /* TODO: destroy the AST */
}
