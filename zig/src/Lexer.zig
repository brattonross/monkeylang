const Lexer = @This();

const std = @import("std");
const Token = @import("Token.zig");

input: []const u8,
pos: usize,
read_pos: usize,
ch: u8,

pub fn init(input: []const u8) Lexer {
    var l = Lexer{
        .input = input,
        .pos = 0,
        .read_pos = 0,
        .ch = 0,
    };
    l.readChar();
    return l;
}

fn readChar(self: *Lexer) void {
    if (self.read_pos >= self.input.len) {
        self.ch = 0;
    } else {
        self.ch = self.input[self.read_pos];
    }
    self.pos = self.read_pos;
    self.read_pos += 1;
}

fn isLetter(char: u8) bool {
    return switch (char) {
        'a'...'z', 'A'...'Z', '_' => true,
        else => false,
    };
}

fn readIdentifier(self: *Lexer) []const u8 {
    const pos = self.pos;
    while (isLetter(self.ch)) {
        self.readChar();
    }
    return self.input[pos..self.pos];
}

const ident_map = std.StaticStringMap(Token.Type).initComptime(.{
    .{ "fn", .function },
    .{ "let", .let },
    .{ "true", .true },
    .{ "false", .false },
    .{ "if", .@"if" },
    .{ "else", .@"else" },
    .{ "return", .@"return" },
});

fn lookupIdent(literal: []const u8) Token.Type {
    return ident_map.get(literal) orelse .ident;
}

fn isWhitespace(char: u8) bool {
    return switch (char) {
        ' ', '\t', '\n', '\r' => true,
        else => false,
    };
}

fn skipWhitespace(self: *Lexer) void {
    while (isWhitespace(self.ch)) {
        self.readChar();
    }
}

fn isDigit(char: u8) bool {
    return switch (char) {
        '0'...'9' => true,
        else => false,
    };
}

fn readNumber(self: *Lexer) []const u8 {
    const pos = self.pos;
    while (isDigit(self.ch)) {
        self.readChar();
    }
    return self.input[pos..self.pos];
}

fn peekChar(self: Lexer) u8 {
    if (self.read_pos >= self.input.len) {
        return 0;
    }
    return self.input[self.read_pos];
}

pub fn nextToken(self: *Lexer) Token {
    self.skipWhitespace();

    const token = switch (self.ch) {
        '=' => blk: {
            switch (self.peekChar()) {
                '=' => {
                    self.readChar();
                    break :blk Token.eq;
                },
                else => break :blk Token.assign,
            }
        },
        '+' => Token.plus,
        '-' => Token.minus,
        '!' => blk: {
            switch (self.peekChar()) {
                '=' => {
                    self.readChar();
                    break :blk Token.ne;
                },
                else => break :blk Token.bang,
            }
        },
        '/' => Token.slash,
        '*' => Token.asterisk,
        '<' => Token.lt,
        '>' => Token.gt,
        ',' => Token.comma,
        ';' => Token.semicolon,
        '(' => Token.lparen,
        ')' => Token.rparen,
        '{' => Token.lbrace,
        '}' => Token.rbrace,
        0 => Token.eof,
        else => {
            if (isLetter(self.ch)) {
                const literal = self.readIdentifier();
                return Token{ .type = lookupIdent(literal), .literal = literal };
            } else if (isDigit(self.ch)) {
                return Token.int(self.readNumber());
            }
            return Token{ .type = .illegal, .literal = &[_]u8{self.ch} };
        },
    };
    self.readChar();
    return token;
}

test "nextToken" {
    const input =
        \\let five = 5;
        \\let ten = 10;
        \\
        \\let add = fn(x, y) {
        \\  x + y;
        \\};
        \\
        \\let result = add(five, ten);
        \\!-/*5;
        \\5 < 10 > 5;
        \\
        \\if (5 < 10) {
        \\  return true;
        \\} else {
        \\  return false;
        \\}
        \\
        \\10 == 10;
        \\10 != 9;
    ;
    const test_cases = [_]Token{
        Token.let,
        Token.ident("five"),
        Token.assign,
        Token.int("5"),
        Token.semicolon,
        Token.let,
        Token.ident("ten"),
        Token.assign,
        Token.int("10"),
        Token.semicolon,
        Token.let,
        Token.ident("add"),
        Token.assign,
        Token.function,
        Token.lparen,
        Token.ident("x"),
        Token.comma,
        Token.ident("y"),
        Token.rparen,
        Token.lbrace,
        Token.ident("x"),
        Token.plus,
        Token.ident("y"),
        Token.semicolon,
        Token.rbrace,
        Token.semicolon,
        Token.let,
        Token.ident("result"),
        Token.assign,
        Token.ident("add"),
        Token.lparen,
        Token.ident("five"),
        Token.comma,
        Token.ident("ten"),
        Token.rparen,
        Token.semicolon,
        Token.bang,
        Token.minus,
        Token.slash,
        Token.asterisk,
        Token.int("5"),
        Token.semicolon,
        Token.int("5"),
        Token.lt,
        Token.int("10"),
        Token.gt,
        Token.int("5"),
        Token.semicolon,
        Token.@"if",
        Token.lparen,
        Token.int("5"),
        Token.lt,
        Token.int("10"),
        Token.rparen,
        Token.lbrace,
        Token.@"return",
        Token.true,
        Token.semicolon,
        Token.rbrace,
        Token.@"else",
        Token.lbrace,
        Token.@"return",
        Token.false,
        Token.semicolon,
        Token.rbrace,
        Token.int("10"),
        Token.eq,
        Token.int("10"),
        Token.semicolon,
        Token.int("10"),
        Token.ne,
        Token.int("9"),
        Token.semicolon,
        Token.eof,
    };

    var l = Lexer.init(input);
    for (test_cases) |expected| {
        const actual = l.nextToken();
        try std.testing.expect(expected.type == actual.type);
        try std.testing.expect(std.mem.eql(u8, expected.literal, actual.literal));
    }
}
