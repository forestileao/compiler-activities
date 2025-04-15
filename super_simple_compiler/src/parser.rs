use crate::lexer::{Token, Token::*}; // Vamos supor que temos um módulo lexer

#[derive(Debug)]
pub enum Expr {
    Number(i32),
    StringLiteral(String),
    Variable(String),
    BinaryOp {
        left: Box<Expr>,
        op: String,
        right: Box<Expr>,
    },
}

#[derive(Debug)]
pub enum Statement {
    Declaration {
        var_type: String,
        name: String,
        value: Expr,
    },
    If {
        condition: Expr,
        then_branch: Vec<Statement>,
        else_branch: Vec<Statement>,
    },
    Print(Expr),
}


pub fn parse(tokens: &[Token]) -> Vec<Statement> {
    let mut parser = Parser {
        tokens,
        position: 0,
    };

    let mut statements = Vec::new();

    while !parser.is_at_end() {
        statements.push(parser.parse_statement());
    }

    statements
}

struct Parser<'a> {
    tokens: &'a [Token],
    position: usize,
}

impl<'a> Parser<'a> {
    fn is_at_end(&self) -> bool {
        self.position >= self.tokens.len()
    }

    fn peek(&self) -> &Token {
        &self.tokens[self.position]
    }

    fn advance(&mut self) -> &Token {
        let tok = &self.tokens[self.position];
        self.position += 1;
        tok
    }

    fn expect(&mut self, expected: Token) {
        let tok = self.advance();
        if tok != &expected {
            panic!("Esperado {:?}, encontrado {:?}", expected, tok);
        }
    }

    fn parse_statement(&mut self) -> Statement {
        match self.peek() {
            Int => self.parse_declaration(),
            If => self.parse_if(),
            Print => self.parse_print(),
            _ => panic!("Declaração inválida: {:?}", self.peek()),
        }
    }

    fn parse_declaration(&mut self) -> Statement {
        self.advance(); // consume 'int'
        let name = match self.advance() {
            Identifier(name) => name.clone(),
            tok => panic!("Esperado identificador, encontrado {:?}", tok),
        };
        self.expect(Equal);
        let value = self.parse_expression();
        self.expect(Semicolon);

        Statement::Declaration {
            var_type: "int".to_string(),
            name,
            value,
        }
    }

    fn parse_print(&mut self) -> Statement {
        self.advance(); // consume 'print'
        self.expect(OpenParen);
        let expr = self.parse_expression();
        self.expect(CloseParen);
        self.expect(Semicolon);
        Statement::Print(expr)
    }

    fn parse_if(&mut self) -> Statement {
        self.advance(); // consume 'if'
        self.expect(OpenParen);
        let condition = self.parse_expression();
        self.expect(CloseParen);
        self.expect(OpenBrace);
        let mut then_branch = Vec::new();
        while *self.peek() != CloseBrace {
            then_branch.push(self.parse_statement());
        }
        self.expect(CloseBrace);

        let mut else_branch = Vec::new();
        if let Else = self.peek() {
            self.advance(); // consume 'else'
            self.expect(OpenBrace);
            while *self.peek() != CloseBrace {
                else_branch.push(self.parse_statement());
            }
            self.expect(CloseBrace);
        }

        Statement::If {
            condition,
            then_branch,
            else_branch,
        }
    }

    fn parse_expression(&mut self) -> Expr {
        // Suporta apenas: var > var, número, string, variável
        let left = self.parse_primary();

        if let Greater = self.peek() {
            self.advance();
            let right = self.parse_primary();
            return Expr::BinaryOp {
                left: Box::new(left),
                op: ">".to_string(),
                right: Box::new(right),
            };
        }

        if let Less = self.peek() {
            self.advance();
            let right = self.parse_primary();
            return Expr::BinaryOp {
                left: Box::new(left),
                op: "<".to_string(),
                right: Box::new(right),
            };
        }

        left
    }

    fn parse_primary(&mut self) -> Expr {
        match self.advance() {
            Number(n) => Expr::Number(*n),
            StringLiteral(s) => Expr::StringLiteral(s.clone()),
            Identifier(name) => Expr::Variable(name.clone()),
            tok => panic!("Token inesperado: {:?}", tok),
        }
    }
}
