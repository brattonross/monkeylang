use crate::ast::{
    Expression, ExpressionStatement, LetStatement, Program, ReturnStatement, Statement,
};
use crate::lexer::Lexer;
use crate::token::Token;

enum Precedence {
    Lowest,
    Equals,
    LessGreater,
    Sum,
    Product,
    Prefix,
    Call,
}

pub struct Parser {
    lexer: Lexer,
    current_token: Token,
    peek_token: Token,

    pub errors: Vec<String>,
}

impl Parser {
    pub fn new(mut lexer: Lexer) -> Parser {
        let current_token = lexer.next_token();
        let peek_token = lexer.next_token();

        return Parser {
            lexer,
            errors: vec![],
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
            Token::Return => self.parse_return_statement(),
            _ => self.parse_expression_statement(),
        }
    }

    fn parse_let_statement(&mut self) -> Result<Statement, String> {
        let name = match &self.peek_token {
            Token::Identifier(name) => name.to_string(),
            _ => {
                self.peek_error(Token::Identifier(String::from("")));
                return Err(String::from("Expected identifier"));
            }
        };
        self.next_token();

        if self.peek_token != Token::Assign {
            self.peek_error(Token::Assign);
            return Err(String::from("Expected assign"));
        }
        self.next_token();

        // TODO: This skips expressions until we encounter a semicolon
        while self.current_token != Token::Semicolon {
            self.next_token();
        }

        return Ok(Statement::Let(LetStatement {
            name: Token::Identifier(name),
            value: Expression::Identifier(Token::Identifier(String::from(""))),
        }));
    }

    fn parse_return_statement(&mut self) -> Result<Statement, String> {
        self.next_token();

        // TODO: This skips expressions until we encounter a semicolon
        while self.current_token != Token::Semicolon {
            self.next_token();
        }

        return Ok(Statement::Return(ReturnStatement {
            value: Expression::Identifier(Token::Identifier(String::from(""))),
        }));
    }

    fn parse_expression_statement(&mut self) -> Result<Statement, String> {
        let expression = self.parse_expression(Precedence::Lowest);
        if self.peek_token == Token::Semicolon {
            self.next_token();
        }
        return Ok(Statement::Expression(ExpressionStatement {
            expression: expression?,
        }));
    }

    fn parse_expression(&self, precedence: Precedence) -> Result<Expression, String> {
        match self.current_token {
            Token::Identifier(_) => self.parse_identifier(),
            _ => Err(String::from("Expected identifier")),
        }
    }

    fn parse_identifier(&self) -> Result<Expression, String> {
        Ok(Expression::Identifier(self.current_token.clone()))
    }

    fn peek_error(&mut self, expected: Token) {
        self.errors.push(format!(
            "Expected next token to be {:?}, got {:?} instead",
            expected, self.peek_token
        ));
    }
}

#[cfg(test)]
mod tests {
    use super::*;
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

        assert_eq!(parser.errors.len(), 0);
        assert_eq!(program.statements.len(), 3);

        let expected = vec!["x", "y", "foobar"];
        for (i, name) in expected.iter().enumerate() {
            let statement = &program.statements[i];
            match statement {
                Statement::Let(let_statement) => {
                    assert_eq!(let_statement.name, Token::Identifier(name.to_string()));
                }
                _ => panic!("Expected LetStatement"),
            }
        }
    }

    #[test]
    fn test_return_statements() {
        let input = String::from(
            "
            return 5;
            return 10;
            return 993322;
        ",
        );

        let lexer = Lexer::new(input);
        let mut parser = Parser::new(lexer);
        let program = parser.parse_program();

        assert_eq!(parser.errors.len(), 0);
        assert_eq!(program.statements.len(), 3);

        for statement in program.statements {
            match statement {
                Statement::Return(_) => {}
                _ => panic!("Expected ReturnStatement"),
            }
        }
    }

    #[test]
    fn test_identifier_expression() {
        let input = String::from("foobar;");

        let lexer = Lexer::new(input);
        let mut parser = Parser::new(lexer);
        let program = parser.parse_program();

        assert_eq!(parser.errors.len(), 0);
        assert_eq!(program.statements.len(), 1);

        let statement = &program.statements[0];
        let expression = match statement {
            Statement::Expression(expression_statement) => &expression_statement.expression,
            _ => panic!("Expected ExpressionStatement"),
        };
        let identifier = match expression {
            Expression::Identifier(token) => token,
            _ => panic!("Expected Identifier"),
        };
        assert_eq!(identifier, &Token::Identifier(String::from("foobar")));
    }
}
