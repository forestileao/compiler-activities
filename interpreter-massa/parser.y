%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern int yylex();
extern FILE *yyin;
extern char *yytext;
extern int yylineno;

int yyerror(char *msg);

/* Definição de tipos de dados */
#define TIPO_INT 1
#define TIPO_FLOAT 2
#define TIPO_BOOLEAN 3
#define TIPO_CHAR 4
#define MAX_VAR 100 /* Máximo de 100 variáveis */

/* Struct para a tabela de símbolos */
typedef struct {
    char nome[11];  /* Limitado a 10 caracteres + nulo */
    int tipo;       /* 1=int, 2=float, 3=boolean, 4=char */
    union {
        int ival;
        float fval;
        int bval;   /* 0=false, 1=true */
        char cval;
    } valor;
} simbolo;

simbolo tabela[MAX_VAR]; /* Tabela de símbolos */
int nvars = 0;           /* Contador de variáveis */

/* Função para buscar variável na tabela */
int busca_var(char *nome) {
    int i;
    for (i = 0; i < nvars; i++) {
        if (strcmp(tabela[i].nome, nome) == 0) {
            return i;
        }
    }
    return -1;  /* Não encontrou */
}

/* Função para adicionar variável à tabela */
int adiciona_var(char *nome, int tipo) {
    if (nvars >= MAX_VAR) {
        yyerror("Limite máximo de variáveis excedido");
        return -1;
    }
    
    if (busca_var(nome) != -1) {
        yyerror("Variável já declarada");
        return -1;
    }
    
    strcpy(tabela[nvars].nome, nome);
    tabela[nvars].tipo = tipo;
    
    /* Inicialização padrão */
    switch (tipo) {
        case TIPO_INT:
            tabela[nvars].valor.ival = 0;
            break;
        case TIPO_FLOAT:
            tabela[nvars].valor.fval = 0.0;
            break;
        case TIPO_BOOLEAN:
            tabela[nvars].valor.bval = 0;
            break;
        case TIPO_CHAR:
            tabela[nvars].valor.cval = '\0';
            break;
    }
    
    return nvars++;
}

/* Definição do tipo união para valor */
typedef union {
    int ival;
    float fval;
    int bval;
    char cval;
} valor_t;

/* Função para atribuir valor à variável */
void atribui_valor(int pos, int tipo_expr, valor_t valor) {
    if (pos < 0 || pos >= nvars) {
        yyerror("Variável inexistente");
        return;
    }
    
    /* Verificar compatibilidade de tipos */
    switch (tabela[pos].tipo) {
        case TIPO_INT:
            if (tipo_expr == TIPO_INT) {
                tabela[pos].valor.ival = valor.ival;
            } else if (tipo_expr == TIPO_FLOAT) {
                tabela[pos].valor.ival = (int)valor.fval;
            } else if (tipo_expr == TIPO_CHAR) {
                tabela[pos].valor.ival = (int)valor.cval;
            } else {
                yyerror("Tipo incompatível em atribuição para variável int");
            }
            break;
        case TIPO_FLOAT:
            if (tipo_expr == TIPO_INT) {
                tabela[pos].valor.fval = (float)valor.ival;
            } else if (tipo_expr == TIPO_FLOAT) {
                tabela[pos].valor.fval = valor.fval;
            } else if (tipo_expr == TIPO_CHAR) {
                tabela[pos].valor.fval = (float)valor.cval;
            } else {
                yyerror("Tipo incompatível em atribuição para variável float");
            }
            break;
        case TIPO_BOOLEAN:
            if (tipo_expr == TIPO_BOOLEAN) {
                tabela[pos].valor.bval = valor.bval;
            } else {
                yyerror("Tipo incompatível em atribuição para variável boolean");
            }
            break;
        case TIPO_CHAR:
            if (tipo_expr == TIPO_CHAR) {
                tabela[pos].valor.cval = valor.cval;
            } else if (tipo_expr == TIPO_INT && valor.ival >= 0 && valor.ival <= 255) {
                tabela[pos].valor.cval = (char)valor.ival;
            } else {
                yyerror("Tipo incompatível em atribuição para variável char");
            }
            break;
    }
}

%}

%union {
    int num;
    float fnum;
    int bval;
    char cval;
    char str[256];
    
    struct {
        int tipo;
        union {
            int ival;
            float fval;
            int bval;
            char cval;
        } valor;
    } expr;
}

/* Definição dos tokens */
%token INT FLOAT BOOLEAN CHAR
%token IF THEN ELSE READ WRITE
%token OR AND NOT
%token <str> ID
%token <num> NUM_INT
%token <fnum> NUM_FLOAT
%token <bval> BOOL_VAL
%token <cval> CHAR_VAL
%token <str> STRING
%token PLUS MINUS MULT DIV
%token ASSIGN LT GT LE GE NE EQ
%token LP RP LB RB SEMICOLON COMMA

/* Tipos para as regras */
%type <expr> expr_aritmetica termo fator expr_logica op_cmp

%%

programa: declaracoes comandos
        ;

declaracoes: declaracao SEMICOLON
          | declaracoes declaracao SEMICOLON
          ;

declaracao: INT ID { adiciona_var($2, TIPO_INT); }
         | FLOAT ID { adiciona_var($2, TIPO_FLOAT); }
         | BOOLEAN ID { adiciona_var($2, TIPO_BOOLEAN); }
         | CHAR ID { adiciona_var($2, TIPO_CHAR); }
         ;

comandos: comando
        | comandos comando
        ;

comando: comando_if
       | atribs
       | comando_read SEMICOLON
       | comando_write SEMICOLON
       ;

comando_read: READ LP ID RP {
                int pos = busca_var($3);
                if (pos == -1) {
                    yyerror("Variável não declarada");
                } else {
                    printf("Informe valor para %s: ", $3);
                    switch (tabela[pos].tipo) {
                        case TIPO_INT:
                            scanf("%d", &tabela[pos].valor.ival);
                            break;
                        case TIPO_FLOAT:
                            scanf("%f", &tabela[pos].valor.fval);
                            break;
                        case TIPO_BOOLEAN: {
                            int temp;
                            scanf("%d", &temp);
                            tabela[pos].valor.bval = temp != 0;
                            break;
                        }
                        case TIPO_CHAR:
                            scanf(" %c", &tabela[pos].valor.cval);
                            break;
                    }
                }
            }
          ;

comando_write: WRITE LP ID RP {
                int pos = busca_var($3);
                if (pos == -1) {
                    yyerror("Variável não declarada");
                } else {
                    switch (tabela[pos].tipo) {
                        case TIPO_INT:
                            printf("%d\n", tabela[pos].valor.ival);
                            break;
                        case TIPO_FLOAT:
                            printf("%f\n", tabela[pos].valor.fval);
                            break;
                        case TIPO_BOOLEAN:
                            printf("%s\n", tabela[pos].valor.bval ? "true" : "false");
                            break;
                        case TIPO_CHAR:
                            printf("%c\n", tabela[pos].valor.cval);
                            break;
                    }
                }
             }
           | WRITE LP NUM_INT RP { printf("%d\n", $3); }
           | WRITE LP NUM_FLOAT RP { printf("%f\n", $3); }
           | WRITE LP STRING RP { 
                /* Remover aspas do início e fim */
                int len = strlen($3);
                char str[256];
                strncpy(str, $3 + 1, len - 2);
                str[len - 2] = '\0';
                printf("%s\n", str); 
             }
           | WRITE LP CHAR_VAL RP { printf("%c\n", $3); }
           ;

atribs: atrib SEMICOLON
      | atribs atrib SEMICOLON
      ;

comando_if: IF LP expr_logica RP THEN LB atribs RB {
                if ($3.tipo != TIPO_BOOLEAN) {
                    yyerror("Expressão do IF deve ser booleana");
                } else {
                    if ($3.valor.bval) {
                        /* Código do THEN já executado na redução gramatical */
                    }
                }
            }
          | IF LP expr_logica RP THEN LB atribs RB ELSE LB atribs RB {
                if ($3.tipo != TIPO_BOOLEAN) {
                    yyerror("Expressão do IF deve ser booleana");
                } else {
                    /* Código do THEN ou ELSE já executado na redução gramatical */
                }
            }
          ;

expr_logica: expr_aritmetica op_cmp expr_aritmetica {
                $$.tipo = TIPO_BOOLEAN;
                
                if ($1.tipo == TIPO_BOOLEAN || $3.tipo == TIPO_BOOLEAN) {
                    yyerror("Operador de comparação não pode ser usado com boolean");
                    $$.valor.bval = 0;
                } else {
                    float val1, val2;
                    
                    /* Converter para float para comparação */
                    if ($1.tipo == TIPO_INT) val1 = (float)$1.valor.ival;
                    else if ($1.tipo == TIPO_FLOAT) val1 = $1.valor.fval;
                    else if ($1.tipo == TIPO_CHAR) val1 = (float)$1.valor.cval;
                    
                    if ($3.tipo == TIPO_INT) val2 = (float)$3.valor.ival;
                    else if ($3.tipo == TIPO_FLOAT) val2 = $3.valor.fval;
                    else if ($3.tipo == TIPO_CHAR) val2 = (float)$3.valor.cval;
                    
                    switch ($2.valor.ival) {
                        case 1: $$.valor.bval = (val1 < val2); break;  /* LT */
                        case 2: $$.valor.bval = (val1 > val2); break;  /* GT */
                        case 3: $$.valor.bval = (val1 <= val2); break; /* LE */
                        case 4: $$.valor.bval = (val1 >= val2); break; /* GE */
                        case 5: $$.valor.bval = (val1 != val2); break; /* NE */
                        case 6: $$.valor.bval = (val1 == val2); break; /* EQ */
                    }
                }
            }
          | LP expr_logica OR expr_logica RP {
                if ($2.tipo != TIPO_BOOLEAN || $4.tipo != TIPO_BOOLEAN) {
                    yyerror("Operador OR requer operandos booleanos");
                    $$.tipo = TIPO_BOOLEAN;
                    $$.valor.bval = 0;
                } else {
                    $$.tipo = TIPO_BOOLEAN;
                    $$.valor.bval = ($2.valor.bval || $4.valor.bval);
                }
            }
          | LP expr_logica AND expr_logica RP {
                if ($2.tipo != TIPO_BOOLEAN || $4.tipo != TIPO_BOOLEAN) {
                    yyerror("Operador AND requer operandos booleanos");
                    $$.tipo = TIPO_BOOLEAN;
                    $$.valor.bval = 0;
                } else {
                    $$.tipo = TIPO_BOOLEAN;
                    $$.valor.bval = ($2.valor.bval && $4.valor.bval);
                }
            }
          | LP NOT expr_logica RP {
                if ($3.tipo != TIPO_BOOLEAN) {
                    yyerror("Operador NOT requer operando booleano");
                    $$.tipo = TIPO_BOOLEAN;
                    $$.valor.bval = 0;
                } else {
                    $$.tipo = TIPO_BOOLEAN;
                    $$.valor.bval = !$3.valor.bval;
                }
            }
          | ID {
                int pos = busca_var($1);
                if (pos == -1) {
                    yyerror("Variável não declarada");
                    $$.tipo = TIPO_BOOLEAN;
                    $$.valor.bval = 0;
                } else if (tabela[pos].tipo != TIPO_BOOLEAN) {
                    yyerror("Variável deve ser booleana");
                    $$.tipo = TIPO_BOOLEAN;
                    $$.valor.bval = 0;
                } else {
                    $$.tipo = TIPO_BOOLEAN;
                    $$.valor.bval = tabela[pos].valor.bval;
                }
            }
          | BOOL_VAL {
                $$.tipo = TIPO_BOOLEAN;
                $$.valor.bval = $1;
            }
          ;

op_cmp: LT { $$.tipo = TIPO_INT; $$.valor.ival = 1; }
      | GT { $$.tipo = TIPO_INT; $$.valor.ival = 2; }
      | LE { $$.tipo = TIPO_INT; $$.valor.ival = 3; }
      | GE { $$.tipo = TIPO_INT; $$.valor.ival = 4; }
      | NE { $$.tipo = TIPO_INT; $$.valor.ival = 5; }
      | EQ { $$.tipo = TIPO_INT; $$.valor.ival = 6; }
      ;

atrib: ID ASSIGN expr_aritmetica {
            int pos = busca_var($1);
            if (pos == -1) {
                yyerror("Variável não declarada");
            } else {
                /* Verificar compatibilidade de tipos */
                valor_t valor;
                
                switch ($3.tipo) {
                    case TIPO_INT:
                        valor.ival = $3.valor.ival;
                        break;
                    case TIPO_FLOAT:
                        valor.fval = $3.valor.fval;
                        break;
                    case TIPO_BOOLEAN:
                        valor.bval = $3.valor.bval;
                        break;
                    case TIPO_CHAR:
                        valor.cval = $3.valor.cval;
                        break;
                }
                
                atribui_valor(pos, $3.tipo, valor);
            }
        }
      ;

expr_aritmetica: termo
               | expr_aritmetica PLUS termo {
                    if ($1.tipo == TIPO_BOOLEAN || $3.tipo == TIPO_BOOLEAN) {
                        yyerror("Operador + não pode ser usado com boolean");
                        $$.tipo = TIPO_INT;
                        $$.valor.ival = 0;
                    } else if ($1.tipo == TIPO_FLOAT || $3.tipo == TIPO_FLOAT) {
                        $$.tipo = TIPO_FLOAT;
                        float v1 = ($1.tipo == TIPO_INT) ? (float)$1.valor.ival : 
                               ($1.tipo == TIPO_CHAR) ? (float)$1.valor.cval : $1.valor.fval;
                        float v2 = ($3.tipo == TIPO_INT) ? (float)$3.valor.ival : 
                               ($3.tipo == TIPO_CHAR) ? (float)$3.valor.cval : $3.valor.fval;
                        $$.valor.fval = v1 + v2;
                    } else if ($1.tipo == TIPO_CHAR && $3.tipo == TIPO_CHAR) {
                        yyerror("Operador + não pode ser usado com char");
                        $$.tipo = TIPO_INT;
                        $$.valor.ival = 0;
                    } else {
                        $$.tipo = TIPO_INT;
                        int v1 = ($1.tipo == TIPO_INT) ? $1.valor.ival : (int)$1.valor.cval;
                        int v2 = ($3.tipo == TIPO_INT) ? $3.valor.ival : (int)$3.valor.cval;
                        $$.valor.ival = v1 + v2;
                    }
                }
               | expr_aritmetica MINUS termo {
                    if ($1.tipo == TIPO_BOOLEAN || $3.tipo == TIPO_BOOLEAN) {
                        yyerror("Operador - não pode ser usado com boolean");
                        $$.tipo = TIPO_INT;
                        $$.valor.ival = 0;
                    } else if ($1.tipo == TIPO_FLOAT || $3.tipo == TIPO_FLOAT) {
                        $$.tipo = TIPO_FLOAT;
                        float v1 = ($1.tipo == TIPO_INT) ? (float)$1.valor.ival : 
                               ($1.tipo == TIPO_CHAR) ? (float)$1.valor.cval : $1.valor.fval;
                        float v2 = ($3.tipo == TIPO_INT) ? (float)$3.valor.ival : 
                               ($3.tipo == TIPO_CHAR) ? (float)$3.valor.cval : $3.valor.fval;
                        $$.valor.fval = v1 - v2;
                    } else if ($1.tipo == TIPO_CHAR && $3.tipo == TIPO_CHAR) {
                        yyerror("Operador - não pode ser usado com char");
                        $$.tipo = TIPO_INT;
                        $$.valor.ival = 0;
                    } else {
                        $$.tipo = TIPO_INT;
                        int v1 = ($1.tipo == TIPO_INT) ? $1.valor.ival : (int)$1.valor.cval;
                        int v2 = ($3.tipo == TIPO_INT) ? $3.valor.ival : (int)$3.valor.cval;
                        $$.valor.ival = v1 - v2;
                    }
                }
               ;

termo: fator
     | termo MULT fator {
            if ($1.tipo == TIPO_BOOLEAN || $3.tipo == TIPO_BOOLEAN) {
                yyerror("Operador * não pode ser usado com boolean");
                $$.tipo = TIPO_INT;
                $$.valor.ival = 0;
            } else if ($1.tipo == TIPO_FLOAT || $3.tipo == TIPO_FLOAT) {
                $$.tipo = TIPO_FLOAT;
                float v1 = ($1.tipo == TIPO_INT) ? (float)$1.valor.ival : 
                       ($1.tipo == TIPO_CHAR) ? (float)$1.valor.cval : $1.valor.fval;
                float v2 = ($3.tipo == TIPO_INT) ? (float)$3.valor.ival : 
                       ($3.tipo == TIPO_CHAR) ? (float)$3.valor.cval : $3.valor.fval;
                $$.valor.fval = v1 * v2;
            } else if ($1.tipo == TIPO_CHAR || $3.tipo == TIPO_CHAR) {
                yyerror("Operador * não pode ser usado com char");
                $$.tipo = TIPO_INT;
                $$.valor.ival = 0;
            } else {
                $$.tipo = TIPO_INT;
                $$.valor.ival = $1.valor.ival * $3.valor.ival;
            }
        }
     | termo DIV fator {
            if ($1.tipo == TIPO_BOOLEAN || $3.tipo == TIPO_BOOLEAN) {
                yyerror("Operador / não pode ser usado com boolean");
                $$.tipo = TIPO_INT;
                $$.valor.ival = 0;
            } else if ($3.tipo == TIPO_INT && $3.valor.ival == 0) {
                yyerror("Divisão por zero");
                $$.tipo = TIPO_INT;
                $$.valor.ival = 0;
            } else if ($3.tipo == TIPO_FLOAT && $3.valor.fval == 0.0) {
                yyerror("Divisão por zero");
                $$.tipo = TIPO_FLOAT;
                $$.valor.fval = 0.0;
            } else if ($1.tipo == TIPO_FLOAT || $3.tipo == TIPO_FLOAT) {
                $$.tipo = TIPO_FLOAT;
                float v1 = ($1.tipo == TIPO_INT) ? (float)$1.valor.ival : 
                       ($1.tipo == TIPO_CHAR) ? (float)$1.valor.cval : $1.valor.fval;
                float v2 = ($3.tipo == TIPO_INT) ? (float)$3.valor.ival : 
                       ($3.tipo == TIPO_CHAR) ? (float)$3.valor.cval : $3.valor.fval;
                $$.valor.fval = v1 / v2;
            } else if ($1.tipo == TIPO_CHAR || $3.tipo == TIPO_CHAR) {
                yyerror("Operador / não pode ser usado com char");
                $$.tipo = TIPO_INT;
                $$.valor.ival = 0;
            } else {
                $$.tipo = TIPO_INT;
                $$.valor.ival = $1.valor.ival / $3.valor.ival;
            }
        }
     ;

fator: ID {
            int pos = busca_var($1);
            if (pos == -1) {
                yyerror("Variável não declarada");
                $$.tipo = TIPO_INT;
                $$.valor.ival = 0;
            } else {
                $$.tipo = tabela[pos].tipo;
                switch (tabela[pos].tipo) {
                    case TIPO_INT:
                        $$.valor.ival = tabela[pos].valor.ival;
                        break;
                    case TIPO_FLOAT:
                        $$.valor.fval = tabela[pos].valor.fval;
                        break;
                    case TIPO_BOOLEAN:
                        $$.valor.bval = tabela[pos].valor.bval;
                        break;
                    case TIPO_CHAR:
                        $$.valor.cval = tabela[pos].valor.cval;
                        break;
                }
            }
        }
     | NUM_INT {
            $$.tipo = TIPO_INT;
            $$.valor.ival = $1;
        }
     | NUM_FLOAT {
            $$.tipo = TIPO_FLOAT;
            $$.valor.fval = $1;
        }
     | CHAR_VAL {
            $$.tipo = TIPO_CHAR;
            $$.valor.cval = $1;
        }
     | LP expr_aritmetica RP {
            $$ = $2;
        }
     ;

%%

int yyerror(char *msg) {
    fprintf(stderr, "Erro: %s (linha %d): %s\n", msg, yylineno, yytext);
    return 0;
}

int main(int argc, char **argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            printf("Não foi possível abrir o arquivo %s\n", argv[1]);
            return 1;
        }
    } else {
        yyin = stdin;
    }
    
    yyparse();
    
    if (yyin != stdin) {
        fclose(yyin);
    }
    
    return 0;
}
