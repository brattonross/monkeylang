const std = @import("std");
const Token = @import("Token.zig");

pub const Expression = struct {};

pub const Identifier = struct {
    token: Token,
    value: []const u8,

    pub fn tokenLiteral(self: Identifier) []const u8 {
        return self.token.literal;
    }
};

pub const StatementType = enum {
    let,
};

pub const LetStatement = struct {
    token: Token,
    name: Identifier,
    value: Expression,
};

pub const Statement = union(StatementType) {
    let: LetStatement,

    pub fn tokenLiteral(self: Statement) []const u8 {
        return switch (self) {
            .let => |s| s.token.literal,
        };
    }
};

pub const Program = struct {
    statements: std.ArrayList(Statement),

    pub fn init(allocator: std.mem.Allocator) Program {
        return .{
            .statements = std.ArrayList(Statement).init(allocator),
        };
    }

    pub fn deinit(self: *Program) void {
        self.statements.deinit();
    }

    pub fn tokenLiteral(self: Program) []const u8 {
        return if (self.statements.len > 0) self.statements[0].tokenLiteral() else "";
    }
};
