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
            Token::False => Ok(Expression::Boolean(false)),
            Token::Identifier(_) => self.parse_identifier(),
            Token::Int(_) => self.parse_integer_literal(),
            Token::LParen => self.parse_grouped_expression(),
            Token::Minus => self.parse_prefix_expression(),
            Token::True => Ok(Expression::Boolean(true)),
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

    fn parse_grouped_expression(&mut self) -> Result<Expression, String> {
        self.next_token();
        let expression = self.parse_expression(Precedence::Lowest)?;
        if self.peek_token != Token::RParen {
            return Err(String::from("Expected closing parenthesis"));
        }
        self.next_token();
        return Ok(expression);
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
            ("!5;", PrefixOperator::Bang, Expression::IntegerLiteral(5)),
            (
                "-15;",
                PrefixOperator::Minus,
                Expression::IntegerLiteral(15),
            ),
            ("!true;", PrefixOperator::Bang, Expression::Boolean(true)),
            ("!false;", PrefixOperator::Bang, Expression::Boolean(false)),
        ];

        for (input, operator, expected_right) in tests {
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
            assert_eq!(*prefix.right, expected_right);
        }
    }

    #[test]
    fn test_parse_infix_expressions() {
        let tests = vec![
            (
                "5 + 5;",
                Expression::IntegerLiteral(5),
                InfixOperator::Plus,
                Expression::IntegerLiteral(5),
            ),
            (
                "5 - 5;",
                Expression::IntegerLiteral(5),
                InfixOperator::Minus,
                Expression::IntegerLiteral(5),
            ),
            (
                "5 * 5;",
                Expression::IntegerLiteral(5),
                InfixOperator::Asterisk,
                Expression::IntegerLiteral(5),
            ),
            (
                "5 / 5;",
                Expression::IntegerLiteral(5),
                InfixOperator::Slash,
                Expression::IntegerLiteral(5),
            ),
            (
                "5 > 5;",
                Expression::IntegerLiteral(5),
                InfixOperator::GreaterThan,
                Expression::IntegerLiteral(5),
            ),
            (
                "5 < 5;",
                Expression::IntegerLiteral(5),
                InfixOperator::LessThan,
                Expression::IntegerLiteral(5),
            ),
            (
                "5 == 5;",
                Expression::IntegerLiteral(5),
                InfixOperator::Equal,
                Expression::IntegerLiteral(5),
            ),
            (
                "5 != 5;",
                Expression::IntegerLiteral(5),
                InfixOperator::NotEqual,
                Expression::IntegerLiteral(5),
            ),
            (
                "true == true",
                Expression::Boolean(true),
                InfixOperator::Equal,
                Expression::Boolean(true),
            ),
            (
                "true != false",
                Expression::Boolean(true),
                InfixOperator::NotEqual,
                Expression::Boolean(false),
            ),
            (
                "false == false",
                Expression::Boolean(false),
                InfixOperator::Equal,
                Expression::Boolean(false),
            ),
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
            assert_eq!(*infix.left, expected_left);
            assert_eq!(*infix.right, expected_right);
        }
    }

    #[test]
    fn test_operator_precedence_parsing() {
        let tests = vec![
            ("-a * b;", "((-a) * b)"),
            ("!-a;", "(!(-a))"),
            ("a + b + c;", "((a + b) + c)"),
            ("a + b - c;", "((a + b) - c)"),
            ("a * b * c;", "((a * b) * c)"),
            ("a * b / c;", "((a * b) / c)"),
            ("a + b / c;", "(a + (b / c))"),
            ("a + b * c + d / e - f;", "(((a + (b * c)) + (d / e)) - f)"),
            ("3 + 4; -5 * 5;", "(3 + 4)((-5) * 5)"),
            ("5 > 4 == 3 < 4;", "((5 > 4) == (3 < 4))"),
            ("5 < 4 != 3 > 4;", "((5 < 4) != (3 > 4))"),
            (
                "3 + 4 * 5 == 3 * 1 + 4 * 5;",
                "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))",
            ),
            ("true;", "true"),
            ("false;", "false"),
            ("3 > 5 == false;", "((3 > 5) == false)"),
            ("3 < 5 == true;", "((3 < 5) == true)"),
            ("1 + (2 + 3) + 4;", "((1 + (2 + 3)) + 4)"),
            ("(5 + 5) * 2;", "((5 + 5) * 2)"),
            ("2 / (5 + 5);", "(2 / (5 + 5))"),
            ("-(5 + 5);", "(-(5 + 5))"),
            ("!(true == true);", "(!(true == true))"),
        ];

        for (input, expected) in tests {
            let lexer = Lexer::new(String::from(input));
            let mut parser = Parser::new(lexer);
            let program = parser.parse_program();
            assert_eq!(parser.errors, Vec::<String>::new());
            assert_eq!(program.to_string(), expected);
        }
    }

    #[test]
    fn test_boolean_expression() {
        let tests = vec![("true;", true), ("false;", false)];
        for (input, expected) in tests {
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
            let boolean = match expression {
                Expression::Boolean(token) => token,
                _ => panic!("Expected Boolean"),
            };
            assert_eq!(boolean, &expected);
        }
    }
}
