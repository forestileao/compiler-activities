/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "parser.y"

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


#line 211 "y.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    INT = 258,                     /* INT  */
    FLOAT = 259,                   /* FLOAT  */
    BOOLEAN = 260,                 /* BOOLEAN  */
    CHAR = 261,                    /* CHAR  */
    IF = 262,                      /* IF  */
    THEN = 263,                    /* THEN  */
    ELSE = 264,                    /* ELSE  */
    READ = 265,                    /* READ  */
    WRITE = 266,                   /* WRITE  */
    OR = 267,                      /* OR  */
    AND = 268,                     /* AND  */
    NOT = 269,                     /* NOT  */
    ID = 270,                      /* ID  */
    NUM_INT = 271,                 /* NUM_INT  */
    NUM_FLOAT = 272,               /* NUM_FLOAT  */
    BOOL_VAL = 273,                /* BOOL_VAL  */
    CHAR_VAL = 274,                /* CHAR_VAL  */
    STRING = 275,                  /* STRING  */
    PLUS = 276,                    /* PLUS  */
    MINUS = 277,                   /* MINUS  */
    MULT = 278,                    /* MULT  */
    DIV = 279,                     /* DIV  */
    ASSIGN = 280,                  /* ASSIGN  */
    LT = 281,                      /* LT  */
    GT = 282,                      /* GT  */
    LE = 283,                      /* LE  */
    GE = 284,                      /* GE  */
    NE = 285,                      /* NE  */
    EQ = 286,                      /* EQ  */
    LP = 287,                      /* LP  */
    RP = 288,                      /* RP  */
    LB = 289,                      /* LB  */
    RB = 290,                      /* RB  */
    SEMICOLON = 291,               /* SEMICOLON  */
    COMMA = 292                    /* COMMA  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define INT 258
#define FLOAT 259
#define BOOLEAN 260
#define CHAR 261
#define IF 262
#define THEN 263
#define ELSE 264
#define READ 265
#define WRITE 266
#define OR 267
#define AND 268
#define NOT 269
#define ID 270
#define NUM_INT 271
#define NUM_FLOAT 272
#define BOOL_VAL 273
#define CHAR_VAL 274
#define STRING 275
#define PLUS 276
#define MINUS 277
#define MULT 278
#define DIV 279
#define ASSIGN 280
#define LT 281
#define GT 282
#define LE 283
#define GE 284
#define NE 285
#define EQ 286
#define LP 287
#define RP 288
#define LB 289
#define RB 290
#define SEMICOLON 291
#define COMMA 292

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 141 "parser.y"

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

#line 356 "y.tab.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_INT = 3,                        /* INT  */
  YYSYMBOL_FLOAT = 4,                      /* FLOAT  */
  YYSYMBOL_BOOLEAN = 5,                    /* BOOLEAN  */
  YYSYMBOL_CHAR = 6,                       /* CHAR  */
  YYSYMBOL_IF = 7,                         /* IF  */
  YYSYMBOL_THEN = 8,                       /* THEN  */
  YYSYMBOL_ELSE = 9,                       /* ELSE  */
  YYSYMBOL_READ = 10,                      /* READ  */
  YYSYMBOL_WRITE = 11,                     /* WRITE  */
  YYSYMBOL_OR = 12,                        /* OR  */
  YYSYMBOL_AND = 13,                       /* AND  */
  YYSYMBOL_NOT = 14,                       /* NOT  */
  YYSYMBOL_ID = 15,                        /* ID  */
  YYSYMBOL_NUM_INT = 16,                   /* NUM_INT  */
  YYSYMBOL_NUM_FLOAT = 17,                 /* NUM_FLOAT  */
  YYSYMBOL_BOOL_VAL = 18,                  /* BOOL_VAL  */
  YYSYMBOL_CHAR_VAL = 19,                  /* CHAR_VAL  */
  YYSYMBOL_STRING = 20,                    /* STRING  */
  YYSYMBOL_PLUS = 21,                      /* PLUS  */
  YYSYMBOL_MINUS = 22,                     /* MINUS  */
  YYSYMBOL_MULT = 23,                      /* MULT  */
  YYSYMBOL_DIV = 24,                       /* DIV  */
  YYSYMBOL_ASSIGN = 25,                    /* ASSIGN  */
  YYSYMBOL_LT = 26,                        /* LT  */
  YYSYMBOL_GT = 27,                        /* GT  */
  YYSYMBOL_LE = 28,                        /* LE  */
  YYSYMBOL_GE = 29,                        /* GE  */
  YYSYMBOL_NE = 30,                        /* NE  */
  YYSYMBOL_EQ = 31,                        /* EQ  */
  YYSYMBOL_LP = 32,                        /* LP  */
  YYSYMBOL_RP = 33,                        /* RP  */
  YYSYMBOL_LB = 34,                        /* LB  */
  YYSYMBOL_RB = 35,                        /* RB  */
  YYSYMBOL_SEMICOLON = 36,                 /* SEMICOLON  */
  YYSYMBOL_COMMA = 37,                     /* COMMA  */
  YYSYMBOL_YYACCEPT = 38,                  /* $accept  */
  YYSYMBOL_programa = 39,                  /* programa  */
  YYSYMBOL_declaracoes = 40,               /* declaracoes  */
  YYSYMBOL_declaracao = 41,                /* declaracao  */
  YYSYMBOL_comandos = 42,                  /* comandos  */
  YYSYMBOL_comando = 43,                   /* comando  */
  YYSYMBOL_comando_read = 44,              /* comando_read  */
  YYSYMBOL_comando_write = 45,             /* comando_write  */
  YYSYMBOL_atribs = 46,                    /* atribs  */
  YYSYMBOL_comando_if = 47,                /* comando_if  */
  YYSYMBOL_expr_logica = 48,               /* expr_logica  */
  YYSYMBOL_op_cmp = 49,                    /* op_cmp  */
  YYSYMBOL_atrib = 50,                     /* atrib  */
  YYSYMBOL_expr_aritmetica = 51,           /* expr_aritmetica  */
  YYSYMBOL_termo = 52,                     /* termo  */
  YYSYMBOL_fator = 53                      /* fator  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  12
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   122

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  38
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  16
/* YYNRULES -- Number of rules.  */
#define YYNRULES  48
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  100

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   292


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   178,   178,   181,   182,   185,   186,   187,   188,   191,
     192,   195,   196,   197,   198,   201,   228,   249,   250,   251,
     259,   262,   263,   266,   275,   284,   312,   322,   332,   342,
     357,   363,   364,   365,   366,   367,   368,   371,   399,   400,
     423,   448,   449,   470,   501,   525,   529,   533,   537
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "INT", "FLOAT",
  "BOOLEAN", "CHAR", "IF", "THEN", "ELSE", "READ", "WRITE", "OR", "AND",
  "NOT", "ID", "NUM_INT", "NUM_FLOAT", "BOOL_VAL", "CHAR_VAL", "STRING",
  "PLUS", "MINUS", "MULT", "DIV", "ASSIGN", "LT", "GT", "LE", "GE", "NE",
  "EQ", "LP", "RP", "LB", "RB", "SEMICOLON", "COMMA", "$accept",
  "programa", "declaracoes", "declaracao", "comandos", "comando",
  "comando_read", "comando_write", "atribs", "comando_if", "expr_logica",
  "op_cmp", "atrib", "expr_aritmetica", "termo", "fator", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-86)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-30)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
       5,    -2,     0,     8,    23,    47,    74,    18,   -86,   -86,
     -86,   -86,   -86,    26,    36,    43,    41,    52,    42,   -86,
      66,    67,    75,   -86,    68,   -86,    13,    86,    54,    27,
     -86,   -86,   -86,   -86,    69,   -86,    -9,   -86,   -86,   -86,
     -86,     3,    73,    65,    10,   -86,    76,    77,    78,    79,
      80,    81,   -86,    27,    29,   -86,    13,    70,    34,    99,
      27,    27,   -86,   -86,   -86,   -86,   -86,   -86,    27,    27,
      27,   -86,   -86,   -86,   -86,   -86,   -86,    15,    82,    13,
      13,   -86,    83,    10,    10,    29,   -86,   -86,   -86,    85,
      87,    75,   -86,   -86,   -10,   107,    88,    75,    -8,   -86
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     5,     6,
       7,     8,     1,     0,     0,     0,     0,     0,     2,     9,
       0,     0,    12,    11,     0,     3,     0,     0,     0,     0,
       4,    10,    13,    14,     0,    21,    44,    45,    46,    30,
      47,     0,     0,     0,    38,    41,     0,     0,     0,     0,
       0,     0,    44,     0,    37,    22,     0,     0,     0,     0,
       0,     0,    31,    32,    33,    34,    35,    36,     0,     0,
       0,    15,    16,    17,    18,    20,    19,     0,     0,     0,
       0,    48,     0,    39,    40,    25,    42,    43,    28,     0,
       0,     0,    26,    27,     0,    23,     0,     0,     0,    24
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -86,   -86,   -86,   102,   -86,   101,   -86,   -86,   -85,   -86,
     -40,   -86,   -22,   -27,    37,    30
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     5,     6,     7,    18,    19,    20,    21,    22,    23,
      42,    68,    24,    43,    44,    45
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      34,    57,    54,   -29,   -29,    16,    94,    16,     1,     2,
       3,     4,    98,     8,    58,     9,    78,    56,    36,    37,
      38,    39,    40,    10,   -29,    95,    77,    99,    36,    37,
      38,    39,    40,    69,    70,    41,    60,    61,    11,    89,
      90,    85,    52,    37,    38,    41,    40,    12,    81,    13,
      60,    61,    14,    15,    25,    60,    61,    16,    26,    53,
      62,    63,    64,    65,    66,    67,    29,    81,    27,    47,
      48,    49,    34,    50,    51,    28,    34,     1,     2,     3,
       4,    13,    79,    80,    14,    15,    60,    61,    30,    16,
      16,    62,    63,    64,    65,    66,    67,    83,    84,    86,
      87,    46,    32,    33,    35,    55,    59,    82,    17,    71,
      72,    73,    74,    75,    76,    88,    96,    91,    92,    31,
      93,     0,    97
};

static const yytype_int8 yycheck[] =
{
      22,    41,    29,    12,    13,    15,    91,    15,     3,     4,
       5,     6,    97,    15,    41,    15,    56,    14,    15,    16,
      17,    18,    19,    15,    33,    35,    53,    35,    15,    16,
      17,    18,    19,    23,    24,    32,    21,    22,    15,    79,
      80,    68,    15,    16,    17,    32,    19,     0,    33,     7,
      21,    22,    10,    11,    36,    21,    22,    15,    32,    32,
      26,    27,    28,    29,    30,    31,    25,    33,    32,    15,
      16,    17,    94,    19,    20,    32,    98,     3,     4,     5,
       6,     7,    12,    13,    10,    11,    21,    22,    36,    15,
      15,    26,    27,    28,    29,    30,    31,    60,    61,    69,
      70,    15,    36,    36,    36,    36,    33,     8,     6,    33,
      33,    33,    33,    33,    33,    33,     9,    34,    33,    18,
      33,    -1,    34
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,     6,    39,    40,    41,    15,    15,
      15,    15,     0,     7,    10,    11,    15,    41,    42,    43,
      44,    45,    46,    47,    50,    36,    32,    32,    32,    25,
      36,    43,    36,    36,    50,    36,    15,    16,    17,    18,
      19,    32,    48,    51,    52,    53,    15,    15,    16,    17,
      19,    20,    15,    32,    51,    36,    14,    48,    51,    33,
      21,    22,    26,    27,    28,    29,    30,    31,    49,    23,
      24,    33,    33,    33,    33,    33,    33,    51,    48,    12,
      13,    33,     8,    52,    52,    51,    53,    53,    33,    48,
      48,    34,    33,    33,    46,    35,     9,    34,    46,    35
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    38,    39,    40,    40,    41,    41,    41,    41,    42,
      42,    43,    43,    43,    43,    44,    45,    45,    45,    45,
      45,    46,    46,    47,    47,    48,    48,    48,    48,    48,
      48,    49,    49,    49,    49,    49,    49,    50,    51,    51,
      51,    52,    52,    52,    53,    53,    53,    53,    53
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     2,     3,     2,     2,     2,     2,     1,
       2,     1,     1,     2,     2,     4,     4,     4,     4,     4,
       4,     2,     3,     8,    12,     3,     5,     5,     4,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     3,
       3,     1,     3,     3,     1,     1,     1,     1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 5: /* declaracao: INT ID  */
#line 185 "parser.y"
                   { adiciona_var((yyvsp[0].str), TIPO_INT); }
#line 1449 "y.tab.c"
    break;

  case 6: /* declaracao: FLOAT ID  */
#line 186 "parser.y"
                    { adiciona_var((yyvsp[0].str), TIPO_FLOAT); }
#line 1455 "y.tab.c"
    break;

  case 7: /* declaracao: BOOLEAN ID  */
#line 187 "parser.y"
                      { adiciona_var((yyvsp[0].str), TIPO_BOOLEAN); }
#line 1461 "y.tab.c"
    break;

  case 8: /* declaracao: CHAR ID  */
#line 188 "parser.y"
                   { adiciona_var((yyvsp[0].str), TIPO_CHAR); }
#line 1467 "y.tab.c"
    break;

  case 15: /* comando_read: READ LP ID RP  */
#line 201 "parser.y"
                            {
                int pos = busca_var((yyvsp[-1].str));
                if (pos == -1) {
                    yyerror("Variável não declarada");
                } else {
                    printf("Informe valor para %s: ", (yyvsp[-1].str));
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
#line 1497 "y.tab.c"
    break;

  case 16: /* comando_write: WRITE LP ID RP  */
#line 228 "parser.y"
                              {
                int pos = busca_var((yyvsp[-1].str));
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
#line 1523 "y.tab.c"
    break;

  case 17: /* comando_write: WRITE LP NUM_INT RP  */
#line 249 "parser.y"
                                 { printf("%d\n", (yyvsp[-1].num)); }
#line 1529 "y.tab.c"
    break;

  case 18: /* comando_write: WRITE LP NUM_FLOAT RP  */
#line 250 "parser.y"
                                   { printf("%f\n", (yyvsp[-1].fnum)); }
#line 1535 "y.tab.c"
    break;

  case 19: /* comando_write: WRITE LP STRING RP  */
#line 251 "parser.y"
                                { 
                /* Remover aspas do início e fim */
                int len = strlen((yyvsp[-1].str));
                char str[256];
                strncpy(str, (yyvsp[-1].str) + 1, len - 2);
                str[len - 2] = '\0';
                printf("%s\n", str); 
             }
#line 1548 "y.tab.c"
    break;

  case 20: /* comando_write: WRITE LP CHAR_VAL RP  */
#line 259 "parser.y"
                                  { printf("%c\n", (yyvsp[-1].cval)); }
#line 1554 "y.tab.c"
    break;

  case 23: /* comando_if: IF LP expr_logica RP THEN LB atribs RB  */
#line 266 "parser.y"
                                                   {
                if ((yyvsp[-5].expr).tipo != TIPO_BOOLEAN) {
                    yyerror("Expressão do IF deve ser booleana");
                } else {
                    if ((yyvsp[-5].expr).valor.bval) {
                        /* Código do THEN já executado na redução gramatical */
                    }
                }
            }
#line 1568 "y.tab.c"
    break;

  case 24: /* comando_if: IF LP expr_logica RP THEN LB atribs RB ELSE LB atribs RB  */
#line 275 "parser.y"
                                                                     {
                if ((yyvsp[-9].expr).tipo != TIPO_BOOLEAN) {
                    yyerror("Expressão do IF deve ser booleana");
                } else {
                    /* Código do THEN ou ELSE já executado na redução gramatical */
                }
            }
#line 1580 "y.tab.c"
    break;

  case 25: /* expr_logica: expr_aritmetica op_cmp expr_aritmetica  */
#line 284 "parser.y"
                                                    {
                (yyval.expr).tipo = TIPO_BOOLEAN;
                
                if ((yyvsp[-2].expr).tipo == TIPO_BOOLEAN || (yyvsp[0].expr).tipo == TIPO_BOOLEAN) {
                    yyerror("Operador de comparação não pode ser usado com boolean");
                    (yyval.expr).valor.bval = 0;
                } else {
                    float val1, val2;
                    
                    /* Converter para float para comparação */
                    if ((yyvsp[-2].expr).tipo == TIPO_INT) val1 = (float)(yyvsp[-2].expr).valor.ival;
                    else if ((yyvsp[-2].expr).tipo == TIPO_FLOAT) val1 = (yyvsp[-2].expr).valor.fval;
                    else if ((yyvsp[-2].expr).tipo == TIPO_CHAR) val1 = (float)(yyvsp[-2].expr).valor.cval;
                    
                    if ((yyvsp[0].expr).tipo == TIPO_INT) val2 = (float)(yyvsp[0].expr).valor.ival;
                    else if ((yyvsp[0].expr).tipo == TIPO_FLOAT) val2 = (yyvsp[0].expr).valor.fval;
                    else if ((yyvsp[0].expr).tipo == TIPO_CHAR) val2 = (float)(yyvsp[0].expr).valor.cval;
                    
                    switch ((yyvsp[-1].expr).valor.ival) {
                        case 1: (yyval.expr).valor.bval = (val1 < val2); break;  /* LT */
                        case 2: (yyval.expr).valor.bval = (val1 > val2); break;  /* GT */
                        case 3: (yyval.expr).valor.bval = (val1 <= val2); break; /* LE */
                        case 4: (yyval.expr).valor.bval = (val1 >= val2); break; /* GE */
                        case 5: (yyval.expr).valor.bval = (val1 != val2); break; /* NE */
                        case 6: (yyval.expr).valor.bval = (val1 == val2); break; /* EQ */
                    }
                }
            }
#line 1613 "y.tab.c"
    break;

  case 26: /* expr_logica: LP expr_logica OR expr_logica RP  */
#line 312 "parser.y"
                                             {
                if ((yyvsp[-3].expr).tipo != TIPO_BOOLEAN || (yyvsp[-1].expr).tipo != TIPO_BOOLEAN) {
                    yyerror("Operador OR requer operandos booleanos");
                    (yyval.expr).tipo = TIPO_BOOLEAN;
                    (yyval.expr).valor.bval = 0;
                } else {
                    (yyval.expr).tipo = TIPO_BOOLEAN;
                    (yyval.expr).valor.bval = ((yyvsp[-3].expr).valor.bval || (yyvsp[-1].expr).valor.bval);
                }
            }
#line 1628 "y.tab.c"
    break;

  case 27: /* expr_logica: LP expr_logica AND expr_logica RP  */
#line 322 "parser.y"
                                              {
                if ((yyvsp[-3].expr).tipo != TIPO_BOOLEAN || (yyvsp[-1].expr).tipo != TIPO_BOOLEAN) {
                    yyerror("Operador AND requer operandos booleanos");
                    (yyval.expr).tipo = TIPO_BOOLEAN;
                    (yyval.expr).valor.bval = 0;
                } else {
                    (yyval.expr).tipo = TIPO_BOOLEAN;
                    (yyval.expr).valor.bval = ((yyvsp[-3].expr).valor.bval && (yyvsp[-1].expr).valor.bval);
                }
            }
#line 1643 "y.tab.c"
    break;

  case 28: /* expr_logica: LP NOT expr_logica RP  */
#line 332 "parser.y"
                                  {
                if ((yyvsp[-1].expr).tipo != TIPO_BOOLEAN) {
                    yyerror("Operador NOT requer operando booleano");
                    (yyval.expr).tipo = TIPO_BOOLEAN;
                    (yyval.expr).valor.bval = 0;
                } else {
                    (yyval.expr).tipo = TIPO_BOOLEAN;
                    (yyval.expr).valor.bval = !(yyvsp[-1].expr).valor.bval;
                }
            }
#line 1658 "y.tab.c"
    break;

  case 29: /* expr_logica: ID  */
#line 342 "parser.y"
               {
                int pos = busca_var((yyvsp[0].str));
                if (pos == -1) {
                    yyerror("Variável não declarada");
                    (yyval.expr).tipo = TIPO_BOOLEAN;
                    (yyval.expr).valor.bval = 0;
                } else if (tabela[pos].tipo != TIPO_BOOLEAN) {
                    yyerror("Variável deve ser booleana");
                    (yyval.expr).tipo = TIPO_BOOLEAN;
                    (yyval.expr).valor.bval = 0;
                } else {
                    (yyval.expr).tipo = TIPO_BOOLEAN;
                    (yyval.expr).valor.bval = tabela[pos].valor.bval;
                }
            }
#line 1678 "y.tab.c"
    break;

  case 30: /* expr_logica: BOOL_VAL  */
#line 357 "parser.y"
                     {
                (yyval.expr).tipo = TIPO_BOOLEAN;
                (yyval.expr).valor.bval = (yyvsp[0].bval);
            }
#line 1687 "y.tab.c"
    break;

  case 31: /* op_cmp: LT  */
#line 363 "parser.y"
           { (yyval.expr).tipo = TIPO_INT; (yyval.expr).valor.ival = 1; }
#line 1693 "y.tab.c"
    break;

  case 32: /* op_cmp: GT  */
#line 364 "parser.y"
           { (yyval.expr).tipo = TIPO_INT; (yyval.expr).valor.ival = 2; }
#line 1699 "y.tab.c"
    break;

  case 33: /* op_cmp: LE  */
#line 365 "parser.y"
           { (yyval.expr).tipo = TIPO_INT; (yyval.expr).valor.ival = 3; }
#line 1705 "y.tab.c"
    break;

  case 34: /* op_cmp: GE  */
#line 366 "parser.y"
           { (yyval.expr).tipo = TIPO_INT; (yyval.expr).valor.ival = 4; }
#line 1711 "y.tab.c"
    break;

  case 35: /* op_cmp: NE  */
#line 367 "parser.y"
           { (yyval.expr).tipo = TIPO_INT; (yyval.expr).valor.ival = 5; }
#line 1717 "y.tab.c"
    break;

  case 36: /* op_cmp: EQ  */
#line 368 "parser.y"
           { (yyval.expr).tipo = TIPO_INT; (yyval.expr).valor.ival = 6; }
#line 1723 "y.tab.c"
    break;

  case 37: /* atrib: ID ASSIGN expr_aritmetica  */
#line 371 "parser.y"
                                 {
            int pos = busca_var((yyvsp[-2].str));
            if (pos == -1) {
                yyerror("Variável não declarada");
            } else {
                /* Verificar compatibilidade de tipos */
                valor_t valor;
                
                switch ((yyvsp[0].expr).tipo) {
                    case TIPO_INT:
                        valor.ival = (yyvsp[0].expr).valor.ival;
                        break;
                    case TIPO_FLOAT:
                        valor.fval = (yyvsp[0].expr).valor.fval;
                        break;
                    case TIPO_BOOLEAN:
                        valor.bval = (yyvsp[0].expr).valor.bval;
                        break;
                    case TIPO_CHAR:
                        valor.cval = (yyvsp[0].expr).valor.cval;
                        break;
                }
                
                atribui_valor(pos, (yyvsp[0].expr).tipo, valor);
            }
        }
#line 1754 "y.tab.c"
    break;

  case 39: /* expr_aritmetica: expr_aritmetica PLUS termo  */
#line 400 "parser.y"
                                            {
                    if ((yyvsp[-2].expr).tipo == TIPO_BOOLEAN || (yyvsp[0].expr).tipo == TIPO_BOOLEAN) {
                        yyerror("Operador + não pode ser usado com boolean");
                        (yyval.expr).tipo = TIPO_INT;
                        (yyval.expr).valor.ival = 0;
                    } else if ((yyvsp[-2].expr).tipo == TIPO_FLOAT || (yyvsp[0].expr).tipo == TIPO_FLOAT) {
                        (yyval.expr).tipo = TIPO_FLOAT;
                        float v1 = ((yyvsp[-2].expr).tipo == TIPO_INT) ? (float)(yyvsp[-2].expr).valor.ival : 
                               ((yyvsp[-2].expr).tipo == TIPO_CHAR) ? (float)(yyvsp[-2].expr).valor.cval : (yyvsp[-2].expr).valor.fval;
                        float v2 = ((yyvsp[0].expr).tipo == TIPO_INT) ? (float)(yyvsp[0].expr).valor.ival : 
                               ((yyvsp[0].expr).tipo == TIPO_CHAR) ? (float)(yyvsp[0].expr).valor.cval : (yyvsp[0].expr).valor.fval;
                        (yyval.expr).valor.fval = v1 + v2;
                    } else if ((yyvsp[-2].expr).tipo == TIPO_CHAR && (yyvsp[0].expr).tipo == TIPO_CHAR) {
                        yyerror("Operador + não pode ser usado com char");
                        (yyval.expr).tipo = TIPO_INT;
                        (yyval.expr).valor.ival = 0;
                    } else {
                        (yyval.expr).tipo = TIPO_INT;
                        int v1 = ((yyvsp[-2].expr).tipo == TIPO_INT) ? (yyvsp[-2].expr).valor.ival : (int)(yyvsp[-2].expr).valor.cval;
                        int v2 = ((yyvsp[0].expr).tipo == TIPO_INT) ? (yyvsp[0].expr).valor.ival : (int)(yyvsp[0].expr).valor.cval;
                        (yyval.expr).valor.ival = v1 + v2;
                    }
                }
#line 1782 "y.tab.c"
    break;

  case 40: /* expr_aritmetica: expr_aritmetica MINUS termo  */
#line 423 "parser.y"
                                             {
                    if ((yyvsp[-2].expr).tipo == TIPO_BOOLEAN || (yyvsp[0].expr).tipo == TIPO_BOOLEAN) {
                        yyerror("Operador - não pode ser usado com boolean");
                        (yyval.expr).tipo = TIPO_INT;
                        (yyval.expr).valor.ival = 0;
                    } else if ((yyvsp[-2].expr).tipo == TIPO_FLOAT || (yyvsp[0].expr).tipo == TIPO_FLOAT) {
                        (yyval.expr).tipo = TIPO_FLOAT;
                        float v1 = ((yyvsp[-2].expr).tipo == TIPO_INT) ? (float)(yyvsp[-2].expr).valor.ival : 
                               ((yyvsp[-2].expr).tipo == TIPO_CHAR) ? (float)(yyvsp[-2].expr).valor.cval : (yyvsp[-2].expr).valor.fval;
                        float v2 = ((yyvsp[0].expr).tipo == TIPO_INT) ? (float)(yyvsp[0].expr).valor.ival : 
                               ((yyvsp[0].expr).tipo == TIPO_CHAR) ? (float)(yyvsp[0].expr).valor.cval : (yyvsp[0].expr).valor.fval;
                        (yyval.expr).valor.fval = v1 - v2;
                    } else if ((yyvsp[-2].expr).tipo == TIPO_CHAR && (yyvsp[0].expr).tipo == TIPO_CHAR) {
                        yyerror("Operador - não pode ser usado com char");
                        (yyval.expr).tipo = TIPO_INT;
                        (yyval.expr).valor.ival = 0;
                    } else {
                        (yyval.expr).tipo = TIPO_INT;
                        int v1 = ((yyvsp[-2].expr).tipo == TIPO_INT) ? (yyvsp[-2].expr).valor.ival : (int)(yyvsp[-2].expr).valor.cval;
                        int v2 = ((yyvsp[0].expr).tipo == TIPO_INT) ? (yyvsp[0].expr).valor.ival : (int)(yyvsp[0].expr).valor.cval;
                        (yyval.expr).valor.ival = v1 - v2;
                    }
                }
#line 1810 "y.tab.c"
    break;

  case 42: /* termo: termo MULT fator  */
#line 449 "parser.y"
                        {
            if ((yyvsp[-2].expr).tipo == TIPO_BOOLEAN || (yyvsp[0].expr).tipo == TIPO_BOOLEAN) {
                yyerror("Operador * não pode ser usado com boolean");
                (yyval.expr).tipo = TIPO_INT;
                (yyval.expr).valor.ival = 0;
            } else if ((yyvsp[-2].expr).tipo == TIPO_FLOAT || (yyvsp[0].expr).tipo == TIPO_FLOAT) {
                (yyval.expr).tipo = TIPO_FLOAT;
                float v1 = ((yyvsp[-2].expr).tipo == TIPO_INT) ? (float)(yyvsp[-2].expr).valor.ival : 
                       ((yyvsp[-2].expr).tipo == TIPO_CHAR) ? (float)(yyvsp[-2].expr).valor.cval : (yyvsp[-2].expr).valor.fval;
                float v2 = ((yyvsp[0].expr).tipo == TIPO_INT) ? (float)(yyvsp[0].expr).valor.ival : 
                       ((yyvsp[0].expr).tipo == TIPO_CHAR) ? (float)(yyvsp[0].expr).valor.cval : (yyvsp[0].expr).valor.fval;
                (yyval.expr).valor.fval = v1 * v2;
            } else if ((yyvsp[-2].expr).tipo == TIPO_CHAR || (yyvsp[0].expr).tipo == TIPO_CHAR) {
                yyerror("Operador * não pode ser usado com char");
                (yyval.expr).tipo = TIPO_INT;
                (yyval.expr).valor.ival = 0;
            } else {
                (yyval.expr).tipo = TIPO_INT;
                (yyval.expr).valor.ival = (yyvsp[-2].expr).valor.ival * (yyvsp[0].expr).valor.ival;
            }
        }
#line 1836 "y.tab.c"
    break;

  case 43: /* termo: termo DIV fator  */
#line 470 "parser.y"
                       {
            if ((yyvsp[-2].expr).tipo == TIPO_BOOLEAN || (yyvsp[0].expr).tipo == TIPO_BOOLEAN) {
                yyerror("Operador / não pode ser usado com boolean");
                (yyval.expr).tipo = TIPO_INT;
                (yyval.expr).valor.ival = 0;
            } else if ((yyvsp[0].expr).tipo == TIPO_INT && (yyvsp[0].expr).valor.ival == 0) {
                yyerror("Divisão por zero");
                (yyval.expr).tipo = TIPO_INT;
                (yyval.expr).valor.ival = 0;
            } else if ((yyvsp[0].expr).tipo == TIPO_FLOAT && (yyvsp[0].expr).valor.fval == 0.0) {
                yyerror("Divisão por zero");
                (yyval.expr).tipo = TIPO_FLOAT;
                (yyval.expr).valor.fval = 0.0;
            } else if ((yyvsp[-2].expr).tipo == TIPO_FLOAT || (yyvsp[0].expr).tipo == TIPO_FLOAT) {
                (yyval.expr).tipo = TIPO_FLOAT;
                float v1 = ((yyvsp[-2].expr).tipo == TIPO_INT) ? (float)(yyvsp[-2].expr).valor.ival : 
                       ((yyvsp[-2].expr).tipo == TIPO_CHAR) ? (float)(yyvsp[-2].expr).valor.cval : (yyvsp[-2].expr).valor.fval;
                float v2 = ((yyvsp[0].expr).tipo == TIPO_INT) ? (float)(yyvsp[0].expr).valor.ival : 
                       ((yyvsp[0].expr).tipo == TIPO_CHAR) ? (float)(yyvsp[0].expr).valor.cval : (yyvsp[0].expr).valor.fval;
                (yyval.expr).valor.fval = v1 / v2;
            } else if ((yyvsp[-2].expr).tipo == TIPO_CHAR || (yyvsp[0].expr).tipo == TIPO_CHAR) {
                yyerror("Operador / não pode ser usado com char");
                (yyval.expr).tipo = TIPO_INT;
                (yyval.expr).valor.ival = 0;
            } else {
                (yyval.expr).tipo = TIPO_INT;
                (yyval.expr).valor.ival = (yyvsp[-2].expr).valor.ival / (yyvsp[0].expr).valor.ival;
            }
        }
#line 1870 "y.tab.c"
    break;

  case 44: /* fator: ID  */
#line 501 "parser.y"
          {
            int pos = busca_var((yyvsp[0].str));
            if (pos == -1) {
                yyerror("Variável não declarada");
                (yyval.expr).tipo = TIPO_INT;
                (yyval.expr).valor.ival = 0;
            } else {
                (yyval.expr).tipo = tabela[pos].tipo;
                switch (tabela[pos].tipo) {
                    case TIPO_INT:
                        (yyval.expr).valor.ival = tabela[pos].valor.ival;
                        break;
                    case TIPO_FLOAT:
                        (yyval.expr).valor.fval = tabela[pos].valor.fval;
                        break;
                    case TIPO_BOOLEAN:
                        (yyval.expr).valor.bval = tabela[pos].valor.bval;
                        break;
                    case TIPO_CHAR:
                        (yyval.expr).valor.cval = tabela[pos].valor.cval;
                        break;
                }
            }
        }
#line 1899 "y.tab.c"
    break;

  case 45: /* fator: NUM_INT  */
#line 525 "parser.y"
               {
            (yyval.expr).tipo = TIPO_INT;
            (yyval.expr).valor.ival = (yyvsp[0].num);
        }
#line 1908 "y.tab.c"
    break;

  case 46: /* fator: NUM_FLOAT  */
#line 529 "parser.y"
                 {
            (yyval.expr).tipo = TIPO_FLOAT;
            (yyval.expr).valor.fval = (yyvsp[0].fnum);
        }
#line 1917 "y.tab.c"
    break;

  case 47: /* fator: CHAR_VAL  */
#line 533 "parser.y"
                {
            (yyval.expr).tipo = TIPO_CHAR;
            (yyval.expr).valor.cval = (yyvsp[0].cval);
        }
#line 1926 "y.tab.c"
    break;

  case 48: /* fator: LP expr_aritmetica RP  */
#line 537 "parser.y"
                             {
            (yyval.expr) = (yyvsp[-1].expr);
        }
#line 1934 "y.tab.c"
    break;


#line 1938 "y.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 542 "parser.y"


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
