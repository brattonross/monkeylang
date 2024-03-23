use crate::token::Token;

pub struct Lexer {
    input: Vec<u8>,
    position: usize,
}

impl Lexer {
    pub fn new(input: String) -> Lexer {
        return Lexer {
            input: input.into_bytes(),
            position: 0,
        };
    }

    pub fn next_token(&mut self) -> Token {
        self.skip_whitespace();

        let token = match self.current_char() {
            Some(b'=') => match self.peek_char() {
                Some(b'=') => {
                    self.advance();
                    Token::Equal
                }
                _ => Token::Assign,
            },
            Some(b'+') => Token::Plus,
            Some(b'-') => Token::Minus,
            Some(b'!') => match self.peek_char() {
                Some(b'=') => {
                    self.advance();
                    Token::NotEqual
                }
                _ => Token::Bang,
            },
            Some(b'*') => Token::Asterisk,
            Some(b'/') => Token::Slash,
            Some(b'<') => Token::LessThan,
            Some(b'>') => Token::GreaterThan,
            Some(b';') => Token::Semicolon,
            Some(b'(') => Token::LParen,
            Some(b')') => Token::RParen,
            Some(b',') => Token::Comma,
            Some(b'{') => Token::LBrace,
            Some(b'}') => Token::RBrace,
            Some(b'a'..=b'z' | b'A'..=b'Z' | b'_') => {
                let identifier = self.read_identifier();
                return match identifier.as_str() {
                    "fn" => Token::Function,
                    "let" => Token::Let,
                    "if" => Token::If,
                    "else" => Token::Else,
                    "return" => Token::Return,
                    "true" => Token::True,
                    "false" => Token::False,
                    _ => Token::Identifier(identifier),
                };
            }
            Some(b'0'..=b'9') => {
                let number = self.read_number();
                return Token::Int(number);
            }
            None => Token::EOF,
            _ => Token::Illegal,
        };

        self.advance();
        return token;
    }

    fn advance(&mut self) {
        self.position += 1;
    }

    fn current_char(&self) -> Option<u8> {
        self.char_at(self.position)
    }

    fn peek_char(&self) -> Option<u8> {
        self.char_at(self.position + 1)
    }

    fn char_at(&self, position: usize) -> Option<u8> {
        if position >= self.input.len() {
            return None;
        } else {
            return Some(self.input[position]);
        }
    }

    fn skip_whitespace(&mut self) {
        while let Some(char) = self.current_char() {
            if char.is_ascii_whitespace() {
                self.advance();
            } else {
                break;
            }
        }
    }

    fn read_identifier(&mut self) -> String {
        let position = self.position;
        while let Some(char) = self.current_char() {
            if char.is_ascii_alphabetic() || char == b'_' {
                self.advance();
            } else {
                break;
            }
        }
        return String::from_utf8_lossy(&self.input[position..self.position]).to_string();
    }

    fn read_number(&mut self) -> String {
        let position = self.position;
        while let Some(char) = self.current_char() {
            if char.is_ascii_digit() {
                self.advance();
            } else {
                break;
            }
        }
        return String::from_utf8_lossy(&self.input[position..self.position]).to_string();
    }
}

#[cfg(test)]
mod tests {
    use super::{Lexer, Token};

    #[test]
    fn test_next_token() {
        let input = "
            let five = 5;
            let ten = 10;

            let add = fn(x, y) {
                x + y;
            };

            let result = add(five, ten);
            !-/*5;
            5 < 10 > 5;

            if (5 < 10) {
                return true;
            } else {
                return false;
            }

            10 == 10;
            10 != 9;
        ";
        let expected = vec![
            Token::Let,
            Token::Identifier(String::from("five")),
            Token::Assign,
            Token::Int(String::from("5")),
            Token::Semicolon,
            Token::Let,
            Token::Identifier(String::from("ten")),
            Token::Assign,
            Token::Int(String::from("10")),
            Token::Semicolon,
            Token::Let,
            Token::Identifier(String::from("add")),
            Token::Assign,
            Token::Function,
            Token::LParen,
            Token::Identifier(String::from("x")),
            Token::Comma,
            Token::Identifier(String::from("y")),
            Token::RParen,
            Token::LBrace,
            Token::Identifier(String::from("x")),
            Token::Plus,
            Token::Identifier(String::from("y")),
            Token::Semicolon,
            Token::RBrace,
            Token::Semicolon,
            Token::Let,
            Token::Identifier(String::from("result")),
            Token::Assign,
            Token::Identifier(String::from("add")),
            Token::LParen,
            Token::Identifier(String::from("five")),
            Token::Comma,
            Token::Identifier(String::from("ten")),
            Token::RParen,
            Token::Semicolon,
            Token::Bang,
            Token::Minus,
            Token::Slash,
            Token::Asterisk,
            Token::Int(String::from("5")),
            Token::Semicolon,
            Token::Int(String::from("5")),
            Token::LessThan,
            Token::Int(String::from("10")),
            Token::GreaterThan,
            Token::Int(String::from("5")),
            Token::Semicolon,
            Token::If,
            Token::LParen,
            Token::Int(String::from("5")),
            Token::LessThan,
            Token::Int(String::from("10")),
            Token::RParen,
            Token::LBrace,
            Token::Return,
            Token::True,
            Token::Semicolon,
            Token::RBrace,
            Token::Else,
            Token::LBrace,
            Token::Return,
            Token::False,
            Token::Semicolon,
            Token::RBrace,
            Token::Int(String::from("10")),
            Token::Equal,
            Token::Int(String::from("10")),
            Token::Semicolon,
            Token::Int(String::from("10")),
            Token::NotEqual,
            Token::Int(String::from("9")),
            Token::Semicolon,
            Token::EOF,
        ];

        let mut lexer = Lexer::new(input.to_string());

        for expected_token in expected {
            let token = lexer.next_token();
            assert_eq!(token, expected_token);
        }
    }
}
