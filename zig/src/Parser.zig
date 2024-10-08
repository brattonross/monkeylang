const std = @import("std");
const ast = @import("ast.zig");
const Lexer = @import("Lexer.zig");
const Token = @import("Token.zig");

const Parser = @This();

allocator: std.mem.Allocator,
lexer: Lexer,
cur_token: Token,
peek_token: Token,
errors: std.ArrayList([]const u8),

pub fn init(allocator: std.mem.Allocator, lexer: Lexer) Parser {
    var p = Parser{ .allocator = allocator, .errors = std.ArrayList([]const u8).init(allocator), .lexer = lexer, .cur_token = undefined, .peek_token = undefined };

    p.nextToken();
    p.nextToken();

    return p;
}

pub fn deinit(self: *Parser) void {
    for (self.errors.items) |err| {
        self.allocator.free(err);
    }
    self.errors.deinit();
}

fn nextToken(self: *Parser) void {
    self.cur_token = self.peek_token;
    self.peek_token = self.lexer.nextToken();
}

fn curTokenIs(self: Parser, t: Token.Type) bool {
    return self.cur_token.type == t;
}

fn peekTokenIs(self: Parser, t: Token.Type) bool {
    return self.peek_token.type == t;
}

fn expectPeek(self: *Parser, t: Token.Type) !bool {
    if (!self.peekTokenIs(t)) {
        try self.peekError(t);
        return false;
    }

    self.nextToken();
    return true;
}

fn parseLetStatement(self: *Parser) !?ast.Statement {
    const token = self.cur_token;
    if (!try self.expectPeek(.ident)) {
        return null;
    }

    const let = ast.LetStatement{
        .token = token,
        .name = ast.Identifier{
            .token = self.cur_token,
            .value = self.cur_token.literal,
        },
        .value = undefined,
    };

    if (!try self.expectPeek(.assign)) {
        return null;
    }

    // TODO: we're skipping expressions until we encounter semicolon.
    while (!self.curTokenIs(.semicolon)) {
        self.nextToken();
    }

    return .{ .let = let };
}

fn parseReturnStatement(self: *Parser) ?ast.Statement {
    const token = self.cur_token;
    self.nextToken();

    // TODO: we're skipping expressions until we encounter semicolon.
    while (!self.curTokenIs(.semicolon)) {
        self.nextToken();
    }

    return .{ .@"return" = .{ .token = token, .return_value = undefined } };
}

fn parseStatement(self: *Parser) !?ast.Statement {
    return switch (self.cur_token.type) {
        .let => try self.parseLetStatement(),
        .@"return" => self.parseReturnStatement(),
        else => null,
    };
}

fn peekError(self: *Parser, t: Token.Type) !void {
    const msg = try std.fmt.allocPrint(self.allocator, "expected next token to be {}, got {} instead", .{ t, self.peek_token.type });
    try self.errors.append(msg);
}

pub fn parseProgram(self: *Parser) !ast.Program {
    var p = ast.Program.init(self.allocator);

    while (self.cur_token.type != .eof) {
        if (try self.parseStatement()) |s| {
            try p.statements.append(s);
        }
        self.nextToken();
    }

    return p;
}

fn testLetStatement(s: ast.Statement, expected: []const u8) !void {
    try std.testing.expect(std.mem.eql(u8, s.tokenLiteral(), "let"));

    const let = s.let;
    try std.testing.expect(std.mem.eql(u8, let.name.value, expected));
    try std.testing.expect(std.mem.eql(u8, let.name.tokenLiteral(), expected));
}

fn checkParserErrors(p: Parser) !void {
    if (p.errors.items.len > 0) {
        std.debug.print("found {} parser errors\n", .{p.errors.items.len});
        for (p.errors.items) |err| {
            std.debug.print("parser error: {s}\n", .{err});
        }
    }
    try std.testing.expect(p.errors.items.len == 0);
}

test "let statements" {
    const l = Lexer.init(
        \\let x = 5;
        \\let y = 10;
        \\let foobar = 838383;
    );
    var p = Parser.init(std.testing.allocator, l);
    defer p.deinit();

    var program = try p.parseProgram();
    defer program.deinit();

    try checkParserErrors(p);

    try std.testing.expect(program.statements.items.len == 3);

    const tests = [_][]const u8{ "x", "y", "foobar" };
    for (tests, 0..) |expected, i| {
        const s = program.statements.items[i];
        try testLetStatement(s, expected);
    }
}

test "return statements" {
    const l = Lexer.init(
        \\return 5;
        \\return 10;
        \\return 993322;
    );
    var p = Parser.init(std.testing.allocator, l);
    defer p.deinit();

    var program = try p.parseProgram();
    defer program.deinit();

    try checkParserErrors(p);

    try std.testing.expect(program.statements.items.len == 3);

    for (program.statements.items) |s| {
        try std.testing.expect(std.mem.eql(u8, s.tokenLiteral(), "return"));
    }
}
