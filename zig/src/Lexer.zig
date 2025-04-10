pub const Token = struct {
    type: Type,
    literal: []const u8,

    pub const Type = enum {
        illegal,

        identifier,
        integer,

        assign,
        plus,
        minus,
        bang,
        asterisk,
        slash,

        less_than,
        greater_than,
        equal,
        not_equal,

        comma,
        semicolon,

        left_paren,
        right_paren,
        left_brace,
        right_brace,

        function,
        let,
        true,
        false,
        @"if",
        @"else",
        @"return",
    };

    pub fn format(self: Token, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.print("{s}", .{self.literal});
    }
};

const Lexer = @This();

buffer: []const u8,
pos: usize,

pub fn init(buffer: []const u8) Lexer {
    return .{ .buffer = buffer, .pos = 0 };
}

pub fn nextToken(self: *Lexer) ?Token {
    self.skipWhitespace();

    const current = self.currentByte() orelse return null;
    var token = Token{ .type = undefined, .literal = self.buffer[self.pos .. self.pos + 1] };
    token.type = switch (current) {
        '=' => blk: {
            if (self.peekByte() == '=') {
                token.literal = self.buffer[self.pos .. self.pos + 2];
                self.advance();
                break :blk .equal;
            }
            break :blk .assign;
        },
        '+' => .plus,
        '-' => .minus,
        '!' => blk: {
            if (self.peekByte() == '=') {
                token.literal = self.buffer[self.pos .. self.pos + 2];
                self.advance();
                break :blk .not_equal;
            }
            break :blk .bang;
        },
        '/' => .slash,
        '*' => .asterisk,
        '<' => .less_than,
        '>' => .greater_than,
        ';' => .semicolon,
        '(' => .left_paren,
        ')' => .right_paren,
        ',' => .comma,
        '{' => .left_brace,
        '}' => .right_brace,
        else => blk: {
            if (isLetter(current)) {
                const literal = self.readIdentifier();
                const token_type = lookupIdent(literal);
                return .{ .type = token_type, .literal = literal };
            } else if (isDigit(current)) {
                const literal = self.readNumber();
                return .{ .type = .integer, .literal = literal };
            }
            break :blk .illegal;
        },
    };

    self.advance();
    return token;
}

fn advance(self: *Lexer) void {
    self.pos += 1;
}

fn skipWhitespace(self: *Lexer) void {
    while (self.currentByte()) |c| {
        if (!isWhitespace(c)) break;
        self.advance();
    }
}

fn currentByte(self: Lexer) ?u8 {
    return self.byteAtPos(self.pos);
}

fn peekByte(self: Lexer) ?u8 {
    return self.byteAtPos(self.pos + 1);
}

fn byteAtPos(self: Lexer, pos: usize) ?u8 {
    if (pos > self.buffer.len - 1) {
        return null;
    }
    return self.buffer[pos];
}

fn readIdentifier(self: *Lexer) []const u8 {
    const pos = self.pos;
    while (self.currentByte()) |current| {
        if (!isLetter(current)) break;
        self.advance();
    }
    return self.buffer[pos..self.pos];
}

fn readNumber(self: *Lexer) []const u8 {
    const pos = self.pos;
    while (self.currentByte()) |c| {
        if (!isDigit(c)) break;
        self.advance();
    }
    return self.buffer[pos..self.pos];
}

fn lookupIdent(identifier: []const u8) Token.Type {
    if (std.mem.eql(u8, identifier, "fn")) {
        return .function;
    } else if (std.mem.eql(u8, identifier, "let")) {
        return .let;
    } else if (std.mem.eql(u8, identifier, "true")) {
        return .true;
    } else if (std.mem.eql(u8, identifier, "false")) {
        return .false;
    } else if (std.mem.eql(u8, identifier, "if")) {
        return .@"if";
    } else if (std.mem.eql(u8, identifier, "else")) {
        return .@"else";
    } else if (std.mem.eql(u8, identifier, "return")) {
        return .@"return";
    }
    return .identifier;
}

fn isLetter(c: u8) bool {
    return 'a' <= c and c <= 'z' or 'A' <= c and c <= 'Z' or c == '_';
}

fn isDigit(c: u8) bool {
    return '0' <= c and c <= '9';
}

fn isWhitespace(c: u8) bool {
    return c == ' ' or c == '\t' or c == '\n' or c == '\r';
}

const std = @import("std");
