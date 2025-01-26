const std = @import("std");
const Lexer = @import("./Lexer.zig");
const Parser = @import("./Parser.zig");
const Evaluator = @import("./Evaluator.zig");
const Environment = @import("./Environment.zig");

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer std.debug.assert(gpa.deinit() == .ok);

    const allocator = gpa.allocator();

    var arena = std.heap.ArenaAllocator.init(allocator);
    defer arena.deinit();

    const root_alloc = arena.allocator();

    var args = try std.process.argsWithAllocator(root_alloc);
    _ = args.next() orelse unreachable; // ignore exe path

    if (args.next()) |filename| {
        const cwd = std.fs.cwd();
        var file = try cwd.openFile(filename, .{});
        defer file.close();

        const input = try file.readToEndAlloc(root_alloc, 4096);
        var lexer = Lexer.init(input);
        var parser = try Parser.init(root_alloc, &lexer);
        const program = try parser.parseProgram();
        var env = Environment.init(root_alloc);

        for (parser.errors.items) |err| {
            std.log.err("{s}", .{err});
        }

        var evaluator = Evaluator{ .allocator = root_alloc };
        const obj = try evaluator.evalProgram(program, &env);
        std.debug.print("{?}\n", .{obj});
    } else {
        var stdout = std.io.getStdOut().writer();
        var stdin = std.io.getStdIn().reader();
        var env = Environment.init(root_alloc);

        while (true) {
            try stdout.writeAll(">> ");

            var buf: [4096]u8 = undefined;
            if (try stdin.readUntilDelimiterOrEof(&buf, '\n')) |input| {
                var lexer = Lexer.init(input);
                var parser = try Parser.init(root_alloc, &lexer);
                const program = try parser.parseProgram();

                for (parser.errors.items) |err| {
                    std.log.err("{s}", .{err});
                }

                var evaluator = Evaluator{ .allocator = root_alloc };
                if (try evaluator.evalProgram(program, &env)) |result| {
                    std.debug.print("{}\n", .{result});
                }
            }
        }
    }
}
