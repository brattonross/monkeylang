use crate::ast::{Expression, LetStatement, Program, Statement};
use crate::lexer::Lexer;
use crate::token::Token;

pub struct Parser {
    lexer: Lexer,

    current_token: Token,
    peek_token: Token,
}

impl Parser {
    pub fn new(mut lexer: Lexer) -> Parser {
        let current_token = lexer.next_token();
        let peek_token = lexer.next_token();
        return Parser {
            lexer,
            current_token,
            peek_token,
        };
    }

    pub fn parse_program(&mut self) -> Program {
        let mut program = Program { statements: vec![] };
        while self.current_token != Token::EOF {
            if let Ok(statement) = self.parse_statement() {
                program.statements.push(statement)
            }
            self.next_token();
        }
        return program;
    }

    fn next_token(&mut self) {
        self.current_token = self.peek_token.clone();
        self.peek_token = self.lexer.next_token();
    }

    fn parse_statement(&mut self) -> Result<Statement, String> {
        match self.current_token {
            Token::Let => self.parse_let_statement(),
            _ => Err(String::from("Unsupported token")),
        }
    }

    fn parse_let_statement(&mut self) -> Result<Statement, String> {
        let name = match &self.peek_token {
            Token::Identifier(name) => name.to_string(),
            _ => return Err(String::from("Expected identifier")),
        };
        self.next_token();

        if self.peek_token != Token::Assign {
            return Err(String::from("Expected assign"));
        }
        self.next_token();

        // TODO: This skips expressions until we encounter a semicolon
        while self.current_token != Token::Semicolon {
            self.next_token();
        }

        return Ok(Statement::Let(LetStatement {
            name: name.to_string(),
            value: Expression {},
        }));
    }
}

#[cfg(test)]
mod tests {
    use super::{Lexer, Parser};
    use crate::ast::Statement;

    #[test]
    fn test_let_statements() {
        let input = String::from(
            "
            let x = 5;
            let y = 10;
            let foobar = 838383;
        ",
        );

        let lexer = Lexer::new(input);
        let mut parser = Parser::new(lexer);
        let program = parser.parse_program();

        assert_eq!(program.statements.len(), 3);

        let expected = vec!["x", "y", "foobar"];
        for (i, name) in expected.iter().enumerate() {
            let statement = &program.statements[i];
            match statement {
                Statement::Let(let_statement) => {
                    assert_eq!(let_statement.name, *name);
                }
            }
        }
    }
}
