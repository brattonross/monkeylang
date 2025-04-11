allocator: Allocator,
lexer: *Lexer,
errors: std.ArrayList([]const u8),
current_token: Token,
peek_token: Token,

pub fn init(allocator: Allocator, lexer: *Lexer) !Parser {
    var parser = Parser{
        .allocator = allocator,
        .lexer = lexer,
        .current_token = undefined,
        .peek_token = undefined,
        .errors = std.ArrayList([]const u8).init(allocator),
    };

    parser.nextToken();
    parser.nextToken();

    return parser;
}

pub fn parseProgram(self: *Parser) !ast.Program {
    var statements = std.ArrayList(ast.Statement).init(self.allocator);
    while (self.current_token.type != .eof) {
        if (try self.parseStatement()) |statement| {
            try statements.append(statement);
        }
        self.nextToken();
    }
    return .{ .statements = statements };
}

fn parseStatement(self: *Parser) !?ast.Statement {
    return switch (self.current_token.type) {
        .let => try self.parseLetStatement(),
        .@"return" => try self.parseReturnStatement(),
        else => try self.parseExpressionStatement(),
    };
}

fn parseLetStatement(self: *Parser) !?ast.Statement {
    const token = self.current_token;

    if (!try self.advanceIfPeek(.identifier)) {
        return null;
    }

    const name = ast.Identifier{
        .token = self.current_token,
        .value = try self.allocator.dupe(u8, self.current_token.literal),
    };

    if (!try self.advanceIfPeek(.assign)) {
        return null;
    }
    self.nextToken();

    const value = try self.parseExpression(.lowest) orelse return null;

    if (self.peek_token.type == .semicolon) self.nextToken();

    const let = ast.LetStatement{ .token = token, .name = name, .value = value };
    return .{ .let = let };
}

fn parseReturnStatement(self: *Parser) !?ast.Statement {
    const token = self.current_token;
    self.nextToken();

    const value = try self.parseExpression(.lowest) orelse return null;

    if (self.peek_token.type == .semicolon) self.nextToken();

    const ret = ast.ReturnStatement{ .token = token, .return_value = value };
    return .{ .@"return" = ret };
}

fn parseExpressionStatement(self: *Parser) !?ast.Statement {
    const token = self.current_token;
    const expression = try self.parseExpression(.lowest) orelse return null;

    if (self.peek_token.type == .semicolon) self.nextToken();

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
    index,
};

fn parseExpression(self: *Parser, precedence: Precedence) anyerror!?*ast.Expression {
    var left = switch (self.current_token.type) {
        .identifier => try self.parseIdentifier(),
        .integer => try self.parseIntegerLiteral(),
        .bang, .minus => try self.parsePrefixExpression(),
        .true, .false => try self.parseBoolean(),
        .left_paren => try self.parseGroupedExpression(),
        .@"if" => try self.parseIfExpression(),
        .function => try self.parseFunctionLiteral(),
        .string => try self.parseStringLiteral(),
        .left_bracket => try self.parseArrayLiteral(),
        .left_brace => try self.parseHashLiteral(),
        else => {
            const msg = try std.fmt.allocPrint(self.allocator, "no prefix parse function for {} found", .{self.current_token.type});
            try self.errors.append(msg);
            return null;
        },
    } orelse return null;

    while (self.peek_token.type != .semicolon and @intFromEnum(precedence) < @intFromEnum(self.peekPrecedence())) {
        switch (self.peek_token.type) {
            .plus, .minus, .slash, .asterisk, .equal, .not_equal, .less_than, .greater_than => {
                self.nextToken();
                left = try self.parseInfixExpression(left) orelse return null;
            },
            .left_paren => {
                self.nextToken();
                left = try self.parseCallExpression(left) orelse return null;
            },
            .left_bracket => {
                self.nextToken();
                left = try self.parseIndexExpression(left) orelse return null;
            },
            else => return left,
        }
    }

    return left;
}

fn parseIdentifier(self: *Parser) !?*ast.Expression {
    const ret = try self.allocator.create(ast.Expression);
    ret.* = .{
        .identifier = .{
            .token = self.current_token,
            .value = try self.allocator.dupe(u8, self.current_token.literal),
        },
    };
    return ret;
}

fn parseIntegerLiteral(self: *Parser) !?*ast.Expression {
    const token = self.current_token;
    const value = std.fmt.parseInt(i64, token.literal, 10) catch {
        const msg = try std.fmt.allocPrint(self.allocator, "could not parse {s} as integer", .{token.literal});
        try self.errors.append(msg);
        return null;
    };
    const integer = ast.IntegerLiteral{ .token = token, .value = value };
    const ret = try self.allocator.create(ast.Expression);
    ret.* = .{ .integer = integer };
    return ret;
}

fn parsePrefixExpression(self: *Parser) !?*ast.Expression {
    const token = self.current_token;
    self.nextToken();

    const right = try self.parseExpression(.prefix) orelse return null;

    const operator = try self.allocator.dupe(u8, token.literal);
    errdefer self.allocator.free(operator);

    const prefix = ast.PrefixExpression{
        .token = token,
        .operator = operator,
        .right = right,
    };

    const ret = try self.allocator.create(ast.Expression);
    ret.* = .{ .prefix = prefix };
    return ret;
}

fn parseInfixExpression(self: *Parser, left: *ast.Expression) !?*ast.Expression {
    const token = self.current_token;
    const precedence = self.currentPrecedence();
    self.nextToken();

    const right = try self.parseExpression(precedence) orelse return null;

    const operator = try self.allocator.dupe(u8, token.literal);
    errdefer self.allocator.free(operator);

    const infix = ast.InfixExpression{ .token = token, .operator = operator, .left = left, .right = right };

    const ret = try self.allocator.create(ast.Expression);
    ret.* = .{ .infix = infix };
    return ret;
}

fn parseBoolean(self: *Parser) !?*ast.Expression {
    const token = self.current_token;
    const boolean = ast.Boolean{ .token = token, .value = token.type == .true };
    const ret = try self.allocator.create(ast.Expression);
    ret.* = .{ .boolean = boolean };
    return ret;
}

fn parseGroupedExpression(self: *Parser) !?*ast.Expression {
    self.nextToken();
    const expression = try self.parseExpression(.lowest);
    if (!try self.advanceIfPeek(.right_paren)) {
        return null;
    }
    return expression;
}

fn parseIfExpression(self: *Parser) !?*ast.Expression {
    const token = self.current_token;
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
    var if_expression = ast.IfExpression{
        .token = token,
        .condition = condition,
        .consequence = consequence,
        .alternative = undefined,
    };

    if (self.peek_token.type == .@"else") {
        self.nextToken();
        if (!try self.advanceIfPeek(.left_brace)) {
            return null;
        }
        if_expression.alternative = try self.parseBlockStatement() orelse return null;
    }

    const ret = try self.allocator.create(ast.Expression);
    ret.* = .{ .@"if" = if_expression };
    return ret;
}

fn parseFunctionLiteral(self: *Parser) !?*ast.Expression {
    const token = self.current_token;
    if (!try self.advanceIfPeek(.left_paren)) {
        return null;
    }

    const parameters = try self.parseFunctionParameters() orelse return null;
    if (!try self.advanceIfPeek(.left_brace)) {
        return null;
    }

    const body = try self.parseBlockStatement() orelse return null;
    const func = ast.FunctionLiteral{ .token = token, .parameters = parameters, .body = body };
    const ret = try self.allocator.create(ast.Expression);
    ret.* = .{ .function = func };
    return ret;
}

fn parseFunctionParameters(self: *Parser) !?std.ArrayList(ast.Identifier) {
    var params = std.ArrayList(ast.Identifier).init(self.allocator);

    if (self.peek_token.type == .right_paren) {
        self.nextToken();
        return params;
    }

    self.nextToken();

    const token = self.current_token;
    try params.append(.{
        .token = token,
        .value = try self.allocator.dupe(u8, token.literal),
    });

    while (self.peek_token.type == .comma) {
        self.nextToken();
        self.nextToken();
        const current = self.current_token;
        try params.append(.{
            .token = current,
            .value = try self.allocator.dupe(u8, current.literal),
        });
    }

    if (!try self.advanceIfPeek(.right_paren)) {
        return null;
    }

    return params;
}

fn parseExpressionList(self: *Parser, end: Token.Type) !std.ArrayList(*ast.Expression) {
    var expressions = std.ArrayList(*ast.Expression).init(self.allocator);

    if (self.peek_token.type == end) {
        self.nextToken();
        return expressions;
    }

    self.nextToken();
    const expression = try self.parseExpression(.lowest) orelse return error.NullExpression;
    try expressions.append(expression);

    while (self.peek_token.type == .comma) {
        self.nextToken();
        self.nextToken();
        const exp = try self.parseExpression(.lowest) orelse return error.NullExpression;
        try expressions.append(exp);
    }

    if (self.peek_token.type != end) {
        return error.UnterminatedExpressionList;
    }
    self.nextToken();

    return expressions;
}

fn parseStringLiteral(self: *Parser) !?*ast.Expression {
    const ret = try self.allocator.create(ast.Expression);
    ret.* = .{
        .string = .{
            .token = self.current_token,
            .value = try self.allocator.dupe(u8, self.current_token.literal),
        },
    };
    return ret;
}

fn parseArrayLiteral(self: *Parser) !?*ast.Expression {
    const token = self.current_token;
    const elements = try self.parseExpressionList(.right_bracket);
    const ret = try self.allocator.create(ast.Expression);
    ret.* = .{ .array = .{ .token = token, .elements = elements } };
    return ret;
}

fn parseHashLiteral(self: *Parser) !?*ast.Expression {
    const token = self.current_token;
    var pairs = std.AutoHashMap(*ast.Expression, *ast.Expression).init(self.allocator);

    while (self.peek_token.type != .right_brace) {
        self.nextToken();
        const key = try self.parseExpression(.lowest) orelse return error.NullExpression;

        if (self.peek_token.type != .colon) {
            return error.InvalidHash;
        }
        self.nextToken();

        self.nextToken();
        const value = try self.parseExpression(.lowest) orelse return error.NullExpression;

        try pairs.putNoClobber(key, value);

        if (self.peek_token.type != .right_brace and self.peek_token.type != .comma) {
            return error.InvalidHash;
        }
        if (self.peek_token.type == .comma) {
            self.nextToken();
        }
    }

    if (self.peek_token.type != .right_brace) {
        return error.InvalidHash;
    }
    self.nextToken();

    const ret = try self.allocator.create(ast.Expression);
    ret.* = .{ .hash = .{ .token = token, .pairs = pairs } };
    return ret;
}

fn parseCallExpression(self: *Parser, function: *ast.Expression) !?*ast.Expression {
    const token = self.current_token;
    const args = try self.parseCallArguments() orelse return null;
    const call = ast.CallExpression{
        .token = token,
        .function = function,
        .arguments = args,
    };
    const ret = try self.allocator.create(ast.Expression);
    ret.* = .{ .call = call };
    return ret;
}

fn parseCallArguments(self: *Parser) !?std.ArrayList(*ast.Expression) {
    var args = std.ArrayList(*ast.Expression).init(self.allocator);

    if (self.peek_token.type == .right_paren) {
        self.nextToken();
        return args;
    }

    self.nextToken();
    const first = try self.parseExpression(.lowest) orelse return null;
    try args.append(first);

    while (self.peek_token.type == .comma) {
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

fn parseIndexExpression(self: *Parser, left: *ast.Expression) !?*ast.Expression {
    const token = self.current_token;
    self.nextToken();

    const index = try self.parseExpression(.lowest) orelse return error.NullExpression;

    if (self.peek_token.type != .right_bracket) {
        return error.UnterminatedIndexExpression;
    }
    self.nextToken();

    const ret = try self.allocator.create(ast.Expression);
    ret.* = .{ .index = .{ .token = token, .left = left, .index = index } };
    return ret;
}

fn parseBlockStatement(self: *Parser) !?ast.BlockStatement {
    const token = self.current_token;
    var statements = std.ArrayList(ast.Statement).init(self.allocator);

    self.nextToken();

    while (self.current_token.type != .eof) {
        if (self.current_token.type == .right_brace) break;
        if (try self.parseStatement()) |statement| {
            try statements.append(statement);
        }
        self.nextToken();
    }

    return .{ .token = token, .statements = statements };
}

fn advanceIfPeek(self: *Parser, t: Token.Type) !bool {
    if (self.peek_token.type == t) {
        self.nextToken();
        return true;
    }
    const err = try std.fmt.allocPrint(self.allocator, "expected next token to be {}, got {} instead, pos: {}", .{ t, self.peek_token.type, self.lexer.pos });
    try self.errors.append(err);
    return false;
}

fn nextToken(self: *Parser) void {
    self.current_token = self.peek_token;
    self.peek_token = self.lexer.nextToken();
}

fn currentPrecedence(self: Parser) Precedence {
    return tokenPrecedence(self.current_token.type);
}

fn peekPrecedence(self: Parser) Precedence {
    return tokenPrecedence(self.peek_token.type);
}

fn tokenPrecedence(t: Token.Type) Precedence {
    return switch (t) {
        .equal, .not_equal => .equals,
        .less_than, .greater_than => .less_greater,
        .plus, .minus => .sum,
        .slash, .asterisk => .product,
        .left_paren => .call,
        .left_bracket => .index,
        else => .lowest,
    };
}

const Parser = @This();

const std = @import("std");
const Allocator = std.mem.Allocator;
const ast = @import("./ast.zig");
const Lexer = @import("./Lexer.zig");
const Token = Lexer.Token;
