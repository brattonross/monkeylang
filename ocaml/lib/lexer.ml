type t =
  { input : string
  ; position : int
  ; read_position : int
  ; ch : char option
  }
[@@deriving show]

let read_char lexer =
  if lexer.read_position >= String.length lexer.input
  then { lexer with ch = None }
  else
    { lexer with
      ch = Some (String.get lexer.input lexer.read_position)
    ; position = lexer.read_position
    ; read_position = lexer.read_position + 1
    }
;;

let make input = { input; position = 0; read_position = 0; ch = None } |> read_char

let next_token lexer =
  match lexer.ch with
  | None -> read_char lexer, None
  | Some ch ->
    (match ch with
     | '=' -> read_char lexer, Some Token.Assign
     | ';' -> read_char lexer, Some Token.Semicolon
     | '(' -> read_char lexer, Some Token.LParen
     | ')' -> read_char lexer, Some Token.RParen
     | ',' -> read_char lexer, Some Token.Comma
     | '+' -> read_char lexer, Some Token.Plus
     | '{' -> read_char lexer, Some Token.LBrace
     | '}' -> read_char lexer, Some Token.RBrace
     | _ -> Fmt.failwith "unknown char: %c" ch)
;;

let%test "next_token" =
  let input = "=+(){},;" in
  let expected =
    [ Token.Assign
    ; Token.Plus
    ; Token.LParen
    ; Token.RParen
    ; Token.LBrace
    ; Token.RBrace
    ; Token.Comma
    ; Token.Semicolon
    ]
  in
  let lexer = make input in
  let rec loop lexer tokens =
    let lexer, token = next_token lexer in
    match token with
    | None -> List.rev tokens
    | Some token -> loop lexer (token :: tokens)
  in
  let tokens = loop lexer [] in
  List.for_all2 Token.equal tokens expected
;;
