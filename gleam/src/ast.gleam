pub type Program {
  Program(statements: List(Statement))
}

pub type Statement {
  LetStatement(name: String, value: Expression)
}

pub type Expression {
  Expression
}
