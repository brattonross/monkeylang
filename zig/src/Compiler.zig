allocator: Allocator,
instructions: code.Instructions,
constants: std.ArrayList(object.Object),

pub fn init(allocator: Allocator) Compiler {
    return .{
        .allocator = allocator,
        .instructions = code.Instructions.init(allocator),
        .constants = std.ArrayList(object.Object).init(allocator),
    };
}

pub fn deinit(self: *Compiler) void {
    self.constants.deinit();
}

pub fn compile(self: *Compiler, program: ast.Program) !void {
    for (program.statements.items) |statement| {
        try self.compileStatement(statement);
    }
}

fn compileStatement(self: *Compiler, statement: ast.Statement) !void {
    switch (statement) {
        .expression => {
            try self.compileExpression(statement.expression.expression.*);
            _ = try self.emit(.pop, &.{});
        },
        else => std.debug.panic("unsupported statement type: {s}", .{@tagName(statement)}),
    }
}

fn compileExpression(self: *Compiler, expression: ast.Expression) !void {
    switch (expression) {
        .integer => |integer| {
            const int_object: object.Object = .{ .integer = .{ .value = integer.value } };
            const pos = try self.addConstant(int_object);
            var operands = [_]usize{pos};
            _ = try self.emit(.constant, &operands);
        },
        .boolean => |b| {
            _ = try self.emit(if (b.value) .true else .false, &.{});
        },
        .prefix => |prefix| {
            try self.compileExpression(prefix.right.*);
            if (std.mem.eql(u8, "!", prefix.operator)) {
                _ = try self.emit(.bang, &.{});
            } else if (std.mem.eql(u8, "-", prefix.operator)) {
                _ = try self.emit(.minus, &.{});
            } else {
                return error.UnknownOperator;
            }
        },
        .infix => |infix| {
            if (std.mem.eql(u8, "<", infix.operator)) {
                try self.compileExpression(infix.right.*);
                try self.compileExpression(infix.left.*);
                _ = try self.emit(.greater_than, &.{});
                return;
            }

            try self.compileExpression(infix.left.*);
            try self.compileExpression(infix.right.*);

            if (std.mem.eql(u8, "+", infix.operator)) {
                _ = try self.emit(.add, &.{});
            } else if (std.mem.eql(u8, "-", infix.operator)) {
                _ = try self.emit(.sub, &.{});
            } else if (std.mem.eql(u8, "*", infix.operator)) {
                _ = try self.emit(.mul, &.{});
            } else if (std.mem.eql(u8, "/", infix.operator)) {
                _ = try self.emit(.div, &.{});
            } else if (std.mem.eql(u8, ">", infix.operator)) {
                _ = try self.emit(.greater_than, &.{});
            } else if (std.mem.eql(u8, "==", infix.operator)) {
                _ = try self.emit(.equal, &.{});
            } else if (std.mem.eql(u8, "!=", infix.operator)) {
                _ = try self.emit(.not_equal, &.{});
            } else {
                return error.UnknownOperator;
            }
        },
        else => std.debug.panic("unsupported expression type: {s}", .{@tagName(expression)}),
    }
}

fn emit(self: *Compiler, opcode: code.Opcode, operands: []const usize) !usize {
    const ins = try code.make(self.allocator, opcode, operands);
    return try self.addInstruction(ins);
}

fn addConstant(self: *Compiler, obj: object.Object) !usize {
    try self.constants.append(obj);
    return self.constants.items.len - 1;
}

fn addInstruction(self: *Compiler, ins: code.Instructions) !usize {
    const pos = self.instructions.bytes.items.len;
    try self.instructions.bytes.appendSlice(ins.bytes.items);
    return pos;
}

pub const Bytecode = struct {
    instructions: code.Instructions,
    constants: std.ArrayList(object.Object),
};

pub fn bytecode(self: *Compiler) Bytecode {
    return .{
        .instructions = self.instructions,
        .constants = self.constants,
    };
}

// ---

const CompilerTestCase = struct {
    input: []const u8,
    expected_constants: []const usize,
    expected_instructions: []const code.Instructions,
};

fn runCompilerTests(allocator: Allocator, tests: []const CompilerTestCase) !void {
    for (tests) |test_case| {
        const program = try parse(allocator, test_case.input);

        var compiler = Compiler.init(allocator);
        try compiler.compile(program);

        const bc = compiler.bytecode();

        try testInstructions(allocator, test_case.expected_instructions, bc.instructions);
        try testConstants(test_case.expected_constants, bc.constants.items);
    }
}

fn testInstructions(allocator: Allocator, expected: []const code.Instructions, actual: code.Instructions) !void {
    const concatted = try concatInstructions(allocator, expected);
    try std.testing.expectEqual(concatted.bytes.items.len, actual.bytes.items.len);
    try std.testing.expectEqualSlices(u8, concatted.bytes.items, actual.bytes.items);
}

fn testConstants(expected: []const usize, actual: []const object.Object) !void {
    try std.testing.expectEqual(expected.len, actual.len);

    for (expected, 0..) |constant, i| {
        try testIntegerObject(@intCast(constant), actual[i]);
    }
}

fn testIntegerObject(expected: i64, actual: object.Object) !void {
    try std.testing.expectEqual(expected, actual.integer.value);
}

fn concatInstructions(allocator: Allocator, instructions: []const code.Instructions) !code.Instructions {
    var out = code.Instructions.init(allocator);
    for (instructions) |instruction| {
        try out.bytes.appendSlice(instruction.bytes.items);
    }
    return out;
}

const Lexer = @import("./Lexer.zig");
const Parser = @import("./Parser.zig");

fn parse(allocator: Allocator, input: []const u8) !ast.Program {
    var lexer = Lexer.init(input);
    var parser = try Parser.init(allocator, &lexer);
    return try parser.parseProgram();
}

test "integer arithmetic" {
    var arena = std.heap.ArenaAllocator.init(std.testing.allocator);
    defer arena.deinit();

    const allocator = arena.allocator();

    const test_cases = [_]CompilerTestCase{
        .{
            .input = "1 + 2",
            .expected_constants = &.{ 1, 2 },
            .expected_instructions = &.{
                try code.make(allocator, .constant, &.{0}),
                try code.make(allocator, .constant, &.{1}),
                try code.make(allocator, .add, &.{}),
                try code.make(allocator, .pop, &.{}),
            },
        },
        .{
            .input = "1; 2",
            .expected_constants = &.{ 1, 2 },
            .expected_instructions = &.{
                try code.make(allocator, .constant, &.{0}),
                try code.make(allocator, .pop, &.{}),
                try code.make(allocator, .constant, &.{1}),
                try code.make(allocator, .pop, &.{}),
            },
        },
        .{
            .input = "1 - 2",
            .expected_constants = &.{ 1, 2 },
            .expected_instructions = &.{
                try code.make(allocator, .constant, &.{0}),
                try code.make(allocator, .constant, &.{1}),
                try code.make(allocator, .sub, &.{}),
                try code.make(allocator, .pop, &.{}),
            },
        },
        .{
            .input = "1 * 2",
            .expected_constants = &.{ 1, 2 },
            .expected_instructions = &.{
                try code.make(allocator, .constant, &.{0}),
                try code.make(allocator, .constant, &.{1}),
                try code.make(allocator, .mul, &.{}),
                try code.make(allocator, .pop, &.{}),
            },
        },
        .{
            .input = "2 / 1",
            .expected_constants = &.{ 2, 1 },
            .expected_instructions = &.{
                try code.make(allocator, .constant, &.{0}),
                try code.make(allocator, .constant, &.{1}),
                try code.make(allocator, .div, &.{}),
                try code.make(allocator, .pop, &.{}),
            },
        },
        .{
            .input = "-1",
            .expected_constants = &.{1},
            .expected_instructions = &.{
                try code.make(allocator, .constant, &.{0}),
                try code.make(allocator, .minus, &.{}),
                try code.make(allocator, .pop, &.{}),
            },
        },
    };

    try runCompilerTests(allocator, &test_cases);
}

test "boolean expressions" {
    var arena = std.heap.ArenaAllocator.init(std.testing.allocator);
    defer arena.deinit();

    const allocator = arena.allocator();

    const test_cases = [_]CompilerTestCase{
        .{
            .input = "true",
            .expected_constants = &.{},
            .expected_instructions = &.{
                try code.make(allocator, .true, &.{}),
                try code.make(allocator, .pop, &.{}),
            },
        },
        .{
            .input = "false",
            .expected_constants = &.{},
            .expected_instructions = &.{
                try code.make(allocator, .false, &.{}),
                try code.make(allocator, .pop, &.{}),
            },
        },
        .{
            .input = "1 > 2",
            .expected_constants = &.{ 1, 2 },
            .expected_instructions = &.{
                try code.make(allocator, .constant, &.{0}),
                try code.make(allocator, .constant, &.{1}),
                try code.make(allocator, .greater_than, &.{}),
                try code.make(allocator, .pop, &.{}),
            },
        },
        .{
            .input = "1 < 2",
            .expected_constants = &.{ 2, 1 },
            .expected_instructions = &.{
                try code.make(allocator, .constant, &.{0}),
                try code.make(allocator, .constant, &.{1}),
                try code.make(allocator, .greater_than, &.{}),
                try code.make(allocator, .pop, &.{}),
            },
        },
        .{
            .input = "1 == 2",
            .expected_constants = &.{ 1, 2 },
            .expected_instructions = &.{
                try code.make(allocator, .constant, &.{0}),
                try code.make(allocator, .constant, &.{1}),
                try code.make(allocator, .equal, &.{}),
                try code.make(allocator, .pop, &.{}),
            },
        },
        .{
            .input = "1 != 2",
            .expected_constants = &.{ 1, 2 },
            .expected_instructions = &.{
                try code.make(allocator, .constant, &.{0}),
                try code.make(allocator, .constant, &.{1}),
                try code.make(allocator, .not_equal, &.{}),
                try code.make(allocator, .pop, &.{}),
            },
        },
        .{
            .input = "true == false",
            .expected_constants = &.{},
            .expected_instructions = &.{
                try code.make(allocator, .true, &.{}),
                try code.make(allocator, .false, &.{}),
                try code.make(allocator, .equal, &.{}),
                try code.make(allocator, .pop, &.{}),
            },
        },
        .{
            .input = "true != false",
            .expected_constants = &.{},
            .expected_instructions = &.{
                try code.make(allocator, .true, &.{}),
                try code.make(allocator, .false, &.{}),
                try code.make(allocator, .not_equal, &.{}),
                try code.make(allocator, .pop, &.{}),
            },
        },
        .{
            .input = "!true",
            .expected_constants = &.{},
            .expected_instructions = &.{
                try code.make(allocator, .true, &.{}),
                try code.make(allocator, .bang, &.{}),
                try code.make(allocator, .pop, &.{}),
            },
        },
    };

    try runCompilerTests(allocator, &test_cases);
}

const Compiler = @This();

const std = @import("std");
const Allocator = std.mem.Allocator;
const ast = @import("./ast.zig");
const code = @import("./code.zig");
const object = @import("./object.zig");
