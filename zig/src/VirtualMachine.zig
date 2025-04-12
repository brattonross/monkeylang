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
            .add => {
                const right = self.pop();
                const left = self.pop();
                const value = left.integer.value + right.integer.value;
                try self.push(.{ .integer = .{ .value = value } });
            },
        }
    }
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

pub fn stackTop(self: VirtualMachine) ?object.Object {
    return if (self.sp == 0) null else self.stack[self.sp - 1];
}

// ---

test "integer arithmetic" {
    var arena = std.heap.ArenaAllocator.init(std.testing.allocator);
    defer arena.deinit();

    const allocator = arena.allocator();

    const test_cases = [_]VMTestCase{
        .{ .input = "1", .expected = 1 },
        .{ .input = "2", .expected = 2 },
        .{ .input = "1 + 2", .expected = 3 },
    };

    try runVMTests(allocator, &test_cases);
}

const VMTestCase = struct {
    input: []const u8,
    expected: i64,
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

        const stack_elem = vm.stackTop().?;

        try testIntegerObject(test_case.expected, stack_elem);
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

const VirtualMachine = @This();

const std = @import("std");
const Allocator = std.mem.Allocator;
const Compiler = @import("./Compiler.zig");
const ast = @import("./ast.zig");
const code = @import("./code.zig");
const object = @import("./object.zig");
