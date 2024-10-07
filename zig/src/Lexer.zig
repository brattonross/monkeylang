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

fn lookupIdent(literal: []const u8) Token.Type {
    if (std.mem.eql(u8, "fn", literal)) {
        return Token.Type.function;
    } else if (std.mem.eql(u8, "let", literal)) {
        return Token.Type.let;
    }
    return Token.Type.ident;
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

pub fn nextToken(self: *Lexer) Token {
    self.skipWhitespace();

    const token = switch (self.ch) {
        '=' => Token{ .type = Token.Type.assign, .literal = "=" },
        ';' => Token{ .type = Token.Type.semicolon, .literal = ";" },
        '(' => Token{ .type = Token.Type.lparen, .literal = "(" },
        ')' => Token{ .type = Token.Type.rparen, .literal = ")" },
        ',' => Token{ .type = Token.Type.comma, .literal = "," },
        '+' => Token{ .type = Token.Type.plus, .literal = "+" },
        '{' => Token{ .type = Token.Type.lbrace, .literal = "{" },
        '}' => Token{ .type = Token.Type.rbrace, .literal = "}" },
        0 => Token{ .type = Token.Type.eof, .literal = "" },
        else => {
            if (isLetter(self.ch)) {
                const literal = self.readIdentifier();
                return Token{ .type = lookupIdent(literal), .literal = literal };
            } else if (isDigit(self.ch)) {
                return Token{ .type = Token.Type.int, .literal = self.readNumber() };
            }
            return Token{ .type = Token.Type.illegal, .literal = &[_]u8{self.ch} };
        },
    };
    self.readChar();
    return token;
}

test "next_token" {
    const input =
        \\let five = 5;
        \\let ten = 10;
        \\
        \\let add = fn(x, y) {
        \\  x + y;
        \\};
        \\
        \\let result = add(five, ten);
    ;
    const test_cases = [_]struct { type: Token.Type, literal: []const u8 }{
        .{ .type = Token.Type.let, .literal = "let" },
        .{ .type = Token.Type.ident, .literal = "five" },
        .{ .type = Token.Type.assign, .literal = "=" },
        .{ .type = Token.Type.int, .literal = "5" },
        .{ .type = Token.Type.semicolon, .literal = ";" },
        .{ .type = Token.Type.let, .literal = "let" },
        .{ .type = Token.Type.ident, .literal = "ten" },
        .{ .type = Token.Type.assign, .literal = "=" },
        .{ .type = Token.Type.int, .literal = "10" },
        .{ .type = Token.Type.semicolon, .literal = ";" },
        .{ .type = Token.Type.let, .literal = "let" },
        .{ .type = Token.Type.ident, .literal = "add" },
        .{ .type = Token.Type.assign, .literal = "=" },
        .{ .type = Token.Type.function, .literal = "fn" },
        .{ .type = Token.Type.lparen, .literal = "(" },
        .{ .type = Token.Type.ident, .literal = "x" },
        .{ .type = Token.Type.comma, .literal = "," },
        .{ .type = Token.Type.ident, .literal = "y" },
        .{ .type = Token.Type.rparen, .literal = ")" },
        .{ .type = Token.Type.lbrace, .literal = "{" },
        .{ .type = Token.Type.ident, .literal = "x" },
        .{ .type = Token.Type.plus, .literal = "+" },
        .{ .type = Token.Type.ident, .literal = "y" },
        .{ .type = Token.Type.semicolon, .literal = ";" },
        .{ .type = Token.Type.rbrace, .literal = "}" },
        .{ .type = Token.Type.semicolon, .literal = ";" },
        .{ .type = Token.Type.let, .literal = "let" },
        .{ .type = Token.Type.ident, .literal = "result" },
        .{ .type = Token.Type.assign, .literal = "=" },
        .{ .type = Token.Type.ident, .literal = "add" },
        .{ .type = Token.Type.lparen, .literal = "(" },
        .{ .type = Token.Type.ident, .literal = "five" },
        .{ .type = Token.Type.comma, .literal = "," },
        .{ .type = Token.Type.ident, .literal = "ten" },
        .{ .type = Token.Type.rparen, .literal = ")" },
        .{ .type = Token.Type.semicolon, .literal = ";" },
        .{ .type = Token.Type.eof, .literal = "" },
    };

    var l = Lexer.init(input);
    for (test_cases) |expected| {
        const actual = l.nextToken();
        try std.testing.expect(expected.type == actual.type);
        try std.testing.expect(std.mem.eql(u8, expected.literal, actual.literal));
    }
}
