const stack_size = 2048;

constants: std.ArrayList(object.Object),
instructions: code.Instructions,
stack: [stack_size]object.Object,
sp: usize,

pub fn init(bytecode: Compiler.Bytecode) VirtualMachine {
    return .{
        .constants = bytecode.constants,
        .instructions = bytecode.instructions,
        .stack = undefined,
        .sp = 0,
    };
}

pub fn deinit(self: VirtualMachine) void {
    self.constants.deinit();
    self.instructions.deinit();
}

pub fn run(self: *VirtualMachine) !void {
    var ip: usize = 0;
    while (ip < self.instructions.bytes.items.len) : (ip += 1) {
        const opcode: code.Opcode = @enumFromInt(self.instructions.bytes.items[ip]);
        switch (opcode) {
            .constant => {
                const constant_index = code.readU16(usize, self.instructions.bytes.items[ip + 1 ..]);
                ip += 2;
                try self.push(self.constants.items[constant_index]);
            },
            .pop => {
                _ = self.pop();
            },
            .add, .sub, .mul, .div => {
                try self.executeBinaryOperation(opcode);
            },
            .true => {
                try self.push(object.true_object);
            },
            .false => {
                try self.push(object.false_object);
            },
            .equal, .not_equal, .greater_than => {
                try self.executeComparison(opcode);
            },
            .bang => {
                try self.executeBangOperator();
            },
            .minus => {
                try self.executeMinusOperator();
            },
        }
    }
}

fn executeBinaryOperation(self: *VirtualMachine, opcode: code.Opcode) !void {
    const right = self.pop();
    const left = self.pop();

    if (left == .integer and right == .integer) {
        return try self.executeBinaryIntegerOperation(opcode, left.integer, right.integer);
    } else {
        return error.UnsupportedOperation;
    }
}

fn executeBinaryIntegerOperation(self: *VirtualMachine, opcode: code.Opcode, left: object.Integer, right: object.Integer) !void {
    const result = switch (opcode) {
        .add => left.value + right.value,
        .sub => left.value - right.value,
        .mul => left.value * right.value,
        .div => @divFloor(left.value, right.value),
        else => return error.UnsupportedOperation,
    };
    try self.push(.{ .integer = .{ .value = result } });
}

fn executeComparison(self: *VirtualMachine, opcode: code.Opcode) !void {
    const right = self.pop();
    const left = self.pop();

    if (left == .integer and right == .integer) {
        return try self.executeIntegerComparison(opcode, left.integer, right.integer);
    }

    switch (opcode) {
        .equal => {
            try self.push(nativeBoolToBooleanObject(std.meta.eql(left, right)));
        },
        .not_equal => {
            try self.push(nativeBoolToBooleanObject(!std.meta.eql(left, right)));
        },
        else => return error.UnsupportedOperation,
    }
}

fn executeIntegerComparison(self: *VirtualMachine, opcode: code.Opcode, left: object.Integer, right: object.Integer) !void {
    const lvalue = left.value;
    const rvalue = right.value;

    switch (opcode) {
        .equal => try self.push(nativeBoolToBooleanObject(lvalue == rvalue)),
        .not_equal => try self.push(nativeBoolToBooleanObject(lvalue != rvalue)),
        .greater_than => try self.push(nativeBoolToBooleanObject(lvalue > rvalue)),
        else => return error.UnsupportedOperation,
    }
}

fn executeBangOperator(self: *VirtualMachine) !void {
    const operand = self.pop();
    switch (operand) {
        .boolean => |b| {
            try self.push(if (b.value) object.false_object else object.true_object);
        },
        else => try self.push(object.false_object),
    }
}

fn executeMinusOperator(self: *VirtualMachine) !void {
    const operand = self.pop();
    if (operand != .integer) {
        return error.UnsupportedOperation;
    }

    try self.push(.{ .integer = .{ .value = -operand.integer.value } });
}

fn nativeBoolToBooleanObject(b: bool) object.Object {
    return if (b) object.true_object else object.false_object;
}

pub fn stackTop(self: VirtualMachine) ?object.Object {
    return if (self.sp == 0) null else self.stack[self.sp - 1];
}

pub fn lastPoppedStackElem(self: VirtualMachine) object.Object {
    return self.stack[self.sp];
}

fn push(self: *VirtualMachine, obj: object.Object) !void {
    if (self.sp >= stack_size) return error.StackOverflow;
    self.stack[self.sp] = obj;
    self.sp += 1;
}

fn pop(self: *VirtualMachine) object.Object {
    const obj = self.stack[self.sp - 1];
    self.sp -= 1;
    return obj;
}

// ---

test "integer arithmetic" {
    var arena = std.heap.ArenaAllocator.init(std.testing.allocator);
    defer arena.deinit();

    const allocator = arena.allocator();

    const test_cases = [_]VMTestCase{
        .{ .input = "1", .expected = .{ .integer = 1 } },
        .{ .input = "2", .expected = .{ .integer = 2 } },
        .{ .input = "1 + 2", .expected = .{ .integer = 3 } },
        .{ .input = "1 - 2", .expected = .{ .integer = -1 } },
        .{ .input = "1 * 2", .expected = .{ .integer = 2 } },
        .{ .input = "4 / 2", .expected = .{ .integer = 2 } },
        .{ .input = "50 / 2 * 2 + 10 - 5", .expected = .{ .integer = 55 } },
        .{ .input = "5 + 5 + 5 + 5 - 10", .expected = .{ .integer = 10 } },
        .{ .input = "2 * 2 * 2 * 2 * 2", .expected = .{ .integer = 32 } },
        .{ .input = "5 * 2 + 10", .expected = .{ .integer = 20 } },
        .{ .input = "5 + 2 * 10", .expected = .{ .integer = 25 } },
        .{ .input = "5 * (2 + 10)", .expected = .{ .integer = 60 } },
        .{ .input = "-5", .expected = .{ .integer = -5 } },
        .{ .input = "-10", .expected = .{ .integer = -10 } },
        .{ .input = "-50 + 100 + -50", .expected = .{ .integer = 0 } },
        .{ .input = "(5 + 10 * 2 + 15 / 3) * 2 + -10", .expected = .{ .integer = 50 } },
    };

    try runVMTests(allocator, &test_cases);
}

test "boolean expressions" {
    var arena = std.heap.ArenaAllocator.init(std.testing.allocator);
    defer arena.deinit();

    const allocator = arena.allocator();

    const test_cases = [_]VMTestCase{
        .{ .input = "true", .expected = .{ .boolean = true } },
        .{ .input = "false", .expected = .{ .boolean = false } },
        .{ .input = "1 < 2", .expected = .{ .boolean = true } },
        .{ .input = "1 > 2", .expected = .{ .boolean = false } },
        .{ .input = "1 < 1", .expected = .{ .boolean = false } },
        .{ .input = "1 > 1", .expected = .{ .boolean = false } },
        .{ .input = "1 == 1", .expected = .{ .boolean = true } },
        .{ .input = "1 != 1", .expected = .{ .boolean = false } },
        .{ .input = "1 == 2", .expected = .{ .boolean = false } },
        .{ .input = "1 != 2", .expected = .{ .boolean = true } },
        .{ .input = "true == true", .expected = .{ .boolean = true } },
        .{ .input = "false == false", .expected = .{ .boolean = true } },
        .{ .input = "true == false", .expected = .{ .boolean = false } },
        .{ .input = "true != false", .expected = .{ .boolean = true } },
        .{ .input = "false != true", .expected = .{ .boolean = true } },
        .{ .input = "(1 < 2) == true", .expected = .{ .boolean = true } },
        .{ .input = "(1 < 2) == false", .expected = .{ .boolean = false } },
        .{ .input = "(1 > 2) == true", .expected = .{ .boolean = false } },
        .{ .input = "(1 > 2) == false", .expected = .{ .boolean = true } },
        .{ .input = "!true", .expected = .{ .boolean = false } },
        .{ .input = "!false", .expected = .{ .boolean = true } },
        .{ .input = "!5", .expected = .{ .boolean = false } },
        .{ .input = "!!true", .expected = .{ .boolean = true } },
        .{ .input = "!!false", .expected = .{ .boolean = false } },
        .{ .input = "!!5", .expected = .{ .boolean = true } },
    };

    try runVMTests(allocator, &test_cases);
}

const VMTestCase = struct {
    input: []const u8,
    expected: union(enum) {
        integer: i64,
        boolean: bool,
    },
};

fn runVMTests(allocator: Allocator, test_cases: []const VMTestCase) !void {
    for (test_cases) |test_case| {
        const program = try parse(allocator, test_case.input);

        var compiler = Compiler.init(allocator);
        defer compiler.deinit();

        try compiler.compile(program);

        var vm = VirtualMachine.init(compiler.bytecode());
        defer vm.deinit();

        try vm.run();

        const stack_elem = vm.lastPoppedStackElem();
        switch (test_case.expected) {
            .integer => try testIntegerObject(test_case.expected.integer, stack_elem),
            .boolean => try testBooleanObject(test_case.expected.boolean, stack_elem),
        }
    }
}

const Lexer = @import("./Lexer.zig");
const Parser = @import("./Parser.zig");

fn parse(allocator: Allocator, input: []const u8) !ast.Program {
    var lexer = Lexer.init(input);
    var parser = try Parser.init(allocator, &lexer);
    defer parser.deinit();
    return try parser.parseProgram();
}

fn testIntegerObject(expected: i64, actual: object.Object) !void {
    try std.testing.expectEqual(expected, actual.integer.value);
}

fn testBooleanObject(expected: bool, actual: object.Object) !void {
    try std.testing.expectEqual(expected, actual.boolean.value);
}

const VirtualMachine = @This();

const std = @import("std");
const Allocator = std.mem.Allocator;
const Compiler = @import("./Compiler.zig");
const ast = @import("./ast.zig");
const code = @import("./code.zig");
const object = @import("./object.zig");
