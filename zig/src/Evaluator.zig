const std = @import("std");
const Allocator = std.mem.Allocator;
const ast = @import("./ast.zig");
const Environment = @import("./Environment.zig");

const Evaluator = @This();

allocator: Allocator,

pub fn evalProgram(self: *Evaluator, program: ast.Program, env: *Environment) !?Object {
    var result: ?Object = null;
    for (program.statements.items) |statement| {
        result = try self.evalStatement(statement, env);
        if (result) |res| switch (res) {
            .@"return" => |ret| return ret.value,
            .@"error" => return result,
            else => {},
        };
    }
    return result;
}

fn evalStatement(self: *Evaluator, statement: ast.Statement, env: *Environment) !?Object {
    return switch (statement) {
        .let => {
            const value = try self.evalExpression(statement.let.value, env);
            if (value) |obj| {
                if (obj == .@"error") return obj;
                try env.set(statement.let.name.value, obj);
            }
            return null;
        },
        .expression => |s| try self.evalExpression(s.expression, env),
        .@"return" => {
            const value = try self.evalExpression(statement.@"return".return_value, env) orelse return null;
            if (value == .@"error") return value;
            const ret = try self.allocator.create(ReturnValue);
            ret.* = .{ .value = value };
            return .{ .@"return" = ret };
        },
    };
}

fn evalExpression(self: *Evaluator, expression: ast.Expression, env: *Environment) anyerror!?Object {
    return switch (expression) {
        .identifier => try self.evalIdentifier(expression.identifier, env),
        .integer => .{ .integer = .{ .value = expression.integer.value } },
        .boolean => nativeBoolToBooleanObject(expression.boolean.value),
        .prefix => blk: {
            const right = try self.evalExpression(expression.prefix.right, env) orelse return null;
            if (right == .@"error") return right;
            break :blk try self.evalPrefixExpression(expression.prefix.operator, right);
        },
        .infix => blk: {
            const left = try self.evalExpression(expression.infix.left, env) orelse return null;
            if (left == .@"error") return left;
            const right = try self.evalExpression(expression.infix.right, env) orelse return null;
            if (right == .@"error") return right;
            break :blk try self.evalInfixExpression(expression.infix.operator, left, right);
        },
        .@"if" => self.evalIfExpression(expression.@"if", env),
        .function => .{ .function = .{
            .parameters = expression.function.parameters,
            .body = expression.function.body,
            .env = env,
        } },
        .call => {
            const func = try self.evalExpression(expression.call.function, env) orelse return null;
            if (func == .@"error") {
                return func;
            }
            const args = try self.evalExpressions(expression.call.arguments.items, env);
            if (args.items.len == 1 and args.items[0] == .@"error") {
                return args.items[0];
            }
            return try self.applyFunction(func, args.items);
        },
    };
}

fn evalExpressions(self: *Evaluator, expressions: []ast.Expression, env: *Environment) !std.ArrayList(Object) {
    var result = std.ArrayList(Object).init(self.allocator);

    for (expressions) |exp| {
        if (try self.evalExpression(exp, env)) |res| {
            if (res == .@"error") {
                result.clearAndFree();
                try result.append(res);
                return result;
            }
            try result.append(res);
        }
    }

    return result;
}

fn applyFunction(self: *Evaluator, func: Object, args: []Object) !?Object {
    if (func != .function) {
        const msg = try std.fmt.allocPrint(self.allocator, "not a function: {s}", .{@tagName(func)});
        return .{ .@"error" = .{ .message = msg } };
    }
    const extended_env = try self.extendFunctionEnv(func.function, args);
    const result = try self.evalBlockStatement(func.function.body, extended_env) orelse return null;
    return switch (result) {
        .@"return" => result.@"return".value,
        else => result,
    };
}

fn extendFunctionEnv(self: *Evaluator, func: Function, args: []Object) !*Environment {
    var env = try self.allocator.create(Environment);
    env.* = Environment.init(self.allocator);
    env.outer = func.env;
    for (func.parameters.items, 0..) |param, i| {
        try env.set(param.value, args[i]);
    }
    return env;
}

fn evalPrefixExpression(self: *Evaluator, operator: []const u8, right: ?Object) !Object {
    if (std.mem.eql(u8, operator, "!")) {
        return self.evalBangOperatorExpression(right);
    } else if (std.mem.eql(u8, operator, "-")) {
        return self.evalMinusPrefixOperatorExpression(right);
    }
    const msg = try std.fmt.allocPrint(self.allocator, "unknown operator: {s}{}", .{ operator, @as(Object.Type, right.?) });
    return .{ .@"error" = .{ .message = msg } };
}

fn evalBangOperatorExpression(_: *Evaluator, right: ?Object) Object {
    return if (right) |value| switch (value) {
        .boolean => if (value.boolean.value) false_object else true_object,
        .null => true_object,
        else => false_object,
    } else false_object;
}

fn evalMinusPrefixOperatorExpression(self: *Evaluator, right: ?Object) !Object {
    const obj = right orelse return null_object;
    return switch (obj) {
        .integer => |integer| .{ .integer = .{ .value = -integer.value } },
        else => {
            const msg = try std.fmt.allocPrint(self.allocator, "unknown operator: -{}", .{@as(Object.Type, right.?)});
            return .{ .@"error" = .{ .message = msg } };
        },
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
    } else if (@as(Object.Type, l) != @as(Object.Type, r)) {
        const msg = try std.fmt.allocPrint(self.allocator, "type mismatch: {} {s} {}", .{ @as(Object.Type, l), operator, @as(Object.Type, r) });
        return .{ .@"error" = .{ .message = msg } };
    } else {
        const msg = try std.fmt.allocPrint(self.allocator, "unknown operator: {} {s} {}", .{ @as(Object.Type, l), operator, @as(Object.Type, r) });
        return .{ .@"error" = .{ .message = msg } };
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

fn evalIntegerInfixExpression(self: *Evaluator, operator: []const u8, left: Object, right: Object) !Object {
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
        const msg = try std.fmt.allocPrint(self.allocator, "unknown operator: {} {s} {}", .{ @as(Object.Type, left), operator, @as(Object.Type, right) });
        return .{ .@"error" = .{ .message = msg } };
    }
}

fn evalIfExpression(self: *Evaluator, expression: *ast.IfExpression, env: *Environment) !?Object {
    const condition = try self.evalExpression(expression.condition, env) orelse return null;
    if (condition == .@"error") {
        return condition;
    } else if (isTruthy(condition)) {
        return self.evalBlockStatement(expression.consequence, env);
    } else if (expression.alternative) |alt| {
        return self.evalBlockStatement(alt, env);
    } else {
        return null_object;
    }
}

fn evalBlockStatement(self: *Evaluator, blk: ast.BlockStatement, env: *Environment) !?Object {
    var result: ?Object = null;
    for (blk.statements.items) |statement| {
        result = try self.evalStatement(statement, env);
        if (result) |res| if (res == .@"return" or res == .@"error") {
            return result;
        };
    }
    return result;
}

fn evalIdentifier(self: *Evaluator, identifier: ast.Identifier, env: *Environment) !Object {
    return env.get(identifier.value) orelse {
        const msg = try std.fmt.allocPrint(self.allocator, "identifier not found: {s}", .{identifier.value});
        return .{ .@"error" = .{ .message = msg } };
    };
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
    @"error": Error,
    function: Function,

    pub const Type = enum {
        integer,
        boolean,
        null,
        @"return",
        @"error",
        function,
    };

    pub fn format(self: Object, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        switch (self) {
            .integer => try self.integer.format(fmt, options, writer),
            .boolean => try self.boolean.format(fmt, options, writer),
            .null => try writer.writeAll("null"),
            .@"return" => try self.@"return".format(fmt, options, writer),
            .@"error" => try self.@"error".format(fmt, options, writer),
            .function => try self.function.format(fmt, options, writer),
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

pub const Error = struct {
    message: []const u8,

    pub fn format(self: Error, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.print("{s}", .{self.message});
    }
};

pub const Function = struct {
    parameters: std.ArrayList(ast.Identifier),
    body: ast.BlockStatement,
    env: *Environment,

    pub fn format(self: Function, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.writeAll("fn(");
        for (0..self.parameters.items.len) |i| {
            try writer.print("{}", .{self.parameters.items[i]});
            if (i < self.parameters.items.len - 1) {
                try writer.writeAll(", ");
            }
        }
        try writer.print(") {{\n{}\n}}", .{self.body});
    }
};
