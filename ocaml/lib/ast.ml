type node =
  | Program of program
  | Statement of statement
  | Expression of expression

and program = { statements : statement list }

and statement =
  | LetStatement of
      { name : identifier
      ; value : expression
      }

and expression = None
and identifier = string

let token_literal = function
  | Program _ -> "program"
  | Statement _ -> "statement"
  | Expression _ -> "expression"
;;
