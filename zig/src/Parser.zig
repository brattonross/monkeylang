const std = @import("std");
const Allocator = std.mem.Allocator;
const ast = @import("./ast.zig");
const Lexer = @import("./Lexer.zig");
const Token = Lexer.Token;

const Parser = @This();

allocator: Allocator,
lexer: *Lexer,
errors: std.ArrayList([]const u8),
current_token: ?Token,
peek_token: ?Token,

pub fn init(allocator: Allocator, lexer: *Lexer) !Parser {
    var parser = Parser{
        .allocator = allocator,
        .lexer = lexer,
        .current_token = null,
        .peek_token = null,
        .errors = std.ArrayList([]const u8).init(allocator),
    };

    parser.nextToken();
    parser.nextToken();

    return parser;
}

pub fn parseProgram(self: *Parser) !ast.Program {
    var statements = std.ArrayList(ast.Statement).init(self.allocator);
    while (self.current_token) |_| {
        if (try self.parseStatement()) |statement| {
            try statements.append(statement);
        }
        self.nextToken();
    }
    return .{ .statements = statements };
}

fn parseStatement(self: *Parser) !?ast.Statement {
    const token = self.current_token orelse return null;
    return switch (token.type) {
        .let => try self.parseLetStatement(),
        .@"return" => try self.parseReturnStatement(),
        else => try self.parseExpressionStatement(),
    };
}

fn parseLetStatement(self: *Parser) !?ast.Statement {
    const token = self.current_token orelse return null;
    if (!try self.advanceIfPeek(.identifier)) {
        return null;
    }

    const name = ast.Identifier{
        .token = self.current_token orelse return null,
        .value = self.current_token.?.literal,
    };

    if (!try self.advanceIfPeek(.assign)) {
        return null;
    }
    self.nextToken();

    const value = try self.parseExpression(.lowest) orelse return null;
    if (self.peek_token) |peek| if (peek.type == .semicolon) self.nextToken();

    const let = ast.LetStatement{ .token = token, .name = name, .value = value };
    return .{ .let = let };
}

fn parseReturnStatement(self: *Parser) !?ast.Statement {
    const token = self.current_token orelse return null;
    self.nextToken();
    const value = try self.parseExpression(.lowest) orelse return null;

    if (self.peek_token) |peek| if (peek.type == .semicolon) self.nextToken();

    const ret = ast.ReturnStatement{ .token = token, .return_value = value };
    return .{ .@"return" = ret };
}

fn parseExpressionStatement(self: *Parser) !?ast.Statement {
    const token = self.current_token orelse return null;
    const expression = try self.parseExpression(.lowest) orelse return null;
    if (self.peek_token) |peek| if (peek.type == .semicolon) self.nextToken();
    const exp = ast.ExpressionStatement{ .token = token, .expression = expression };
    return .{ .expression = exp };
}

const Precedence = enum(u4) {
    lowest,
    equals,
    less_greater,
    sum,
    product,
    prefix,
    call,
};

fn parseExpression(self: *Parser, precedence: Precedence) anyerror!?ast.Expression {
    const current = self.current_token orelse return null;
    var left = switch (current.type) {
        .identifier => try self.parseIdentifier(),
        .integer => try self.parseIntegerLiteral(),
        .bang, .minus => try self.parsePrefixExpression(),
        .true, .false => try self.parseBoolean(),
        .left_paren => try self.parseGroupedExpression(),
        .@"if" => try self.parseIfExpression(),
        .function => try self.parseFunctionLiteral(),
        else => {
            const msg = try std.fmt.allocPrint(self.allocator, "no prefix parse function for {} found", .{current.type});
            try self.errors.append(msg);
            return null;
        },
    } orelse return null;

    while (self.peek_token) |peek| {
        if (peek.type == .semicolon or @intFromEnum(precedence) >= @intFromEnum(self.peekPrecedence())) {
            break;
        }
        switch (peek.type) {
            .plus, .minus, .slash, .asterisk, .equal, .not_equal, .less_than, .greater_than => {
                self.nextToken();
                left = try self.parseInfixExpression(left) orelse return null;
            },
            .left_paren => {
                self.nextToken();
                left = try self.parseCallExpression(left) orelse return null;
            },
            else => return left,
        }
    }

    return left;
}

fn parseIdentifier(self: *Parser) !?ast.Expression {
    const current = self.current_token orelse return null;
    return .{ .identifier = .{ .token = current, .value = current.literal } };
}

fn parseIntegerLiteral(self: *Parser) !?ast.Expression {
    const token = self.current_token orelse return null;
    const value = std.fmt.parseInt(i64, token.literal, 10) catch {
        const msg = try std.fmt.allocPrint(self.allocator, "could not parse {s} as integer", .{token.literal});
        try self.errors.append(msg);
        return null;
    };
    const integer = ast.IntegerLiteral{ .token = token, .value = value };
    return .{ .integer = integer };
}

fn parsePrefixExpression(self: *Parser) !?ast.Expression {
    const token = self.current_token orelse return null;
    self.nextToken();
    const right = try self.parseExpression(.prefix) orelse return null;
    const prefix = try self.allocator.create(ast.PrefixExpression);
    prefix.* = .{ .token = token, .operator = token.literal, .right = right };
    return .{ .prefix = prefix };
}

fn parseInfixExpression(self: *Parser, left: ast.Expression) !?ast.Expression {
    const token = self.current_token orelse return null;
    const precedence = self.currentPrecedence();
    self.nextToken();
    const right = try self.parseExpression(precedence) orelse return null;
    const infix = try self.allocator.create(ast.InfixExpression);
    infix.* = ast.InfixExpression{ .token = token, .operator = token.literal, .left = left, .right = right };
    return .{ .infix = infix };
}

fn parseBoolean(self: *Parser) !?ast.Expression {
    const token = self.current_token orelse return null;
    const boolean = ast.Boolean{ .token = token, .value = token.type == .true };
    return .{ .boolean = boolean };
}

fn parseGroupedExpression(self: *Parser) !?ast.Expression {
    self.nextToken();
    const expression = try self.parseExpression(.lowest);
    if (!try self.advanceIfPeek(.right_paren)) {
        return null;
    }
    return expression;
}

fn parseIfExpression(self: *Parser) !?ast.Expression {
    const token = self.current_token orelse return null;
    if (!try self.advanceIfPeek(.left_paren)) {
        return null;
    }
    self.nextToken();

    const condition = try self.parseExpression(.lowest) orelse return null;

    if (!try self.advanceIfPeek(.right_paren)) {
        return null;
    }
    if (!try self.advanceIfPeek(.left_brace)) {
        return null;
    }

    const consequence = try self.parseBlockStatement() orelse return null;
    const exp = try self.allocator.create(ast.IfExpression);
    exp.* = .{ .token = token, .condition = condition, .consequence = consequence, .alternative = undefined };

    if (self.peek_token) |peek| if (peek.type == .@"else") {
        self.nextToken();
        if (!try self.advanceIfPeek(.left_brace)) {
            return null;
        }
        exp.alternative = try self.parseBlockStatement() orelse return null;
    };

    return .{ .@"if" = exp };
}

fn parseFunctionLiteral(self: *Parser) !?ast.Expression {
    const token = self.current_token orelse return null;
    if (!try self.advanceIfPeek(.left_paren)) {
        return null;
    }

    const parameters = try self.parseFunctionParameters() orelse return null;
    if (!try self.advanceIfPeek(.left_brace)) {
        return null;
    }

    const body = try self.parseBlockStatement() orelse return null;
    const func = ast.FunctionLiteral{ .token = token, .parameters = parameters, .body = body };
    return .{ .function = func };
}

fn parseFunctionParameters(self: *Parser) !?std.ArrayList(ast.Identifier) {
    var params = std.ArrayList(ast.Identifier).init(self.allocator);

    if (self.peek_token) |peek| if (peek.type == .right_paren) {
        self.nextToken();
        return params;
    };

    self.nextToken();

    const token = self.current_token.?;
    try params.append(.{ .token = token, .value = token.literal });

    while (self.peek_token) |peek| {
        if (peek.type != .comma) break;
        self.nextToken();
        self.nextToken();
        const current = self.current_token.?;
        try params.append(.{ .token = current, .value = current.literal });
    }

    if (!try self.advanceIfPeek(.right_paren)) {
        return null;
    }

    return params;
}

fn parseCallExpression(self: *Parser, function: ast.Expression) !?ast.Expression {
    const token = self.current_token orelse return null;
    const args = try self.parseCallArguments() orelse return null;
    const call = try self.allocator.create(ast.CallExpression);
    call.* = .{ .token = token, .function = function, .arguments = args };
    return .{ .call = call };
}

fn parseCallArguments(self: *Parser) !?std.ArrayList(ast.Expression) {
    var args = std.ArrayList(ast.Expression).init(self.allocator);

    if (self.peek_token) |peek| if (peek.type == .right_paren) {
        self.nextToken();
        return args;
    };

    self.nextToken();
    const first = try self.parseExpression(.lowest) orelse return null;
    try args.append(first);

    while (self.peek_token) |peek| {
        if (peek.type != .comma) break;
        self.nextToken();
        self.nextToken();
        const arg = try self.parseExpression(.lowest) orelse return null;
        try args.append(arg);
    }

    if (!try self.advanceIfPeek(.right_paren)) {
        return null;
    }

    return args;
}

fn parseBlockStatement(self: *Parser) !?ast.BlockStatement {
    const token = self.current_token orelse return null;
    var statements = std.ArrayList(ast.Statement).init(self.allocator);

    self.nextToken();

    while (self.current_token) |current| {
        if (current.type == .right_brace) break;
        if (try self.parseStatement()) |statement| {
            try statements.append(statement);
        }
        self.nextToken();
    }

    return .{ .token = token, .statements = statements };
}

fn advanceIfPeek(self: *Parser, t: Token.Type) !bool {
    const peek = self.peek_token orelse return false;
    if (peek.type == t) {
        self.nextToken();
        return true;
    }
    const err = try std.fmt.allocPrint(self.allocator, "expected next token to be {}, got {} instead, pos: {}", .{ t, peek.type, self.lexer.pos });
    try self.errors.append(err);
    return false;
}

fn nextToken(self: *Parser) void {
    self.current_token = self.peek_token;
    self.peek_token = self.lexer.nextToken();
}

fn currentPrecedence(self: Parser) Precedence {
    const current = self.current_token orelse return .lowest;
    return tokenPrecedence(current.type);
}

fn peekPrecedence(self: Parser) Precedence {
    const peek = self.peek_token orelse return .lowest;
    return tokenPrecedence(peek.type);
}

fn tokenPrecedence(t: Token.Type) Precedence {
    return switch (t) {
        .equal, .not_equal => .equals,
        .less_than, .greater_than => .less_greater,
        .plus, .minus => .sum,
        .slash, .asterisk => .product,
        .left_paren => .call,
        else => .lowest,
    };
}
