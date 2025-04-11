pub const true_object = Object{ .boolean = .{ .value = true } };
pub const false_object = Object{ .boolean = .{ .value = false } };
pub const null_object = Object{ .null = {} };

pub const Object = union(Type) {
    integer: Integer,
    boolean: Boolean,
    null: void,
    @"return": *ReturnValue,
    @"error": Error,
    function: Function,
    string: String,
    builtin: Builtin,
    array: Array,

    pub const Type = enum {
        integer,
        boolean,
        null,
        @"return",
        @"error",
        function,
        string,
        builtin,
        array,
    };

    pub fn format(self: Object, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        switch (self) {
            .integer => try self.integer.format(fmt, options, writer),
            .boolean => try self.boolean.format(fmt, options, writer),
            .null => try writer.writeAll("null"),
            .@"return" => try self.@"return".format(fmt, options, writer),
            .@"error" => try self.@"error".format(fmt, options, writer),
            .function => try self.function.format(fmt, options, writer),
            .string => try self.string.format(fmt, options, writer),
            .builtin => try writer.writeAll("builtin function"),
            .array => try self.array.format(fmt, options, writer),
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

pub const String = struct {
    value: []const u8,

    pub fn format(self: String, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;
        try writer.print("{s}", .{self.value});
    }
};

pub const BuiltinError = std.mem.Allocator.Error;

pub const Builtin = struct {
    allocator: std.mem.Allocator,
    function: *const fn (ctx: *anyopaque, args: []Object) BuiltinError!Object,
};

pub const Array = struct {
    elements: std.ArrayList(Object),

    pub fn format(self: Array, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;

        try writer.writeAll("[");
        for (self.elements.items, 0..) |element, i| {
            try writer.print("{}", .{element});
            if (i < self.elements.items.len - 1) {
                try writer.writeAll(", ");
            }
        }
        try writer.writeAll("]");
    }
};

const std = @import("std");
const ast = @import("./ast.zig");
const Environment = @import("./Environment.zig");
