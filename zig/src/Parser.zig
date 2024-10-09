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

const PrefixParseFn = fn (*Parser) std.mem.Allocator.Error!?ast.Expression;
const InfixParseFn = fn (*Parser, ?ast.Expression) std.mem.Allocator.Error!?ast.Expression;

const Precedence = enum {
    lowest,
    equals,
    lessgreater,
    sum,
    product,
    prefix,
    call,
};

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

fn getTokenTypePrecedence(t: Token.Type) ?Precedence {
    return switch (t) {
        .eq, .ne => .equals,
        .lt, .gt => .lessgreater,
        .plus, .minus => .sum,
        .slash, .asterisk => .product,
        else => null,
    };
}

fn peekPrecedence(self: Parser) Precedence {
    return getTokenTypePrecedence(self.peek_token.type) orelse .lowest;
}

fn curPrecedence(self: Parser) Precedence {
    return getTokenTypePrecedence(self.cur_token.type) orelse .lowest;
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

fn parseIdentifier(self: *Parser) !?ast.Expression {
    return .{
        .ident = .{
            .token = self.cur_token,
            .value = self.cur_token.literal,
        },
    };
}

fn parseIntegerLiteral(self: *Parser) !?ast.Expression {
    const result = std.fmt.parseInt(i64, self.cur_token.literal, 10) catch {
        const msg = try std.fmt.allocPrint(self.allocator, "could not parse {s} as integer", .{self.cur_token.literal});
        try self.errors.append(msg);
        return null;
    };
    return .{ .integer_literal = .{
        .token = self.cur_token,
        .value = result,
    } };
}

fn parsePrefixExpression(self: *Parser) !?ast.Expression {
    var prefix = try self.allocator.create(ast.PrefixExpression);
    prefix.token = self.cur_token;
    prefix.operator = self.cur_token.literal;
    self.nextToken();
    prefix.right = try self.parseExpression(.prefix);
    return .{ .prefix = prefix };
}

fn getPrefixParseFn(t: Token.Type) ?*const PrefixParseFn {
    return switch (t) {
        .ident => parseIdentifier,
        .int => parseIntegerLiteral,
        .bang, .minus => parsePrefixExpression,
        else => null,
    };
}

fn noPrefixParseFnError(self: *Parser, t: Token.Type) !void {
    const msg = try std.fmt.allocPrint(self.allocator, "no prefix parse function found for {}", .{t});
    try self.errors.append(msg);
}

fn parseInfixExpression(self: *Parser, left: ?ast.Expression) !?ast.Expression {
    var infix = try self.allocator.create(ast.InfixExpression);
    infix.token = self.cur_token;
    infix.operator = self.cur_token.literal;
    infix.left = left;

    const precedence = self.curPrecedence();
    self.nextToken();
    infix.right = try self.parseExpression(precedence);

    return .{ .infix = infix };
}

fn getInfixParseFn(t: Token.Type) ?*const InfixParseFn {
    return switch (t) {
        .plus, .minus, .slash, .asterisk, .eq, .ne, .lt, .gt => parseInfixExpression,
        else => null,
    };
}

fn parseExpression(self: *Parser, precedence: Precedence) !?ast.Expression {
    const prefix = getPrefixParseFn(self.cur_token.type) orelse {
        try self.noPrefixParseFnError(self.cur_token.type);
        return null;
    };

    var left = try prefix(self);

    while (!self.peekTokenIs(.semicolon) and @intFromEnum(precedence) < @intFromEnum(self.peekPrecedence())) {
        const infix = getInfixParseFn(self.peek_token.type) orelse {
            return left;
        };
        self.nextToken();
        left = try infix(self, left);
    }

    return left;
}

fn parseExpressionStatement(self: *Parser) !?ast.Statement {
    const s = ast.ExpressionStatement{
        .token = self.cur_token,
        .expression = try self.parseExpression(.lowest),
    };

    if (self.peekTokenIs(.semicolon)) {
        self.nextToken();
    }

    return .{ .expression = s };
}

fn parseStatement(self: *Parser) !?ast.Statement {
    return switch (self.cur_token.type) {
        .let => try self.parseLetStatement(),
        .@"return" => self.parseReturnStatement(),
        else => try self.parseExpressionStatement(),
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

// ---

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

test "identifier expression" {
    const l = Lexer.init("foobar;");
    var p = Parser.init(std.testing.allocator, l);
    defer p.deinit();

    var program = try p.parseProgram();
    defer program.deinit();

    try checkParserErrors(p);

    try std.testing.expect(program.statements.items.len == 1);

    try std.testing.expect(switch (program.statements.items[0]) {
        .expression => true,
        else => false,
    });
    const exp = program.statements.items[0].expression;

    try std.testing.expect(switch (exp.expression.?) {
        .ident => true,
        else => false,
    });
    const ident = exp.expression.?.ident;

    try std.testing.expectEqualStrings("foobar", ident.value);
    try std.testing.expectEqualStrings("foobar", ident.token.literal);
}

test "integer literal expressions" {
    const l = Lexer.init("5;");
    var p = Parser.init(std.testing.allocator, l);
    defer p.deinit();

    var program = try p.parseProgram();
    defer program.deinit();

    try checkParserErrors(p);

    try std.testing.expect(program.statements.items.len == 1);

    try std.testing.expect(switch (program.statements.items[0]) {
        .expression => true,
        else => false,
    });
    const exp = program.statements.items[0].expression;

    try std.testing.expect(switch (exp.expression.?) {
        .integer_literal => true,
        else => false,
    });
    const literal = exp.expression.?.integer_literal;

    try std.testing.expectEqual(5, literal.value);
    try std.testing.expectEqualStrings("5", literal.tokenLiteral());
}

fn testIntegerLiteral(il: ast.Expression, value: i64) !void {
    try std.testing.expect(switch (il) {
        .integer_literal => true,
        else => false,
    });
    const integer_literal = il.integer_literal;

    try std.testing.expectEqual(value, integer_literal.value);
    const expected = try std.fmt.allocPrint(std.testing.allocator, "{}", .{value});
    defer std.testing.allocator.free(expected);
    try std.testing.expectEqualStrings(expected, integer_literal.tokenLiteral());
}

test "prefix expressions" {
    const TestCase = struct {
        input: []const u8,
        operator: []const u8,
        integer_value: i64,
    };
    const test_cases = [_]TestCase{
        .{ .input = "!5;", .operator = "!", .integer_value = 5 },
    };

    for (test_cases) |test_case| {
        const l = Lexer.init(test_case.input);
        var p = Parser.init(std.testing.allocator, l);
        defer p.deinit();

        var program = try p.parseProgram();
        defer program.deinit();

        try checkParserErrors(p);

        try std.testing.expect(program.statements.items.len == 1);

        try std.testing.expect(switch (program.statements.items[0]) {
            .expression => true,
            else => false,
        });
        const statement = program.statements.items[0].expression;

        try std.testing.expect(switch (statement.expression.?) {
            .prefix => true,
            else => false,
        });
        const exp = statement.expression.?.prefix;

        try std.testing.expectEqualStrings(test_case.operator, exp.operator);

        try std.testing.expect(switch (exp.right.?) {
            .integer_literal => true,
            else => false,
        });
        try testIntegerLiteral(exp.right.?, test_case.integer_value);
    }
}

test "infix expressions" {
    const TestCase = struct {
        input: []const u8,
        left_value: i64,
        operator: []const u8,
        right_value: i64,
    };
    const test_cases = [_]TestCase{
        .{ .input = "5 + 5;", .left_value = 5, .operator = "+", .right_value = 5 },
        .{ .input = "5 - 5;", .left_value = 5, .operator = "-", .right_value = 5 },
        .{ .input = "5 * 5;", .left_value = 5, .operator = "*", .right_value = 5 },
        .{ .input = "5 / 5;", .left_value = 5, .operator = "/", .right_value = 5 },
        .{ .input = "5 > 5;", .left_value = 5, .operator = ">", .right_value = 5 },
        .{ .input = "5 < 5;", .left_value = 5, .operator = "<", .right_value = 5 },
        .{ .input = "5 == 5;", .left_value = 5, .operator = "==", .right_value = 5 },
        .{ .input = "5 != 5;", .left_value = 5, .operator = "!=", .right_value = 5 },
    };

    for (test_cases) |test_case| {
        const l = Lexer.init(test_case.input);
        var p = Parser.init(std.testing.allocator, l);
        defer p.deinit();

        var program = try p.parseProgram();
        defer program.deinit();

        try checkParserErrors(p);

        try std.testing.expect(program.statements.items.len == 1);

        const statement = program.statements.items[0].expression;
        const infix = statement.expression.?.infix;

        try testIntegerLiteral(infix.left.?, test_case.left_value);
        try std.testing.expectEqualStrings(test_case.operator, infix.operator);
        try testIntegerLiteral(infix.right.?, test_case.right_value);
    }
}

test "operator precedence" {
    const TestCase = struct {
        input: []const u8,
        expected: []const u8,
    };
    const test_cases = [_]TestCase{
        .{
            .input = "-a * b",
            .expected = "((-a) * b)",
        },
        .{
            .input = "!-a",
            .expected = "(!(-a))",
        },
        .{
            .input = "a + b + c",
            .expected = "((a + b) + c)",
        },
        .{
            .input = "a + b - c",
            .expected = "((a + b) - c)",
        },
        .{
            .input = "a * b * c",
            .expected = "((a * b) * c)",
        },
        .{
            .input = "a * b / c",
            .expected = "((a * b) / c)",
        },
        .{
            .input = "a + b / c",
            .expected = "(a + (b / c))",
        },
        .{
            .input = "a + b * c + d / e - f",
            .expected = "(((a + (b * c)) + (d / e)) - f)",
        },
        .{
            .input = "3 + 4; -5 * 5",
            .expected = "(3 + 4)((-5) * 5)",
        },
        .{
            .input = "5 > 4 == 3 < 4",
            .expected = "((5 > 4) == (3 < 4))",
        },
        .{
            .input = "5 < 4 != 3 > 4",
            .expected = "((5 < 4) != (3 > 4))",
        },
        .{
            .input = "3 + 4 * 5 == 3 * 1 + 4 * 5",
            .expected = "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))",
        },
    };

    for (test_cases) |test_case| {
        const l = Lexer.init(test_case.input);
        var p = Parser.init(std.testing.allocator, l);
        defer p.deinit();

        var program = try p.parseProgram();
        defer program.deinit();

        try checkParserErrors(p);

        const actual = try std.fmt.allocPrint(std.testing.allocator, "{}", .{program});
        defer std.testing.allocator.free(actual);
        try std.testing.expectEqualStrings(test_case.expected, actual);
    }
}
