pub const Program = struct {
    statements: std.ArrayList(Statement),

    pub fn format(self: Program, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        for (self.statements.items) |statement| {
            try writer.print("{}", .{statement});
        }
    }
};

pub const Statement = union(enum) {
    let: LetStatement,
    @"return": ReturnStatement,
    expression: ExpressionStatement,

    pub fn format(self: Statement, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        switch (self) {
            .let => try writer.print("{}", .{self.let}),
            .@"return" => try writer.print("{}", .{self.@"return"}),
            .expression => try writer.print("{}", .{self.expression}),
        }
    }
};

pub const LetStatement = struct {
    token: Token,
    name: Identifier,
    value: *Expression,

    pub fn format(self: LetStatement, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.print("{} {} = {};", .{ self.token, self.name, self.value });
    }
};

pub const ReturnStatement = struct {
    token: Token,
    return_value: *Expression,

    pub fn format(self: ReturnStatement, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.print("{} {};", .{ self.token, self.return_value });
    }
};

pub const ExpressionStatement = struct {
    token: Token,
    expression: *Expression,

    pub fn format(self: ExpressionStatement, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.print("{}", .{self.expression});
    }
};

pub const Identifier = struct {
    token: Token,
    value: []const u8,

    pub fn format(self: Identifier, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.print("{s}", .{self.value});
    }
};

pub const Expression = union(enum) {
    identifier: Identifier,
    integer: IntegerLiteral,
    prefix: PrefixExpression,
    infix: InfixExpression,
    boolean: Boolean,
    @"if": IfExpression,
    function: FunctionLiteral,
    call: CallExpression,

    pub fn format(self: Expression, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        switch (self) {
            .identifier => try writer.print("{}", .{self.identifier}),
            .integer => try writer.print("{}", .{self.integer}),
            .prefix => try writer.print("{}", .{self.prefix}),
            .infix => try writer.print("{}", .{self.infix}),
            .boolean => try writer.print("{}", .{self.boolean}),
            .@"if" => try writer.print("{}", .{self.@"if"}),
            .function => try writer.print("{}", .{self.function}),
            .call => try writer.print("{}", .{self.call}),
        }
    }
};

pub const IntegerLiteral = struct {
    token: Token,
    value: i64,

    pub fn format(self: IntegerLiteral, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.print("{d}", .{self.value});
    }
};

pub const PrefixExpression = struct {
    token: Token,
    operator: []const u8,
    right: *Expression,

    pub fn format(self: PrefixExpression, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.print("{s}{}", .{ self.operator, self.right.* });
    }
};

pub const InfixExpression = struct {
    token: Token,
    left: *Expression,
    operator: []const u8,
    right: *Expression,

    pub fn format(self: InfixExpression, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.print("{} {s} {}", .{ self.left.*, self.operator, self.right.* });
    }
};

pub const Boolean = struct {
    token: Token,
    value: bool,

    pub fn format(self: Boolean, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.print("{}", .{self.value});
    }
};

pub const IfExpression = struct {
    token: Token,
    condition: *Expression,
    consequence: BlockStatement,
    alternative: ?BlockStatement,

    pub fn format(self: IfExpression, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.print("if ({}) {{\n", .{self.condition.*});
        try writer.print("\t{}\n", .{self.consequence});
        try writer.writeAll("}");

        if (self.alternative) |alt| {
            try writer.writeAll(" else {\n");
            try writer.print("\t{}\n", .{alt});
            try writer.writeAll("}");
        }
    }
};

pub const BlockStatement = struct {
    token: Token,
    statements: std.ArrayList(Statement),

    pub fn format(self: BlockStatement, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        for (self.statements.items) |statement| {
            try writer.print("{}", .{statement});
        }
    }
};

pub const FunctionLiteral = struct {
    token: Token,
    parameters: std.ArrayList(Identifier),
    body: BlockStatement,

    pub fn format(self: FunctionLiteral, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.print("{s}(", .{self.token.literal});
        for (0..self.parameters.items.len) |i| {
            try writer.print("{}", .{self.parameters.items[i]});
            if (i != self.parameters.items.len - 1) {
                try writer.writeAll(", ");
            }
        }
        try writer.print(") {{\n{}\n}}", .{self.body});
    }
};

pub const CallExpression = struct {
    token: Token,
    function: *Expression,
    arguments: std.ArrayList(*Expression),

    pub fn format(self: CallExpression, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;

        try writer.print("{}(", .{self.function.*});
        for (0..self.arguments.items.len) |i| {
            try writer.print("{}", .{self.arguments.items[i].*});
            if (i != self.arguments.items.len - 1) {
                try writer.writeAll(", ");
            }
        }
        try writer.writeAll(")");
    }
};

const std = @import("std");
const Token = @import("./Lexer.zig").Token;
