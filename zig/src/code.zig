pub const Instructions = struct {
    allocator: Allocator,
    bytes: std.ArrayList(u8),

    pub fn init(allocator: Allocator) Instructions {
        return .{ .allocator = allocator, .bytes = std.ArrayList(u8).init(allocator) };
    }

    pub fn deinit(self: Instructions) void {
        self.bytes.deinit();
    }

    pub fn format(self: Instructions, comptime fmt: []const u8, options: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = options;

        var i: usize = 0;
        while (i < self.bytes.items.len) {
            try writer.print("{d:04} ", .{i});

            const definition = lookup(@enumFromInt(self.bytes.items[i])).?;

            var result = try readOperands(self.allocator, definition, self.bytes.items[i + 1 ..]);
            defer result.deinit();

            const operand_count = definition.operand_widths.len;
            if (result.operands_read.items.len != operand_count) {
                return error.OperandLengthMismatch;
            }

            switch (operand_count) {
                0 => try writer.print("{s}", .{definition.name}),
                1 => try writer.print("{s} {d}", .{ definition.name, result.operands_read.items[0] }),
                else => return error.Unimplemented,
            }

            try writer.writeAll("\n");

            i += 1 + result.bytes_read;
        }
    }
};

pub const Opcode = enum(u8) {
    constant,
    add,
    pop,
    sub,
    mul,
    div,
    true,
    false,
    equal,
    not_equal,
    greater_than,
    minus,
    bang,
};

pub const Definition = struct {
    name: []const u8,
    operand_widths: []const usize,
};

pub fn lookup(opcode: Opcode) ?Definition {
    return switch (opcode) {
        .constant => .{
            .name = "OpConstant",
            .operand_widths = &.{2},
        },
        .add => .{
            .name = "OpAdd",
            .operand_widths = &.{},
        },
        .pop => .{
            .name = "OpPop",
            .operand_widths = &.{},
        },
        .sub => .{
            .name = "OpSub",
            .operand_widths = &.{},
        },
        .mul => .{
            .name = "OpMul",
            .operand_widths = &.{},
        },
        .div => .{
            .name = "OpDiv",
            .operand_widths = &.{},
        },
        .true => .{
            .name = "OpTrue",
            .operand_widths = &.{},
        },
        .false => .{
            .name = "OpFalse",
            .operand_widths = &.{},
        },
        .equal => .{
            .name = "OpEqual",
            .operand_widths = &.{},
        },
        .not_equal => .{
            .name = "OpNotEqual",
            .operand_widths = &.{},
        },
        .greater_than => .{
            .name = "OpGreaterThan",
            .operand_widths = &.{},
        },
        .minus => .{
            .name = "OpMinus",
            .operand_widths = &.{},
        },
        .bang => .{
            .name = "OpBang",
            .operand_widths = &.{},
        },
    };
}

pub fn make(allocator: Allocator, opcode: Opcode, operands: []const usize) !Instructions {
    const definition = lookup(opcode) orelse return error.InvalidOpcode;

    var instruction_len: usize = 1;
    for (definition.operand_widths) |w| {
        instruction_len += w;
    }

    var instructions = Instructions.init(allocator);
    try instructions.bytes.append(@intFromEnum(opcode));

    for (operands, 0..) |operand, i| {
        const width = definition.operand_widths[i];
        switch (width) {
            2 => {
                const value: u16 = @intCast(operand);
                try instructions.bytes.append(@intCast((value >> 8) & 0xFF));
                try instructions.bytes.append(@intCast(value & 0xFF));
            },
            else => std.debug.panic("unsupported operand width: {d}", .{width}),
        }
    }

    return instructions;
}

const ReadOperandsResult = struct {
    operands_read: std.ArrayList(usize),
    bytes_read: usize,

    pub fn init(allocator: Allocator) ReadOperandsResult {
        return .{ .operands_read = std.ArrayList(usize).init(allocator), .bytes_read = 0 };
    }

    pub fn deinit(self: *ReadOperandsResult) void {
        self.operands_read.deinit();
    }
};

fn readOperands(allocator: Allocator, definition: Definition, instructions: []const u8) !ReadOperandsResult {
    var result = ReadOperandsResult.init(allocator);
    var offset: usize = 0;

    for (definition.operand_widths) |width| {
        switch (width) {
            2 => {
                const value = readU16(usize, instructions[offset..]);
                try result.operands_read.append(value);
            },
            else => std.debug.panic("unsupported operand width: {d}", .{width}),
        }

        offset += width;
    }

    result.bytes_read = offset;
    return result;
}

pub fn readU16(comptime T: type, bytes: []const u8) T {
    return @intCast((@as(u16, bytes[0]) << 8) | bytes[1]);
}

test make {
    const test_cases = [_]struct {
        op: Opcode,
        operands: []const usize,
        expected: []const u8,
    }{
        .{ .op = .constant, .operands = &.{65534}, .expected = &.{ @intFromEnum(Opcode.constant), 255, 254 } },
        .{ .op = .add, .operands = &.{}, .expected = &.{@intFromEnum(Opcode.add)} },
    };

    for (test_cases) |test_case| {
        var instruction = try make(std.testing.allocator, test_case.op, test_case.operands);
        defer instruction.deinit();

        try std.testing.expectEqualSlices(u8, test_case.expected, instruction.bytes.items);
    }
}

test readOperands {
    const test_cases = [_]struct {
        op: Opcode,
        operands: []const usize,
        bytes_read: usize,
    }{
        .{ .op = .constant, .operands = &.{65535}, .bytes_read = 2 },
    };

    for (test_cases) |test_case| {
        var instruction = try make(std.testing.allocator, test_case.op, test_case.operands);
        defer instruction.deinit();

        const definition = lookup(test_case.op).?;

        var result = try readOperands(std.testing.allocator, definition, instruction.bytes.items[1..]);
        defer result.deinit();

        try std.testing.expectEqual(test_case.bytes_read, result.bytes_read);
        for (test_case.operands, 0..) |expected_operand, i| {
            try std.testing.expectEqual(expected_operand, result.operands_read.items[i]);
        }
    }
}

test "instructions string" {
    const allocator = std.testing.allocator;

    const instructions = [_]Instructions{
        try make(allocator, .add, &.{}),
        try make(allocator, .constant, &.{2}),
        try make(allocator, .constant, &.{65535}),
    };
    defer for (instructions) |instruction| {
        instruction.deinit();
    };

    var concatted = Instructions.init(allocator);
    defer concatted.deinit();

    for (instructions) |instruction| {
        try concatted.bytes.appendSlice(instruction.bytes.items);
    }

    const actual = try std.fmt.allocPrint(allocator, "{}", .{concatted});
    defer allocator.free(actual);

    try std.testing.expectEqualStrings(
        \\0000 OpAdd
        \\0001 OpConstant 2
        \\0004 OpConstant 65535
        \\
    , actual);
}

const std = @import("std");
const Allocator = std.mem.Allocator;
