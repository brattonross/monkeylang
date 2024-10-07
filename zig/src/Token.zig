const Token = @This();

type: Type,
literal: []const u8,

pub const Type = enum {
    illegal,
    eof,

    ident,
    int,

    assign,
    plus,
    minus,
    bang,
    asterisk,
    slash,

    lt,
    gt,
    eq,
    ne,

    comma,
    semicolon,

    lparen,
    rparen,
    lbrace,
    rbrace,

    function,
    let,
    true,
    false,
    @"if",
    @"else",
    @"return",
};

pub const eof = Token{ .type = .eof, .literal = "" };
pub const assign = Token{ .type = .assign, .literal = "=" };
pub const plus = Token{ .type = .plus, .literal = "+" };
pub const minus = Token{ .type = .minus, .literal = "-" };
pub const bang = Token{ .type = .bang, .literal = "!" };
pub const asterisk = Token{ .type = .asterisk, .literal = "*" };
pub const slash = Token{ .type = .slash, .literal = "/" };
pub const lt = Token{ .type = .lt, .literal = "<" };
pub const gt = Token{ .type = .gt, .literal = ">" };
pub const eq = Token{ .type = .eq, .literal = "==" };
pub const ne = Token{ .type = .ne, .literal = "!=" };
pub const comma = Token{ .type = .comma, .literal = "," };
pub const semicolon = Token{ .type = .semicolon, .literal = ";" };
pub const lparen = Token{ .type = .lparen, .literal = "(" };
pub const rparen = Token{ .type = .rparen, .literal = ")" };
pub const lbrace = Token{ .type = .lbrace, .literal = "{" };
pub const rbrace = Token{ .type = .rbrace, .literal = "}" };
pub const function = Token{ .type = .function, .literal = "fn" };
pub const let = Token{ .type = .let, .literal = "let" };
pub const @"true" = Token{ .type = .true, .literal = "true" };
pub const @"false" = Token{ .type = .false, .literal = "false" };
pub const @"if" = Token{ .type = .@"if", .literal = "if" };
pub const @"else" = Token{ .type = .@"else", .literal = "else" };
pub const @"return" = Token{ .type = .@"return", .literal = "return" };

pub fn ident(l: []const u8) Token {
    return Token{ .type = .ident, .literal = l };
}

pub fn int(l: []const u8) Token {
    return Token{ .type = .int, .literal = l };
}
