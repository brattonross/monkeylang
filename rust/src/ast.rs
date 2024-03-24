use crate::token::Token;
use std::fmt;

#[derive(Debug)]
pub enum Statement {
    Let(LetStatement),
    Return(ReturnStatement),
    Expression(ExpressionStatement),
}

impl fmt::Display for Statement {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Statement::Let(statement) => write!(f, "{}", statement),
            Statement::Return(statement) => write!(f, "{}", statement),
            Statement::Expression(statement) => write!(f, "{}", statement),
        }
    }
}

#[derive(Debug)]
pub struct LetStatement {
    pub name: Token,
    pub value: Expression,
}

impl fmt::Display for LetStatement {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "let {} = {};", self.name, self.value)
    }
}

#[derive(Debug)]
pub struct ReturnStatement {
    pub value: Expression,
}

impl fmt::Display for ReturnStatement {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "return {};", self.value)
    }
}

#[derive(Debug, PartialEq)]
pub enum PrefixOperator {
    Bang,
    Minus,
}

impl fmt::Display for PrefixOperator {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            PrefixOperator::Bang => write!(f, "!"),
            PrefixOperator::Minus => write!(f, "-"),
        }
    }
}

#[derive(Debug, PartialEq)]
pub struct PrefixExpression {
    pub operator: PrefixOperator,
    pub right: Box<Expression>,
}

#[derive(Debug, PartialEq)]
pub enum InfixOperator {
    Plus,
    Minus,
    Asterisk,
    Slash,
    Equal,
    NotEqual,
    LessThan,
    GreaterThan,
}

impl fmt::Display for InfixOperator {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            InfixOperator::Plus => write!(f, "+"),
            InfixOperator::Minus => write!(f, "-"),
            InfixOperator::Asterisk => write!(f, "*"),
            InfixOperator::Slash => write!(f, "/"),
            InfixOperator::Equal => write!(f, "=="),
            InfixOperator::NotEqual => write!(f, "!="),
            InfixOperator::LessThan => write!(f, "<"),
            InfixOperator::GreaterThan => write!(f, ">"),
        }
    }
}

#[derive(Debug, PartialEq)]
pub struct InfixExpression {
    pub left: Box<Expression>,
    pub operator: InfixOperator,
    pub right: Box<Expression>,
}

#[derive(Debug, PartialEq)]
pub enum Expression {
    Identifier(String),
    IntegerLiteral(isize),
    Prefix(PrefixExpression),
    Infix(InfixExpression),
    Boolean(bool),
}

impl fmt::Display for Expression {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Expression::Identifier(token) => write!(f, "{}", token),
            Expression::IntegerLiteral(token) => write!(f, "{}", token),
            Expression::Prefix(expression) => {
                write!(f, "({}{})", expression.operator, expression.right)
            }
            Expression::Infix(expression) => {
                write!(
                    f,
                    "({} {} {})",
                    expression.left, expression.operator, expression.right
                )
            }
            Expression::Boolean(token) => write!(f, "{}", token),
        }
    }
}

#[derive(Debug)]
pub struct ExpressionStatement {
    pub expression: Expression,
}

impl fmt::Display for ExpressionStatement {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.expression)
    }
}

pub struct Program {
    pub statements: Vec<Statement>,
}

impl fmt::Display for Program {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let mut output = String::new();
        for statement in &self.statements {
            output.push_str(&format!("{}", statement));
        }
        write!(f, "{}", output)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::Token::Identifier;

    #[test]
    fn test_display() {
        let program = Program {
            statements: vec![Statement::Let(LetStatement {
                name: Identifier(String::from("myVar")),
                value: Expression::Identifier(String::from("anotherVar")),
            })],
        };

        assert_eq!(format!("{}", program), "let myVar = anotherVar;");
    }
}
