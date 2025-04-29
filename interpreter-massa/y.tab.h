/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

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

#line 159 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
