const std = @import("std");
const Allocator = std.mem.Allocator;
const ast = @import("./ast.zig");

const Evaluator = @This();

allocator: Allocator,

pub fn evalProgram(self: *Evaluator, program: ast.Program) !Object {
    var result: Object = undefined;
    for (program.statements.items) |statement| {
        result = try self.evalStatement(statement);
        if (result == .@"return") {
            return result.@"return".value;
        }
    }
    return result;
}

fn evalStatement(self: *Evaluator, statement: ast.Statement) !Object {
    return switch (statement) {
        .expression => |s| self.evalExpression(s.expression),
        .@"return" => |s| {
            const value = try self.evalExpression(s.return_value);
            const ret = try self.allocator.create(ReturnValue);
            ret.* = .{ .value = value };
            return .{ .@"return" = ret };
        },
        else => std.debug.panic("unhandled eval statement type: {s}", .{@tagName(statement)}),
    };
}

fn evalExpression(self: *Evaluator, expression: ast.Expression) anyerror!Object {
    return switch (expression) {
        .integer => .{ .integer = .{ .value = expression.integer.value } },
        .boolean => nativeBoolToBooleanObject(expression.boolean.value),
        .prefix => blk: {
            const right = try self.evalExpression(expression.prefix.right);
            break :blk self.evalPrefixExpression(expression.prefix.operator, right);
        },
        .infix => blk: {
            const left = try self.evalExpression(expression.infix.left);
            const right = try self.evalExpression(expression.infix.right);
            break :blk self.evalInfixExpression(expression.infix.operator, left, right);
        },
        .@"if" => self.evalIfExpression(expression.@"if"),
        else => std.debug.panic("unhandled eval expression type: {s}", .{@tagName(expression)}),
    };
}

fn evalPrefixExpression(self: *Evaluator, operator: []const u8, right: ?Object) Object {
    if (std.mem.eql(u8, operator, "!")) {
        return self.evalBangOperatorExpression(right);
    }
    if (std.mem.eql(u8, operator, "-")) {
        return self.evalMinusPrefixOperatorExpression(right);
    }
    return null_object;
}

fn evalBangOperatorExpression(_: *Evaluator, right: ?Object) Object {
    return if (right) |value| switch (value) {
        .boolean => if (value.boolean.value) false_object else true_object,
        .null => true_object,
        else => false_object,
    } else false_object;
}

fn evalMinusPrefixOperatorExpression(_: *Evaluator, right: ?Object) Object {
    const obj = right orelse return null_object;
    return switch (obj) {
        .integer => |integer| .{ .integer = .{ .value = -integer.value } },
        else => null_object,
    };
}

fn evalInfixExpression(self: *Evaluator, operator: []const u8, left: ?Object, right: ?Object) !Object {
    const l = left orelse return null_object;
    const r = right orelse return null_object;
    if (l == .integer and r == .integer) {
        return self.evalIntegerInfixExpression(operator, l, r);
    } else if (std.mem.eql(u8, operator, "==")) {
        return self.evalObjectEqualComparison(l, r);
    } else if (std.mem.eql(u8, operator, "!=")) {
        return self.evalObjectNotEqualComparison(l, r);
    } else {
        return null_object;
    }
}

fn evalObjectEqualComparison(_: *Evaluator, left: Object, right: Object) Object {
    const ltype = @as(Object.Type, left);
    const rtype = @as(Object.Type, right);
    if (ltype != rtype) return false_object;
    return switch (left) {
        .boolean => if (left.boolean.value == right.boolean.value) true_object else false_object,
        .null => true_object,
        else => std.debug.panic("unhandled object comparison: {}", .{ltype}),
    };
}

fn evalObjectNotEqualComparison(_: *Evaluator, left: Object, right: Object) Object {
    const ltype = @as(Object.Type, left);
    const rtype = @as(Object.Type, right);
    if (ltype != rtype) return true_object;
    return switch (left) {
        .boolean => if (left.boolean.value == right.boolean.value) false_object else true_object,
        .null => false_object,
        else => std.debug.panic("unhandled object comparison: {}", .{ltype}),
    };
}

fn evalIntegerInfixExpression(_: *Evaluator, operator: []const u8, left: Object, right: Object) !Object {
    const lvalue = left.integer.value;
    const rvalue = right.integer.value;

    if (std.mem.eql(u8, operator, "+")) {
        return .{ .integer = .{ .value = lvalue + rvalue } };
    } else if (std.mem.eql(u8, operator, "-")) {
        return .{ .integer = .{ .value = lvalue - rvalue } };
    } else if (std.mem.eql(u8, operator, "*")) {
        return .{ .integer = .{ .value = lvalue * rvalue } };
    } else if (std.mem.eql(u8, operator, "/")) {
        const value = try std.math.divFloor(i64, lvalue, rvalue);
        return .{ .integer = .{ .value = value } };
    } else if (std.mem.eql(u8, operator, "<")) {
        return nativeBoolToBooleanObject(lvalue < rvalue);
    } else if (std.mem.eql(u8, operator, ">")) {
        return nativeBoolToBooleanObject(lvalue > rvalue);
    } else if (std.mem.eql(u8, operator, "==")) {
        return nativeBoolToBooleanObject(lvalue == rvalue);
    } else if (std.mem.eql(u8, operator, "!=")) {
        return nativeBoolToBooleanObject(lvalue != rvalue);
    } else {
        return null_object;
    }
}

fn evalIfExpression(self: *Evaluator, expression: *ast.IfExpression) !Object {
    const condition = try self.evalExpression(expression.condition);
    if (isTruthy(condition)) {
        return self.evalBlockStatement(expression.consequence);
    } else if (expression.alternative) |alt| {
        return self.evalBlockStatement(alt);
    } else {
        return null_object;
    }
}

fn evalBlockStatement(self: *Evaluator, blk: ast.BlockStatement) !Object {
    var result: Object = undefined;
    for (blk.statements.items) |statement| {
        result = try self.evalStatement(statement);
        if (result == .@"return") {
            return result;
        }
    }
    return result;
}

fn isTruthy(obj: Object) bool {
    return switch (obj) {
        .null => false,
        .boolean => obj.boolean.value,
        else => true,
    };
}

fn nativeBoolToBooleanObject(input: bool) Object {
    return if (input) true_object else false_object;
}

const true_object = Object{ .boolean = .{ .value = true } };
const false_object = Object{ .boolean = .{ .value = false } };
const null_object = Object{ .null = {} };

pub const Object = union(Type) {
    integer: Integer,
    boolean: Boolean,
    null: void,
    @"return": *ReturnValue,

    pub const Type = enum {
        integer,
        boolean,
        null,
        @"return",
    };

    pub fn format(self: Object, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        switch (self) {
            .integer => try self.integer.format(fmt, options, writer),
            .boolean => try self.boolean.format(fmt, options, writer),
            .null => try writer.writeAll("null"),
            .@"return" => try self.@"return".format(fmt, options, writer),
        }
    }
};

pub const Integer = struct {
    value: i64,

    pub fn format(self: Integer, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.print("{}", .{self.value});
    }
};

pub const Boolean = struct {
    value: bool,

    pub fn format(self: Boolean, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.print("{}", .{self.value});
    }
};

pub const ReturnValue = struct {
    value: Object,

    pub fn format(self: ReturnValue, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.print("{}", .{self.value});
    }
};
