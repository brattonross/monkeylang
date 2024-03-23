#[derive(Debug)]
pub struct Expression {}

#[derive(Debug)]
pub enum Statement {
    Let(LetStatement),
}

#[derive(Debug)]
pub struct LetStatement {
    pub name: String,
    pub value: Expression,
}

pub struct Program {
    pub statements: Vec<Statement>,
}
