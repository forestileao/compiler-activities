
#[derive(Debug, Clone, PartialEq)]
pub enum Token {
    Int,
    Identifier(String),
    Number(i32),
    If,
    Else,
    Print,
    OpenParen,
    CloseParen,
    OpenBrace,
    CloseBrace,
    Equal,
    Greater,
    Less,
    Semicolon,
    StringLiteral(String),
}

pub fn lex(input: &str) -> Vec<Token> {
    let mut tokens = Vec::new();
    let mut chars = input.chars().peekable();

    while let Some(&c) = chars.peek() {
        match c {
            ' ' | '\n' | '\t' => {
                chars.next(); // Ignora espaços e quebras de linha
            }
            'a'..='z' | 'A'..='Z' => {
                let mut ident = String::new();
                while let Some(&ch) = chars.peek() {
                    if ch.is_alphanumeric() {
                        ident.push(ch);
                        chars.next();
                    } else {
                        break;
                    }
                }

                match ident.as_str() {
                    "int" => tokens.push(Token::Int),
                    "if" => tokens.push(Token::If),
                    "else" => tokens.push(Token::Else),
                    "print" => tokens.push(Token::Print),
                    _ => tokens.push(Token::Identifier(ident)),
                }
            }

            '0'..='9' => {
                let mut number = String::new();
                while let Some(&ch) = chars.peek() {
                    if ch.is_digit(10) {
                        number.push(ch);
                        chars.next();
                    } else {
                        break;
                    }
                }
                let value = number.parse::<i32>().unwrap();
                tokens.push(Token::Number(value));
            }
            '"' => {
                chars.next(); // Consome o "
                let mut string = String::new();
                while let Some(ch) = chars.next() {
                    if ch == '"' {
                        break;
                    } else {
                        string.push(ch);
                    }
                }
                tokens.push(Token::StringLiteral(string));
            }

            '=' => {
                chars.next();
                tokens.push(Token::Equal);
            }
            '>' => {
                chars.next();
                tokens.push(Token::Greater);
            }
            '<' => {
                chars.next();
                tokens.push(Token::Less);
            }
            ';' => {
                chars.next();
                tokens.push(Token::Semicolon);
            }
            '(' => {
                chars.next();
                tokens.push(Token::OpenParen);
            }
            ')' => {
                chars.next();
                tokens.push(Token::CloseParen);
            }
            '{' => {
                chars.next();
                tokens.push(Token::OpenBrace);
            }
            '}' => {
                chars.next();
                tokens.push(Token::CloseBrace);
            }
            _ => {
                panic!("Caractere inválido: {}", c);
            }
        }
    }

    tokens
}
