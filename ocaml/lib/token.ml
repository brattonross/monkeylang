type t =
  | Illegal
  (* Identifiers and literals *)
  | Ident of string
  | Integer of int
  (* Operators *)
  | Assign
  | Plus
  (* Delimiters *)
  | Comma
  | Semicolon
  | LParen
  | RParen
  | LBrace
  | RBrace
  (* Keywords *)
  | Function
  | Let
[@@deriving show, eq]
