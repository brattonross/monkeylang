use crate::lexer::{Lexer, Token};

pub mod lexer;

fn main() {
    loop {
        let mut input = String::new();
        std::io::stdin()
            .read_line(&mut input)
            .expect("Failed to read from stdin");
        let mut l = Lexer::new(input);
        loop {
            let token = l.next_token();
            if token == Token::Eof {
                break;
            }
            println!("{:?}", token);
        }
    }
}
