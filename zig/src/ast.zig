const std = @import("std");
const Token = @import("Token.zig");

pub const ExpressionType = enum {
    ident,
    integer_literal,
    prefix,
    infix,
};

pub const Expression = union(ExpressionType) {
    ident: Identifier,
    integer_literal: IntegerLiteral,
    prefix: *PrefixExpression,
    infix: *InfixExpression,

    pub fn deinit(self: *const Expression, allocator: std.mem.Allocator) void {
        switch (self.*) {
            .prefix => self.prefix.deinit(allocator),
            .infix => self.infix.deinit(allocator),
            else => {},
        }
    }

    pub fn format(self: Expression, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        switch (self) {
            .ident => {
                try writer.print("{}", .{self.ident});
            },
            .integer_literal => {
                try writer.print("{}", .{self.integer_literal});
            },
            .prefix => {
                try writer.print("{}", .{self.prefix});
            },
            .infix => {
                try writer.print("{}", .{self.infix});
            },
        }
    }
};

pub const Identifier = struct {
    token: Token,
    value: []const u8,

    pub fn tokenLiteral(self: Identifier) []const u8 {
        return self.token.literal;
    }

    pub fn format(self: Identifier, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{s}", .{self.value});
    }
};

pub const IntegerLiteral = struct {
    token: Token,
    value: i64,

    pub fn tokenLiteral(self: IntegerLiteral) []const u8 {
        return self.token.literal;
    }

    pub fn format(self: IntegerLiteral, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{}", .{self.value});
    }
};

pub const PrefixExpression = struct {
    token: Token,
    operator: []const u8,
    right: ?Expression,

    pub fn deinit(self: *const PrefixExpression, allocator: std.mem.Allocator) void {
        if (self.right) |right| {
            right.deinit(allocator);
        }
        allocator.destroy(self);
    }

    pub fn tokenLiteral(self: PrefixExpression) []const u8 {
        return self.token.literal;
    }

    pub fn format(self: PrefixExpression, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("({s}{?})", .{ self.operator, self.right });
    }
};

pub const InfixExpression = struct {
    token: Token,
    left: ?Expression,
    operator: []const u8,
    right: ?Expression,

    pub fn deinit(self: *const InfixExpression, allocator: std.mem.Allocator) void {
        if (self.left) |left| {
            left.deinit(allocator);
        }
        if (self.right) |right| {
            right.deinit(allocator);
        }
        allocator.destroy(self);
    }

    pub fn tokenLiteral(self: InfixExpression) []const u8 {
        return self.token.literal;
    }

    pub fn format(self: InfixExpression, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("({?} {s} {?})", .{ self.left, self.operator, self.right });
    }
};

pub const StatementType = enum {
    let,
    @"return",
    expression,
};

pub const LetStatement = struct {
    token: Token,
    name: Identifier,
    value: Expression,

    pub fn format(self: LetStatement, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{s} {} = {};", .{ self.token.literal, self.name, self.value });
    }
};

pub const ReturnStatement = struct {
    token: Token,
    return_value: Expression,

    pub fn format(self: ReturnStatement, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{s} {};", .{ self.token.literal, self.return_value });
    }
};

pub const ExpressionStatement = struct {
    token: Token,
    expression: ?Expression,

    pub fn deinit(self: *const ExpressionStatement, allocator: std.mem.Allocator) void {
        if (self.*.expression) |exp| {
            switch (exp) {
                .prefix => exp.prefix.deinit(allocator),
                .infix => exp.infix.deinit(allocator),
                else => {},
            }
        }
    }

    pub fn format(self: ExpressionStatement, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.print("{?}", .{self.expression});
    }
};

pub const Statement = union(StatementType) {
    let: LetStatement,
    @"return": ReturnStatement,
    expression: ExpressionStatement,

    pub fn deinit(self: *const Statement, allocator: std.mem.Allocator) void {
        switch (self.*) {
            .expression => self.expression.deinit(allocator),
            else => {},
        }
    }

    pub fn tokenLiteral(self: Statement) []const u8 {
        return switch (self) {
            .let => |s| s.token.literal,
            .@"return" => |s| s.token.literal,
            .expression => |s| s.token.literal,
        };
    }

    pub fn format(self: Statement, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        return try switch (self) {
            .let => |s| s.format(fmt, options, writer),
            .@"return" => |s| s.format(fmt, options, writer),
            .expression => |s| s.format(fmt, options, writer),
        };
    }
};

pub const Program = struct {
    allocator: std.mem.Allocator,
    statements: std.ArrayList(Statement),

    pub fn init(allocator: std.mem.Allocator) Program {
        return .{
            .allocator = allocator,
            .statements = std.ArrayList(Statement).init(allocator),
        };
    }

    pub fn deinit(self: *Program) void {
        for (self.statements.items) |s| {
            s.deinit(self.allocator);
        }
        self.statements.deinit();
    }

    pub fn tokenLiteral(self: Program) []const u8 {
        return if (self.statements.len > 0) self.statements[0].tokenLiteral() else "";
    }

    pub fn format(self: Program, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        for (self.statements.items) |s| {
            try writer.print("{}", .{s});
        }
    }
};
