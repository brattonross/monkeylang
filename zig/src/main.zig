pub fn main() !void {
    var gpa = std.heap.DebugAllocator(.{}).init;
    defer std.debug.assert(gpa.deinit() == .ok);

    var arena = std.heap.ArenaAllocator.init(gpa.allocator());
    defer arena.deinit();

    const allocator = arena.allocator();

    var args = try std.process.argsWithAllocator(allocator);
    _ = args.next() orelse unreachable; // ignore exe path

    if (args.next()) |file_path| {
        const cwd = std.fs.cwd();
        const input = try cwd.readFileAlloc(allocator, file_path, 4096);
        defer allocator.free(input);

        var lexer = Lexer.init(input);
        var parser = try Parser.init(allocator, &lexer);
        const program = try parser.parseProgram();
        var env = Environment.init(allocator);

        for (parser.errors.items) |err| {
            std.log.err("{s}", .{err});
        }

        var evaluator = Evaluator{ .allocator = allocator };
        const obj = try evaluator.evalProgram(program, &env);
        std.debug.print("{?}\n", .{obj});
    } else {
        var stdout = std.io.getStdOut().writer();
        var stdin = std.io.getStdIn().reader();
        // var env = Environment.init(allocator);

        while (true) {
            try stdout.writeAll(">> ");

            var buf: [4096]u8 = undefined;
            if (try stdin.readUntilDelimiterOrEof(&buf, '\n')) |input| {
                var lexer = Lexer.init(input);
                var parser = try Parser.init(allocator, &lexer);
                const program = try parser.parseProgram();

                for (parser.errors.items) |err| {
                    std.log.err("{s}", .{err});
                    continue;
                }

                var compiler = Compiler.init(allocator);
                try compiler.compile(program);

                var vm = VirtualMachine.init(compiler.bytecode());
                try vm.run();

                const stack_top = vm.stackTop();
                try stdout.print("{?}\n", .{stack_top});

                _ = arena.reset(.free_all);
            }
        }
    }
}

test {
    _ = @import("./Compiler.zig");
    _ = @import("./Parser.zig");
    _ = @import("./VirtualMachine.zig");
    _ = @import("./code.zig");
}

const std = @import("std");
const code = @import("./code.zig");
const Compiler = @import("./Compiler.zig");
const Lexer = @import("./Lexer.zig");
const Parser = @import("./Parser.zig");
const Evaluator = @import("./Evaluator.zig");
const Environment = @import("./Environment.zig");
const VirtualMachine = @import("./VirtualMachine.zig");
