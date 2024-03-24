use crate::ast::{
    Expression, ExpressionStatement, InfixExpression, InfixOperator, LetStatement,
    PrefixExpression, PrefixOperator, Program, ReturnStatement, Statement,
};
use crate::lexer::Lexer;
use crate::token::Token;

#[derive(Debug, PartialEq, PartialOrd)]
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
            value: Expression::Identifier(String::from("")),
        }));
    }

    fn parse_return_statement(&mut self) -> Result<Statement, String> {
        self.next_token();

        // TODO: This skips expressions until we encounter a semicolon
        while self.current_token != Token::Semicolon {
            self.next_token();
        }

        return Ok(Statement::Return(ReturnStatement {
            value: Expression::Identifier(String::from("")),
        }));
    }

    fn parse_expression_statement(&mut self) -> Result<Statement, String> {
        let expression = self.parse_expression(Precedence::Lowest)?;
        if self.peek_token == Token::Semicolon {
            self.next_token();
        }
        return Ok(Statement::Expression(ExpressionStatement { expression }));
    }

    fn parse_expression(&mut self, precedence: Precedence) -> Result<Expression, String> {
        let left = match self.current_token {
            Token::Bang => self.parse_prefix_expression(),
            Token::Identifier(_) => self.parse_identifier(),
            Token::Int(_) => self.parse_integer_literal(),
            Token::Minus => self.parse_prefix_expression(),
            _ => {
                self.errors.push(format!(
                    "No prefix parse function for {:?}",
                    self.current_token
                ));
                Err(String::from("Expected identifier"))
            }
        }?;

        let mut result = left;
        while self.peek_token != Token::Semicolon && precedence < self.peek_precendence() {
            self.next_token();
            result = self.parse_infix_expression(Box::new(result))?;
        }
        return Ok(result);
    }

    fn parse_identifier(&self) -> Result<Expression, String> {
        match &self.current_token {
            Token::Identifier(name) => Ok(Expression::Identifier(name.to_string())),
            _ => Err(String::from("Expected identifier")),
        }
    }

    fn parse_integer_literal(&self) -> Result<Expression, String> {
        let value = match &self.current_token {
            Token::Int(value) => match value.parse() {
                Ok(value) => value,
                Err(_) => return Err(String::from("Expected integer")),
            },
            _ => return Err(String::from("Expected integer")),
        };
        return Ok(Expression::IntegerLiteral(value));
    }

    fn parse_prefix_expression(&mut self) -> Result<Expression, String> {
        let operator = match self.current_token {
            Token::Bang => PrefixOperator::Bang,
            Token::Minus => PrefixOperator::Minus,
            _ => return Err(String::from("Expected prefix operator")),
        };
        self.next_token();
        let expression = self.parse_expression(Precedence::Prefix)?;
        let right = Box::new(expression);
        return Ok(Expression::Prefix(PrefixExpression { operator, right }));
    }

    fn parse_infix_expression(&mut self, left: Box<Expression>) -> Result<Expression, String> {
        let operator = match self.current_token {
            Token::Plus => InfixOperator::Plus,
            Token::Minus => InfixOperator::Minus,
            Token::Asterisk => InfixOperator::Asterisk,
            Token::Slash => InfixOperator::Slash,
            Token::Equal => InfixOperator::Equal,
            Token::NotEqual => InfixOperator::NotEqual,
            Token::LessThan => InfixOperator::LessThan,
            Token::GreaterThan => InfixOperator::GreaterThan,
            _ => return Err(String::from("Expected infix operator")),
        };
        let precedence = self.current_precedence();

        self.next_token();

        let expression = self.parse_expression(precedence)?;
        let right = Box::new(expression);

        return Ok(Expression::Infix(InfixExpression {
            left,
            operator,
            right,
        }));
    }

    fn peek_precendence(&self) -> Precedence {
        Parser::map_precedence(&self.peek_token)
    }

    fn current_precedence(&self) -> Precedence {
        Parser::map_precedence(&self.current_token)
    }

    fn map_precedence(token: &Token) -> Precedence {
        match token {
            Token::Equal => Precedence::Equals,
            Token::NotEqual => Precedence::Equals,
            Token::LessThan => Precedence::LessGreater,
            Token::GreaterThan => Precedence::LessGreater,
            Token::Plus => Precedence::Sum,
            Token::Minus => Precedence::Sum,
            Token::Asterisk => Precedence::Product,
            Token::Slash => Precedence::Product,
            _ => Precedence::Lowest,
        }
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
    use crate::ast::{InfixOperator, PrefixOperator, Statement};

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

        assert_eq!(parser.errors, Vec::<String>::new());
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

        assert_eq!(parser.errors, Vec::<String>::new());
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

        assert_eq!(parser.errors, Vec::<String>::new());
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
        assert_eq!(identifier, &String::from("foobar"));
    }

    #[test]
    fn test_integer_literal_expression() {
        let input = String::from("5;");

        let lexer = Lexer::new(input);
        let mut parser = Parser::new(lexer);
        let program = parser.parse_program();

        assert_eq!(parser.errors, Vec::<String>::new());
        assert_eq!(program.statements.len(), 1);

        let statement = &program.statements[0];
        let expression = match statement {
            Statement::Expression(expression_statement) => &expression_statement.expression,
            _ => panic!("Expected ExpressionStatement"),
        };
        let integer = match expression {
            Expression::IntegerLiteral(token) => token,
            _ => panic!("Expected Integer"),
        };
        assert_eq!(integer, &5);
    }

    #[test]
    fn test_parse_prefix_expressions() {
        let tests = vec![
            ("!5;", PrefixOperator::Bang, 5),
            ("-15;", PrefixOperator::Minus, 15),
        ];

        for (input, operator, value) in tests {
            let lexer = Lexer::new(String::from(input));
            let mut parser = Parser::new(lexer);
            let program = parser.parse_program();

            assert_eq!(parser.errors, Vec::<String>::new());
            assert_eq!(program.statements.len(), 1);

            let statement = &program.statements[0];
            let expression = match statement {
                Statement::Expression(expression_statement) => &expression_statement.expression,
                _ => panic!("Expected ExpressionStatement"),
            };
            let prefix = match expression {
                Expression::Prefix(expression) => expression,
                _ => panic!("Expected Prefix"),
            };
            assert_eq!(prefix.operator, operator);

            let integer = match *prefix.right {
                Expression::IntegerLiteral(token) => token,
                _ => panic!("Expected Integer"),
            };
            assert_eq!(integer, value);
        }
    }

    #[test]
    fn test_parse_infix_expressions() {
        let tests = vec![
            ("5 + 5;", 5, InfixOperator::Plus, 5),
            ("5 - 5;", 5, InfixOperator::Minus, 5),
            ("5 * 5;", 5, InfixOperator::Asterisk, 5),
            ("5 / 5;", 5, InfixOperator::Slash, 5),
            ("5 > 5;", 5, InfixOperator::GreaterThan, 5),
            ("5 < 5;", 5, InfixOperator::LessThan, 5),
            ("5 == 5;", 5, InfixOperator::Equal, 5),
            ("5 != 5;", 5, InfixOperator::NotEqual, 5),
        ];

        for (input, expected_left, expected_operator, expected_right) in tests {
            let lexer = Lexer::new(String::from(input));
            let mut parser = Parser::new(lexer);
            let program = parser.parse_program();

            assert_eq!(parser.errors, Vec::<String>::new());
            assert_eq!(program.statements.len(), 1);

            let statement = &program.statements[0];
            let expression = match statement {
                Statement::Expression(expression_statement) => &expression_statement.expression,
                _ => panic!("Expected ExpressionStatement"),
            };
            let infix = match expression {
                Expression::Infix(expression) => expression,
                _ => panic!("Expected Infix"),
            };
            assert_eq!(infix.operator, expected_operator);
            let left = match *infix.left {
                Expression::IntegerLiteral(token) => token,
                _ => panic!("Expected Integer"),
            };
            assert_eq!(left, expected_left);
            let right = match *infix.right {
                Expression::IntegerLiteral(token) => token,
                _ => panic!("Expected Integer"),
            };
            assert_eq!(right, expected_right);
        }
    }
}
